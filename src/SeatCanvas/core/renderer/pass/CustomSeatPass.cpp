//
//  CustomSeatPass.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/9.
//

#include "CustomSeatPass.hpp"

#include "UniformData.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/renderer/SeatRegionMesh.hpp"

#include <algorithm>
#include <tgfx/core/Image.h>
#include <tgfx/gpu/Context.h>
#include <tgfx/gpu/GPU.h>
#include <tgfx/gpu/GPUBuffer.h>
#include <tgfx/gpu/Texture.h>
#include <tgfx/platform/Print.h>

namespace kk::renderer {
static constexpr char SEAT_VERTEXT_SHADER[] = R"(
in vec2 inPosition;
in vec2 inTextureCoord;
in int inStyleIndex;

layout(std140) uniform VertexUniformBlock {
    mat3 uMVP;
    vec4 uTextureCoordRects[40];
};

out vec2 vTexCoord;

void main() {
    vec3 pos = uMVP * vec3(inPosition, 1.0);
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    vec4 uvRect = uTextureCoordRects[inStyleIndex];
    vTexCoord = uvRect.xy + inTextureCoord * (uvRect.zw - uvRect.xy);
}
)";

static constexpr char SEAT_FRAGMENT_SHADER[] = R"(
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D sAtlasTexture;

out vec4 fragColor;

void main() {
    fragColor = texture(sAtlasTexture, vTexCoord);
}
)";

CustomSeatPass::CustomSeatPass() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);

    memset(&bitFields, 0, sizeof(bitFields));
    bitFields.dirtyUVTable = true;
    bitFields.avaiable = false;

    position = {"inPosition", tgfx::VertexFormat::Float2};
    textureCoord = {"inTextureCoord", tgfx::VertexFormat::Float2};
    styleIndex = {"inStyleIndex", tgfx::VertexFormat::Int};

    mvpUniform = {"uMVP", UniformFormat::Float3x3};
    textureCoordRectsUniform = {"uTextureCoordRects", UniformFormat::Float4, 40};
    uniformData.reset(new UniformData({mvpUniform, textureCoordRectsUniform}));
}

CustomSeatPass::~CustomSeatPass() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void CustomSeatPass::updateRegionMeshes(const std::vector<std::shared_ptr<SeatRegionMesh>> &meshes) {
    regionMeshes = meshes;
}

void CustomSeatPass::updateUVOffset(const std::vector<float> &uvOffset) {
    bitFields.dirtyUVTable = true;
    this->uvOffset = uvOffset;
}

void CustomSeatPass::clearRegionMeshes() {
    regionMeshes.clear();
}

bool CustomSeatPass::hasData() const {
    return !regionMeshes.empty();
}

void CustomSeatPass::setDefaultColor(const tgfx::Color &color) {
    defaultColor = color;
}

void CustomSeatPass::setAtlasTexture(std::shared_ptr<tgfx::Texture> texture) {
    if (texture == atlasTexture) {
        return;
    }
    atlasTexture = texture;
}

bool CustomSeatPass::onDraw(tgfx::CommandEncoder *encoder, const SeatCanvasCoreRendererState *state) {
    bitFields.avaiable = false;

    if (encoder == nullptr || atlasTexture == nullptr || regionMeshes.empty()) {
        return false;
    }

    auto gpu = encoder->gpu();
    if (gpu == nullptr) {
        return false;
    }

    if (!preparePipeline(gpu)) {
        return false;
    }

    auto viewport = state->getBoundsSize();
    auto textureWidth = static_cast<int>(viewport.width);
    auto textureHeight = static_cast<int>(viewport.height);

    if (!prepareRenderTexture(gpu, textureWidth, textureHeight)) {
        return false;
    }

    if (!prepareSampler(gpu)) {
        return false;
    }

    if (!prepareBuffer(gpu)) {
        return false;
    }

    if (!updateMVPMatrix(state)) {
        return false;
    }

    if (!updateUBOBuffer()) {
        return false;
    }

    tgfx::RenderPassDescriptor passDesc(renderTexture, tgfx::LoadAction::Clear, tgfx::StoreAction::Store, tgfx::PMColor::Transparent());
    auto renderPass = encoder->beginRenderPass(passDesc);
    if (!renderPass) {
        return false;
    }

    renderPass->setViewport(0, 0, textureWidth, textureHeight);
    renderPass->setPipeline(pipeline);
    renderPass->setTexture(0, atlasTexture, sampler);
    renderPass->setUniformBuffer(0, uboBuffer, 0, uboBuffer->size());

    for (const auto &mesh : regionMeshes) {
        if (mesh->vertexCount == 0 || !mesh->vbo) {
            continue;
        }

        renderPass->setVertexBuffer(0, mesh->vbo);
        if (mesh->indexCount > 0 && mesh->ibo) {
            renderPass->setIndexBuffer(mesh->ibo);
            renderPass->drawIndexed(tgfx::PrimitiveType::Triangles, mesh->indexCount);
        } else {
            renderPass->draw(tgfx::PrimitiveType::Triangles, mesh->vertexCount);
        }
    }

    renderPass->end();

    bitFields.avaiable = true;

    return true;
}

