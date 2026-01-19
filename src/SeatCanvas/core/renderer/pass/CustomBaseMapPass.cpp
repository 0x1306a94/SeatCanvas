//
//  CustomBaseMapPass.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/9.
//

#include "CustomBaseMapPass.hpp"

#include "UniformData.hpp"
#include "core/renderer/BaseMapMeshBuilder.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"

#include <algorithm>
#include <tgfx/core/Image.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/Context.h>
#include <tgfx/gpu/GPU.h>
#include <tgfx/gpu/GPUBuffer.h>
#include <tgfx/gpu/Texture.h>
#include <tgfx/platform/Print.h>
#include <unordered_set>

namespace kk::renderer {
static constexpr char FILL_VERTEXT_SHADER[] = R"(
in vec2 inPosition;
in float inCoverage;
in int inColorIndex;

layout(std140) uniform VertexUniformBlock {
    mat3 uMVP;
    ivec2 uColorTextureSize;
};

out float vCoverage;
flat out ivec2 vTexCoord;

void main() {
    vec3 pos = uMVP * vec3(inPosition, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    vCoverage = inCoverage;
    // 从颜色纹理中采样：使用多行布局
    // 计算行列位置：row = inColorIndex / textureWidth, col = inColorIndex % textureWidth
    int row = inColorIndex / uColorTextureSize.x;
    int col = inColorIndex % uColorTextureSize.x;
    vTexCoord = ivec2(col, row);
}
)";

static constexpr char FILL_FRAGMENT_SHADER[] = R"(
precision mediump float;

in float vCoverage;
flat in ivec2 vTexCoord;

uniform sampler2D sColorTexture;

out vec4 fragColor;

void main() {
    vec4 color = texelFetch(sColorTexture, vTexCoord, 0);
    fragColor = color * vec4(vCoverage);
}
)";

CustomBaseMapPass::CustomBaseMapPass() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    memset(&bitFields, 0, sizeof(bitFields));
    bitFields.dirtyFillUBO = true;
    bitFields.dirtyFillVBO = true;
    bitFields.dirtyStrokeUBO = true;
    bitFields.dirtyStrokeVBO = true;
    bitFields.avaiable = false;

    position = {"inPosition", tgfx::VertexFormat::Float2};
    coverage = {"inCoverage", tgfx::VertexFormat::Float};
    colorIndex = {"inColorIndex", tgfx::VertexFormat::Int};

    fillMVPUniform = {"uMVP", UniformFormat::Float3x3};
    fillColorTextureSizeUniform = {"uColorTextureSize", UniformFormat::Int2};

    fillUniformData.reset(new UniformData({fillMVPUniform, fillColorTextureSizeUniform}));
}

CustomBaseMapPass::~CustomBaseMapPass() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void CustomBaseMapPass::updateMeshBuilder(std::shared_ptr<BaseMapMeshBuilder> meshBuilder) {
    if (meshBuilder == nullptr) {
        this->meshBuilder = nullptr;
        return;
    }

    if (this->meshBuilder == meshBuilder) {
        return;
    }

    this->meshBuilder = meshBuilder;
    this->fillVBOBuffer = nullptr;
    bitFields.dirtyFillColor = true;
    bitFields.dirtyFillVBO = true;
}

void CustomBaseMapPass::updateColorState(kk::BaseMapColorState state) {
    if (colorState == state) {
        return;
    }
    colorState = state;
    bitFields.dirtyFillColor = true;
}

void CustomBaseMapPass::invalidateColorTable() {
    bitFields.dirtyFillColor = true;
}

