//
//  SeatStyleAtlasManager.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "SeatStyleAtlasManager.hpp"

#include <cmath>

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Color.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/Context.h>

#include "CanvasSeatStyleRenderer.hpp"

namespace kk::renderer {

SeatStyleAtlasManager::SeatStyleAtlasManager(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleConfigs)
    : styleConfigs(styleConfigs) {
}

SeatStyleAtlasManager::~SeatStyleAtlasManager() {
    clear();
}

void SeatStyleAtlasManager::update(float density, const tgfx::Size &seatSize) {
    bool sizeChanged = (this->seatSize != seatSize);
    bool densityChanged = (this->density != density);

    if (!sizeChanged && !densityChanged && atlasTexture != nullptr) {
        return;
    }

    this->density = density;
    this->seatSize = seatSize;
    atlasTexture = nullptr;

    generateAtlas();
}

void SeatStyleAtlasManager::onResetGPUResources() {
    clear();
}

void SeatStyleAtlasManager::generateAtlas() {
    auto context = getContext();
    if (context == nullptr || seatSize.width <= 0 || seatSize.height <= 0) {
        atlasTexture = nullptr;
        uvRects.clear();
        return;
    }

    if (atlasTexture) {
        return;
    }

    itemWidth = static_cast<int>(seatSize.width * this->density);
    itemHeight = static_cast<int>(seatSize.height * this->density);

    auto renderer = getStyleRenderer();
    if (!renderer) {
        atlasTexture = nullptr;
        uvRects.clear();
        return;
    }

    constexpr int maxColumns = 10;

    int styleCount = static_cast<int>(styleConfigs.size());
    if (styleCount == 0) {
        atlasTexture = nullptr;
        uvRects.clear();
        return;
    }

    int rows = static_cast<int>(std::ceil(static_cast<float>(styleCount) / static_cast<float>(maxColumns)));

    atlasWidth = itemWidth * maxColumns + (maxColumns + 1) * itemSpacing;
    atlasHeight = itemHeight * rows + (rows + 1) * itemSpacing;

    tgfx::TextureDescriptor desc{
        atlasWidth,
        atlasHeight,
        tgfx::PixelFormat::RGBA_8888,
        false,
        1,
        tgfx::TextureUsage::RENDER_ATTACHMENT | tgfx::TextureUsage::TEXTURE_BINDING,
    };
    auto texture = context->gpu()->createTexture(desc);
    auto surface = tgfx::Surface::MakeFrom(context, texture->getBackendTexture(), tgfx::ImageOrigin::TopLeft);
    if (!surface) {
        atlasTexture = nullptr;
        uvRects.clear();
        return;
    }

    auto canvas = surface->getCanvas();
    if (!canvas) {
        atlasTexture = nullptr;
        uvRects.clear();
        return;
    }

    canvas->clear(tgfx::Color::Transparent());

    tgfx::Size itemSize(static_cast<float>(itemWidth), static_cast<float>(itemHeight));
    auto renderedRects = renderer->renderAllSeatStyles(canvas, itemSize, maxColumns, itemSpacing, this->density, styleConfigs);

    uvRects.clear();
    uvOffsets.clear();
    styleIdToIndexMap.clear();

    int32_t index = 0;
    for (const auto &[styleId, rect] : renderedRects) {
        float uMin = rect.x() / static_cast<float>(atlasWidth);
        float vMin = rect.y() / static_cast<float>(atlasHeight);
        float uMax = (rect.x() + rect.width()) / static_cast<float>(atlasWidth);
        float vMax = (rect.y() + rect.height()) / static_cast<float>(atlasHeight);

        uvRects[styleId] = tgfx::Rect::MakeLTRB(uMin, vMin, uMax, vMax);

        uvOffsets.push_back(uMin);
        uvOffsets.push_back(vMin);
        uvOffsets.push_back(uMax);
        uvOffsets.push_back(vMax);

        styleIdToIndexMap[styleId] = index;
        index++;
    }

    context->flushAndSubmit();

    atlasTexture = texture;
    if (!atlasTexture) {
        uvRects.clear();
        return;
    }

    if (onAtlasGenerated) {
        onAtlasGenerated(this);
    }
}

bool SeatStyleAtlasManager::setStyleIdToConfigs(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleConfigs) {
    if (this->styleConfigs.size() == styleConfigs.size()) {
        bool isEqual = true;
        for (const auto &[styleId, config] : styleConfigs) {
            auto iter = this->styleConfigs.find(styleId);
            if (iter == this->styleConfigs.end()) {
                isEqual = false;
                break;
            }
            if (iter->second != config) {
                if (!iter->second || !config || !(*iter->second == *config)) {
                    isEqual = false;
                    break;
                }
            }
        }
        if (isEqual) {
            return false;
        }
    }
    this->styleConfigs = styleConfigs;
    atlasTexture = nullptr;
    return true;
}

std::shared_ptr<SeatStyleRenderer> SeatStyleAtlasManager::getStyleRenderer() {
    if (!styleRenderer) {
        styleRenderer = std::make_shared<CanvasSeatStyleRenderer>();
    }
    return styleRenderer;
}

bool SeatStyleAtlasManager::getUVCoords(const std::string &styleId, tgfx::Point &uvMin, tgfx::Point &uvMax) const {
    auto iter = uvRects.find(styleId);
    if (iter == uvRects.end()) {
        return false;
    }

    const auto &rect = iter->second;
    uvMin = tgfx::Point::Make(rect.x(), rect.y());
    uvMax = tgfx::Point::Make(rect.x() + rect.width(), rect.y() + rect.height());
    return true;
}

int32_t SeatStyleAtlasManager::getUVOffsetIndex(const std::string &styleId) const {
    auto iter = styleIdToIndexMap.find(styleId);
    if (iter == styleIdToIndexMap.end()) {
        return -1;
    }
    return iter->second;
}

void SeatStyleAtlasManager::clear() {
    atlasTexture.reset();
    uvRects.clear();
    uvOffsets.clear();
    styleIdToIndexMap.clear();
}

};  // namespace kk::renderer