std::shared_ptr<tgfx::Image> CustomSeatPass::outputImage() {
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

void CustomSeatPass::onResetGPUResources() {
    uboBuffer.reset();
    pipeline.reset();
    textureImage.reset();
    renderTexture.reset();
    sampler.reset();
}

std::string CustomSeatPass::onBuildVertexShader() const {
    return SEAT_VERTEXT_SHADER;
}

std::string CustomSeatPass::onBuildFragmentShader() const {
    return SEAT_FRAGMENT_SHADER;
}

std::vector<tgfx::VertexBufferLayout> CustomSeatPass::vertexBufferLayouts() const {
    tgfx::VertexBufferLayout vertextLayout{{position, textureCoord, styleIndex}, tgfx::VertexStepMode::Vertex};
    return {vertextLayout};
}

std::vector<tgfx::BindingEntry> CustomSeatPass::uniformBlocks() const {
    return {{"VertexUniformBlock", 0}};
}

std::vector<tgfx::BindingEntry> CustomSeatPass::textureSamplers() const {
    return {{"sAtlasTexture", 0}};
}

bool CustomSeatPass::updateMVPMatrix(const SeatCanvasCoreRendererState *state) {
    if (state == nullptr) {
        return false;
    }

    mvpMatrix = state->getMVPMatrix();
    return true;
}

bool CustomSeatPass::prepareRenderTexture(tgfx::GPU *gpu, int width, int height) {
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

bool CustomSeatPass::preparePipeline(tgfx::GPU *gpu) {
    if (pipeline) {
        return true;
    }
    pipeline = createPipeline(gpu);
    return pipeline != nullptr;
}

bool CustomSeatPass::prepareSampler(tgfx::GPU *gpu) {
    if (sampler) {
        return true;
    }

    // 配置采样器：使用线性过滤和边缘夹紧
    tgfx::SamplerDescriptor samplerDesc{};
    samplerDesc.minFilter = tgfx::FilterMode::Linear;
    samplerDesc.magFilter = tgfx::FilterMode::Linear;
    samplerDesc.mipmapMode = tgfx::MipmapMode::None;  // Atlas 通常不需要 mipmap
    samplerDesc.addressModeX = tgfx::AddressMode::ClampToEdge;
    samplerDesc.addressModeY = tgfx::AddressMode::ClampToEdge;
    sampler = gpu->createSampler(samplerDesc);
    return sampler != nullptr;
}

bool CustomSeatPass::prepareBuffer(tgfx::GPU *gpu) {
    do {
        if (uboBuffer) {
            break;
        }

        auto uboSize = uniformData->size();
        uboBuffer = gpu->createBuffer(uboSize, tgfx::GPUBufferUsage::UNIFORM);
        if (!uboBuffer) {
            return false;
        }

    } while (0);

    return true;
}

bool CustomSeatPass::updateUBOBuffer() {
    auto ptr = uboBuffer->map();
    if (ptr == nullptr) {
        return false;
    }
    uniformData->setBuffer(ptr);
    uniformData->setData(mvpUniform.name(), mvpMatrix);

    if (bitFields.dirtyUVTable) {
        auto expectedSize = textureCoordRectsUniform.count() * 4;
        std::vector<float> uvRects(expectedSize, 0.0f);
        auto copySize = std::min(uvOffset.size(), static_cast<size_t>(expectedSize));
        if (copySize > 0) {
            std::copy(uvOffset.begin(), uvOffset.begin() + copySize, uvRects.begin());
        }
        uniformData->setArrayData(textureCoordRectsUniform.name(), uvRects.data(), textureCoordRectsUniform.count());
        bitFields.dirtyUVTable = false;
    }
    uboBuffer->unmap();
    return true;
}

};  // namespace kk::renderer