bool CustomBaseMapPass::onDraw(tgfx::CommandEncoder *encoder, const SeatCanvasCoreRendererState *state) {
    bitFields.avaiable = false;
    if (!meshBuilder) {
        return false;
    }

    auto visibleIndices = getVisibleRegionMesheIndices(state);
    if (visibleIndices.empty()) {
        return false;
    }

    if (encoder == nullptr) {
        return false;
    }

    auto gpu = encoder->gpu();
    if (gpu == nullptr) {
        return false;
    }

    if (!prepareFillPipeline(gpu)) {
        return false;
    }

    auto viewport = state->getBoundsSize();
    auto textureWidth = static_cast<int>(viewport.width);
    auto textureHeight = static_cast<int>(viewport.height);

    if (!prepareRenderTexture(gpu, textureWidth, textureHeight)) {
        return false;
    }
    // 可选
    prepareMSAATexture(gpu, textureWidth, textureHeight);

    if (!prepareFillBuffer(gpu)) {
        return false;
    }

    if (!prepareColorTexture(gpu)) {
        return false;
    }

    if (!updateMVPMatrix(state)) {
        return false;
    }

    if (!updateFillVBOBuffer()) {
        return false;
    }

    if (!updateColorTexture(gpu, state)) {
        return false;
    }

    if (!updateFillUBOBuffer()) {
        return false;
    }

    std::shared_ptr<tgfx::RenderPass> renderPass = nullptr;
    if (msaaTexture) {
        tgfx::RenderPassDescriptor msaaPassDesc(msaaTexture, tgfx::LoadAction::Clear, tgfx::StoreAction::Store, tgfx::PMColor::Transparent(), renderTexture);
        renderPass = encoder->beginRenderPass(msaaPassDesc);
    } else {
        tgfx::RenderPassDescriptor passDesc(renderTexture, tgfx::LoadAction::Clear, tgfx::StoreAction::Store, tgfx::PMColor::Transparent());
        renderPass = encoder->beginRenderPass(passDesc);
    }

    if (!renderPass) {
        return false;
    }

    renderPass->setViewport(0, 0, textureWidth, textureHeight);
    renderFill(renderPass, visibleIndices);
    renderPass->end();
    bitFields.avaiable = true;
    return true;
}

std::shared_ptr<tgfx::Image> CustomBaseMapPass::outputImage() {

    if (!renderTexture || context == nullptr || !bitFields.avaiable) {
        return nullptr;
    }

    if (textureImage) {
        return textureImage;
    }
    auto backendTexture = renderTexture->getBackendTexture();
    textureImage = tgfx::Image::MakeFrom(context, backendTexture, tgfx::ImageOrigin::BottomLeft);
    return textureImage;
}

void CustomBaseMapPass::onResetGPUResources() {
    fillVBOBuffer.reset();
    fillPipeline.reset();
    textureImage.reset();
    msaaTexture.reset();
    renderTexture.reset();
    colorTexture.reset();
    colorSampler.reset();

    bitFields.dirtyFillVBO = true;
}

std::string CustomBaseMapPass::onBuildVertexShader() const {
    return FILL_VERTEXT_SHADER;
}

std::string CustomBaseMapPass::onBuildFragmentShader() const {
    return FILL_FRAGMENT_SHADER;
}

std::vector<tgfx::VertexBufferLayout> CustomBaseMapPass::vertexBufferLayouts() const {
    tgfx::VertexBufferLayout vertextLayout{{position, coverage, colorIndex}, tgfx::VertexStepMode::Vertex};
    return {vertextLayout};
}

std::vector<tgfx::BindingEntry> CustomBaseMapPass::uniformBlocks() const {
    return {{"VertexUniformBlock", 0}};
}

std::vector<tgfx::BindingEntry> CustomBaseMapPass::textureSamplers() const {
    return {{"sColorTexture", 0}};
}

bool CustomBaseMapPass::updateMVPMatrix(const SeatCanvasCoreRendererState *state) {
    if (state == nullptr) {
        return false;
    }

    mvpMatrix = state->getMVPMatrix();
    return true;
}

bool CustomBaseMapPass::prepareRenderTexture(tgfx::GPU *gpu, int width, int height) {
    if (width <= 0 || height <= 0) {
        return false;
    }

    if (renderTexture && renderTexture->width() == width && renderTexture->height() == height) {
        return true;
    }

    tgfx::TextureDescriptor desc{
        width,
        height,
        tgfx::PixelFormat::RGBA_8888,
        false,
        1,
        tgfx::TextureUsage::RENDER_ATTACHMENT | tgfx::TextureUsage::TEXTURE_BINDING,
    };
    renderTexture = gpu->createTexture(desc);
    textureImage.reset();
    return renderTexture != nullptr;
}

bool CustomBaseMapPass::prepareMSAATexture(tgfx::GPU *gpu, int width, int height) {
    if (width <= 0 || height <= 0) {
        return false;
    }

    if (msaaTexture && msaaTexture->width() == width && msaaTexture->height() == height) {
        return true;
    }

    int requestedSampleCount = 4;
    int supportedSampleCount = gpu->getSampleCount(requestedSampleCount, tgfx::PixelFormat::RGBA_8888);
    if (supportedSampleCount < requestedSampleCount) {
        return false;
    }

    tgfx::TextureDescriptor desc{
        width,
        height,
        tgfx::PixelFormat::RGBA_8888,
        false,
        requestedSampleCount,
        tgfx::TextureUsage::RENDER_ATTACHMENT,
    };
    msaaTexture = gpu->createTexture(desc);
    return msaaTexture != nullptr;
}

bool CustomBaseMapPass::prepareFillPipeline(tgfx::GPU *gpu) {
    if (fillPipeline) {
        return true;
    }
    fillPipeline = createPipeline(gpu);
    return fillPipeline != nullptr;
}

bool CustomBaseMapPass::prepareFillBuffer(tgfx::GPU *gpu) {
    do {
        if (fillVBOBuffer) {
            break;
        }
        auto vboSize = (position.size() + coverage.size() + colorIndex.size()) * meshBuilder->getTotalVertexCount();
        fillVBOBuffer = gpu->createBuffer(vboSize, tgfx::GPUBufferUsage::VERTEX);
        if (!fillVBOBuffer) {
            return false;
        }
    } while (0);

    do {
        if (fillUBOBuffer) {
            break;
        }

        size_t uboSize = fillUniformData->size();
        fillUBOBuffer = gpu->createBuffer(uboSize, tgfx::GPUBufferUsage::UNIFORM);
        if (!fillUBOBuffer) {
            return false;
        }

    } while (0);

    return true;
}

bool CustomBaseMapPass::updateFillVBOBuffer() {
    if (!bitFields.dirtyFillVBO) {
        return true;
    }

    BaseMapRegionVertex *ptr = static_cast<BaseMapRegionVertex *>(fillVBOBuffer->map());
    if (ptr == nullptr) {
        return false;
    }

    const auto &regionMeshInfos = meshBuilder->getRegionMeshInfos();
    int32_t index = -1;
    for (const auto &regionMesh : regionMeshInfos) {
        auto drawRange = meshBuilder->findRegionDrawRangeByIndex(++index);
        if (!drawRange) {
            continue;
        }

        auto offset = drawRange->vertexOffset;
        auto &fillVertices = regionMesh->fillVertices;
        auto &strokeVertices = regionMesh->strokeVertices;
        auto fillColorIndex = index * 2;
        auto strokeColorIndex = fillColorIndex + 1;
        for (auto &vertext : fillVertices) {
            vertext.colorIndex = fillColorIndex;
        }

        for (auto &vertext : strokeVertices) {
            vertext.colorIndex = strokeColorIndex;
        }

        if (regionMesh->strokeOnTop) {
            if (!fillVertices.empty()) {
                memcpy(static_cast<void *>((ptr + offset)), fillVertices.data(), sizeof(BaseMapRegionVertex) * fillVertices.size());
                offset += fillVertices.size();
            }

            if (!strokeVertices.empty()) {
                memcpy(static_cast<void *>((ptr + offset)), strokeVertices.data(), sizeof(BaseMapRegionVertex) * strokeVertices.size());
            }
        } else {
            if (!strokeVertices.empty()) {
                memcpy(static_cast<void *>((ptr + offset)), strokeVertices.data(), sizeof(BaseMapRegionVertex) * strokeVertices.size());
                offset += strokeVertices.size();
            }

            if (!fillVertices.empty()) {
                memcpy(static_cast<void *>((ptr + offset)), fillVertices.data(), sizeof(BaseMapRegionVertex) * fillVertices.size());
            }
        }
    }

    fillVBOBuffer->unmap();
    bitFields.dirtyFillVBO = false;
    return true;
}

bool CustomBaseMapPass::updateFillUBOBuffer() {
    if (!fillUBOBuffer) {
        return false;
    }

    auto ptr = fillUBOBuffer->map();
    if (ptr == nullptr) {
        return false;
    }
    fillUniformData->setBuffer(ptr);
    fillUniformData->setData(fillMVPUniform.name(), mvpMatrix);
    int size[2] = {colorTexture->width(), colorTexture->height()};
    fillUniformData->setData(fillColorTextureSizeUniform.name(), size, sizeof(size));
    fillUBOBuffer->unmap();

    bitFields.dirtyFillUBO = false;
    return true;
}

bool CustomBaseMapPass::prepareColorTexture(tgfx::GPU *gpu) {
    if (colorTexture && colorSampler) {
        return true;
    }

    // 创建采样器
    if (!colorSampler) {
        tgfx::SamplerDescriptor samplerDesc{};
        samplerDesc.minFilter = tgfx::FilterMode::Nearest;  // 使用最近邻，因为颜色是离散的
        samplerDesc.magFilter = tgfx::FilterMode::Nearest;
        samplerDesc.mipmapMode = tgfx::MipmapMode::None;
        samplerDesc.addressModeX = tgfx::AddressMode::ClampToEdge;
        samplerDesc.addressModeY = tgfx::AddressMode::ClampToEdge;
        colorSampler = gpu->createSampler(samplerDesc);
        if (!colorSampler) {
            return false;
        }
    }

    // 颜色纹理会在 updateColorTexture 中创建
    return true;
}

bool CustomBaseMapPass::updateColorTexture(tgfx::GPU *gpu, const SeatCanvasCoreRendererState *state) {
    const auto &regionMeshInfos = meshBuilder->getRegionMeshInfos();
    if (regionMeshInfos.empty()) {
        return false;
    }

    // 创建颜色纹理：最大宽度为 512，超过时使用多行布局
    constexpr int kMaxTextureWidth = 512;
    int colorCount = static_cast<int>(regionMeshInfos.size() * 2);
    int textureWidth = std::min(colorCount, kMaxTextureWidth);
    int textureHeight = (colorCount + kMaxTextureWidth - 1) / kMaxTextureWidth;  // 向上取整

    if (!colorTexture || colorTexture->width() != textureWidth || colorTexture->height() != textureHeight) {
        tgfx::TextureDescriptor desc{
            textureWidth,
            textureHeight,
            tgfx::PixelFormat::RGBA_8888,
            false,
            1,
            tgfx::TextureUsage::TEXTURE_BINDING,
        };
        colorTexture = gpu->createTexture(desc);
        if (!colorTexture) {
            return false;
        }
        bitFields.dirtyFillColor = true;
    }

    //    if (!bitFields.dirtyFillColor) {
    //        return true;
    //    }

    auto rowBytes = static_cast<size_t>(textureWidth * 4);
    // 准备像素数据：RGBA 格式，多行布局
    std::vector<uint8_t> pixelData(textureHeight * rowBytes);
    // 初始化未使用的像素为透明黑色
    std::fill(pixelData.begin(), pixelData.end(), 0);

    for (size_t index = 0; index < regionMeshInfos.size(); index++) {
        const auto &regionMesh = regionMeshInfos[index];
        auto fillColorToUse = regionMesh->fillColor;
        if (colorState == kk::BaseMapColorState::Rainbow && regionMesh->priceColor) {
            fillColorToUse = regionMesh->priceColor;
        }

        auto fillColorIndex = index * 2;
        auto strokeColorIndex = fillColorIndex + 1;

        if (fillColorToUse) {
            int row = static_cast<int>(fillColorIndex / textureWidth);
            int col = static_cast<int>(fillColorIndex % textureWidth);
            size_t pixelIndex = (row * textureWidth + col) * 4;
            auto &color = fillColorToUse.value();
            auto additionalAlpha = regionMesh->additionalAlpha;
            pixelData[pixelIndex + 0] = static_cast<uint8_t>(color.red * 255.0f);
            pixelData[pixelIndex + 1] = static_cast<uint8_t>(color.green * 255.0f);
            pixelData[pixelIndex + 2] = static_cast<uint8_t>(color.blue * 255.0f);
            pixelData[pixelIndex + 3] = static_cast<uint8_t>(color.alpha * 255.0f * additionalAlpha);
        }

        if (regionMesh->strokeColor) {
            float strokeAlpha = hairlineStrokeAlpha(state, regionMesh->strokeWidth);
            if (strokeAlpha <= 0.0f) {
                continue;
            }

            int row = static_cast<int>(strokeColorIndex / textureWidth);
            int col = static_cast<int>(strokeColorIndex % textureWidth);
            size_t pixelIndex = (row * textureWidth + col) * 4;
            auto &color = regionMesh->strokeColor.value();

            pixelData[pixelIndex + 0] = static_cast<uint8_t>(color.red * 255.0f);
            pixelData[pixelIndex + 1] = static_cast<uint8_t>(color.green * 255.0f);
            pixelData[pixelIndex + 2] = static_cast<uint8_t>(color.blue * 255.0f);
            pixelData[pixelIndex + 3] = static_cast<uint8_t>(color.alpha * 255.0f * strokeAlpha);
        }
    }

    gpu->queue()->writeTexture(colorTexture, tgfx::Rect::MakeWH(textureWidth, textureHeight), pixelData.data(), rowBytes);

    bitFields.dirtyFillColor = false;
    return true;
}

void CustomBaseMapPass::renderFill(std::shared_ptr<tgfx::RenderPass> &renderPass, const std::vector<size_t> &visibleIndices) {
    if (!fillUBOBuffer || !colorTexture || !colorSampler) {
        return;
    }
    renderPass->setPipeline(fillPipeline);
    renderPass->setUniformBuffer(0, fillUBOBuffer, 0, fillUBOBuffer->size());
    renderPass->setTexture(0, colorTexture, colorSampler);
    renderPass->setVertexBuffer(0, fillVBOBuffer);

    const auto &regionMeshInfos = meshBuilder->getRegionMeshInfos();
    if (visibleIndices.size() == regionMeshInfos.size()) {
        renderPass->draw(tgfx::PrimitiveType::Triangles, static_cast<int>(meshBuilder->getTotalVertexCount()));
        return;
    }

    // 优化策略：合并连续的索引段为一次 draw call
    size_t i = 0;
    while (i < visibleIndices.size()) {
        // 找到连续段的起始索引
        auto startIndex = visibleIndices[i];
        auto endIndex = startIndex;

        // 查找连续段的结束索引
        while (i + 1 < visibleIndices.size() && visibleIndices[i + 1] == endIndex + 1) {
            ++i;
            endIndex = visibleIndices[i];
        }

        // 获取起始区域的顶点偏移
        auto startDrawRange = meshBuilder->findRegionDrawRangeByIndex(startIndex);
        if (!startDrawRange) {
            ++i;
            continue;
        }

        // 获取结束区域的顶点偏移和顶点数，用于计算连续段的总顶点数
        auto endDrawRange = meshBuilder->findRegionDrawRangeByIndex(endIndex);
        if (!endDrawRange) {
            ++i;
            continue;
        }

        // 计算连续段的总顶点数：结束区域的偏移 + 结束区域的顶点数 - 起始区域的偏移
        auto totalVertexCount = (endDrawRange->vertexOffset + endDrawRange->vertexCount - startDrawRange->vertexOffset);

        if (totalVertexCount > 0) {
            // 绘制连续段的顶点（一次 draw call）
            renderPass->draw(tgfx::PrimitiveType::Triangles, totalVertexCount, 1, startDrawRange->vertexOffset);
        }

        ++i;
    }
}

float CustomBaseMapPass::hairlineStrokeAlpha(const SeatCanvasCoreRendererState *state, float strokeWidth) const {
    if (state == nullptr) {
        return 1.0f;
    }

    // 计算 stroke 在屏幕上的实际像素宽度
    // screenStrokeWidth = strokeWidth * originalToNormalizedScale * zoomScale
    auto baseMapSize = state->getOriginSize();
    auto normalizedContentSize = state->getNormalizedContentSize();
    auto zoomScale = state->getZoomScale();

    float originalToNormalizedScale = normalizedContentSize.width / baseMapSize.width;
    float screenStrokeWidth = strokeWidth * originalToNormalizedScale * zoomScale;

    // Hairline 淡出逻辑：
    // - 当 screenStrokeWidth >= 1.0px 时，完全显示 (alpha = 1.0)
    // - 当 screenStrokeWidth < 1.0px 且 > 0.3px 时，线性淡出
    // - 当 screenStrokeWidth <= 0.3px 时，完全不显示 (alpha = 0.0)
    constexpr float kFadeOutStart = 1.0f;  // 开始淡出的阈值
    constexpr float kFadeOutEnd = 0.3f;    // 完全消失的阈值

    float strokeAlpha = 1.0f;
    if (screenStrokeWidth < kFadeOutStart) {
        if (screenStrokeWidth <= kFadeOutEnd) {
            strokeAlpha = 0.0f;
        } else {
            // 线性插值: (current - end) / (start - end)
            strokeAlpha = (screenStrokeWidth - kFadeOutEnd) / (kFadeOutStart - kFadeOutEnd);
        }
    }

    return strokeAlpha;
}

std::vector<size_t> CustomBaseMapPass::getVisibleRegionMesheIndices(const SeatCanvasCoreRendererState *state) const {
    if (!meshBuilder || !state) {
        return {};
    }

    // 计算可见区域
    tgfx::Rect visibleRect = state->getVisibleOriginalRect();
    if (visibleRect.isEmpty()) {
        return {};
    }

    // 扩大一点点，避免出现刚好在边缘的隐藏/显示，视觉上体验不好
    //    visibleRect.outset(60, 60);

    // 查找与可见区域相交的所有区域
    const auto &regionMeshInfos = meshBuilder->getRegionMeshInfos();
    std::vector<size_t> visibleIndices{};
    meshBuilder->findRegionsIntersectingRect(visibleRect, &visibleIndices);
    return visibleIndices;
}
};  // namespace kk::renderer
