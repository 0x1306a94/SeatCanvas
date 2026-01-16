//
//  SeatLayerTree.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "SeatLayerTree.hpp"

#include "core/layers/BaseMapRootLayer.hpp"
#include "core/layers/SeatAtlasLayer.hpp"
#include "core/layers/SeatItemLayer.hpp"
#include "core/layers/SeatRegionLayer.hpp"
#include "core/layers/SeatRootLayer.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/svg/ConvertSVGLayer.hpp"

#include <tgfx/core/Buffer.h>
#include <tgfx/core/Canvas.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Stream.h>
#include <tgfx/layers/DisplayList.h>
#include <tgfx/layers/ImageLayer.h>
#include <tgfx/layers/ShapeLayer.h>
#include <tgfx/layers/TextLayer.h>
#include <tgfx/platform/Print.h>

namespace kk::drawers {

SeatLayerTree::SeatLayerTree()
    : kk::drawers::Drawer("SeatLayerTree")
    , _root(nullptr)
    , _baseMapLayer(nullptr)
    , _displayList(std::make_unique<tgfx::DisplayList>()) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);

    _displayList->setRenderMode(_enableTiled ? tgfx::RenderMode::Tiled : tgfx::RenderMode::Partial);
//    _displayList->setSubtreeCacheMaxSize(1024);
#if DEBUG
    _displayList->showDirtyRegions(_enableTiled);
#endif
};

SeatLayerTree::~SeatLayerTree() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void SeatLayerTree::setBaseMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &baseMapSize) {
    if (_baseMapLayer != nullptr && _baseMapLayer == layer) {
        return;
    }

    _baseMapLayer = std::move(layer);
    _baseMapSize = baseMapSize;
    _baseMapImage = nullptr;
    _rebuildLayer = true;
}

void SeatLayerTree::invalidateSeatStatusImage() {
}

void SeatLayerTree::clearSeatAtlas() {
    std::unordered_map<std::string, std::shared_ptr<kk::layer::SeatAtlasLayer>> temp;
    std::swap(_seatAtlasLayers, temp);
    for (auto &[key, value] : temp) {
        if (value) {
            value->removeFromParent();
        }
    }
}

void SeatLayerTree::hiddenSeatAtlas() {
    for (auto &[key, value] : _seatAtlasLayers) {
        if (value) {
            value->setVisible(false);
        }
    }
}

void SeatLayerTree::hiddenSeatAtlas(const std::unordered_map<std::string, bool> visibleMap) {
    const auto map_end = visibleMap.end();
    for (auto &[key, value] : _seatAtlasLayers) {
        if (!value) {
            continue;
        }

        auto iter = visibleMap.find(key);
        if (iter == map_end) {
            value->setVisible(false);
        } else {
            value->setVisible(iter->second);
        }
    }
}

std::shared_ptr<kk::layer::SeatAtlasLayer> SeatLayerTree::getSeatAtlasLayerOrCreate(const std::string &regionId, std::function<void(kk::layer::SeatAtlasLayer *)> onCreate) {
    if (regionId.empty()) {
        return nullptr;
    }

    auto iter = _seatAtlasLayers.find(regionId);
    std::shared_ptr<kk::layer::SeatAtlasLayer> layer = nullptr;

    if (iter != _seatAtlasLayers.end()) {
        layer = iter->second;
    }

    if (!layer) {
        layer = kk::layer::SeatAtlasLayer::Make();
        if (onCreate) {
            onCreate(layer.get());
        }
        _seatAtlasLayers.insert_or_assign(regionId, layer);
    }

    if (layer->parent() == nullptr && _seatRootLayer) {
        _seatRootLayer->addChild(layer);
    }
    return layer;
}

void SeatLayerTree::removeSeateAtlas(const std::string &regionId) {
    if (regionId.empty()) {
        return;
    }

    auto iter = _seatAtlasLayers.find(regionId);
    if (iter != _seatAtlasLayers.end()) {
        return;
    }

    if (iter->second) {
        iter->second->removeFromParent();
    }
    _seatAtlasLayers.erase(iter);
}

void SeatLayerTree::enableTiled(bool enable) {
    if (_enableTiled == enable) {
        return;
    }
    _enableTiled = enable;
    _displayList->setRenderMode(_enableTiled ? tgfx::RenderMode::Tiled : tgfx::RenderMode::Partial);
#if DEBUG
    _displayList->showDirtyRegions(_enableTiled);
#endif
}

void SeatLayerTree::enableZoomBlur(bool enable) {
    if (_enableZoomBlur == enable) {
        return;
    }
    _enableZoomBlur = enable;
    _displayList->setAllowZoomBlur(_enableZoomBlur);
}

void SeatLayerTree::enableLoadSeat(bool enable) {
    _enableLoadSeat = enable;
}

tgfx::Point SeatLayerTree::globalToLocal(const tgfx::Point &globalPoint) const {
    if (_root == nullptr) {
        return tgfx::Point::Zero();
    }
    auto local = _root->globalToLocal(globalPoint);
    return local;
}

tgfx::Point SeatLayerTree::localToGlobal(const tgfx::Point &localPoint) const {
    if (_root == nullptr) {
        return tgfx::Point::Zero();
    }
    auto global = _root->localToGlobal(localPoint);
    return global;
}

std::optional<std::string> SeatLayerTree::getSeatRegionIdAt(float x, float y) const {
    if (_baseMapLayer == nullptr) {
        return std::nullopt;
    }

    // 先粗略的按照 bounding box 查找
    auto layers = _baseMapLayer->getLayersUnderPoint(x, y);
    if (layers.empty()) {
        return std::nullopt;
    }

    /*
     * 对于座位区域会使用特定的 Layer
     * 所以这里直接找最第一个区域 Layer 就行
     */
    for (const auto &layer : layers) {
        auto layerType = static_cast<kk::layer::CustomLayerType>(layer->type());
        if (layerType != kk::layer::CustomLayerType::Region) {
            continue;
        }

        // 再精确查找
        if (!layer->hitTestPoint(x, y, true)) {
            continue;
        }

        auto shapeLayer = std::static_pointer_cast<kk::layer::SeatRegionLayer>(layer);
        auto name = shapeLayer->name();
        if (name.empty()) {
            return std::nullopt;
        }
        return {name};
    }

    return std::nullopt;
}

std::shared_ptr<kk::layer::SeatAtlasLayer> SeatLayerTree::getSeatAtlasAt(float x, float y) const {
    if (_seatRootLayer == nullptr) {
        return nullptr;
    }

    auto layers = _seatRootLayer->getLayersUnderPoint(x, y);
    if (layers.empty()) {
        return nullptr;
    }

    for (const auto &layer : layers) {
        auto layerType = static_cast<kk::layer::CustomLayerType>(layer->type());
        if (layerType != kk::layer::CustomLayerType::SeatAtlas) {
            continue;
        }

        // 再精确检测
        if (!layer->hitTestPoint(x, y, true)) {
            continue;
        }

        auto atlas = std::static_pointer_cast<kk::layer::SeatAtlasLayer>(layer);
        return atlas;
    }

    return nullptr;
}

std::shared_ptr<kk::layer::SeatItemLayer> SeatLayerTree::getSeatItemAt(float x, float y) const {
    if (_seatRootLayer == nullptr) {
        return nullptr;
    }

    auto layers = _seatRootLayer->getLayersUnderPoint(x, y);
    if (layers.empty()) {
        return nullptr;
    }

    for (const auto &layer : layers) {
        auto layerType = static_cast<kk::layer::CustomLayerType>(layer->type());
        if (layerType != kk::layer::CustomLayerType::Seat) {
            continue;
        }

        //        // 再精确检测
        //        if (!layer->hitTestPoint(x, y, true)) {
        //            continue;
        //        }

        auto atlas = std::static_pointer_cast<kk::layer::SeatItemLayer>(layer);
        return atlas;
    }

    return nullptr;
}

bool SeatLayerTree::hasContentChanged() const {
    return _displayList->hasContentChanged();
}

bool SeatLayerTree::hitTestPoint(const tgfx::Point &location) const {
    if (!_seatRootLayer) {
        return false;
    }

    auto result = _seatRootLayer->hitTestPoint(location.x, location.y);
    return result;
}

void SeatLayerTree::prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) {
    prebuildSeatStatusImage(canvas, state);
    //    prebuildBaseMapImage(canvas, state);

    if (updateContentSize(state) || force || _rebuildLayer) {
        buildLayerTree(canvas, state);
    }

    auto zoomScale = state->zoomScale();
    auto contentOffset = state->contentOffset();
    _displayList->setZoomScale(zoomScale);
    _displayList->setContentOffset(contentOffset.x, contentOffset.y);
}

bool SeatLayerTree::updateContentSize(const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto contentSize = state->getContentSize();
    if (_contentSize == contentSize) {
        return false;
    }

    _contentSize = contentSize;
    return true;
}

bool SeatLayerTree::prebuildBaseMapImage(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto limitSize = state->getContentSize();
    if (limitSize.isEmpty()) {
        return false;
    }

    if (_baseMapLayer == nullptr || _baseMapImage != nullptr) {
        return false;
    }

    auto targetWidth = static_cast<int>(limitSize.width);
    auto ratio = static_cast<float>(targetWidth / _baseMapSize.width);
    auto targetHeight = static_cast<int>(std::ceil(_baseMapSize.height * ratio));

    auto scaleX = static_cast<float>(targetWidth) / _baseMapSize.width;
    auto scaleY = static_cast<float>(targetHeight) / _baseMapSize.height;
    auto fitScale = std::min(scaleX, scaleY);

    auto context = canvas->getSurface()->getContext();
    auto surface = tgfx::Surface::Make(context, targetWidth, targetHeight);
    if (!surface) {
        return false;
    }

    auto root = tgfx::Layer::Make();
    root->addChild(_baseMapLayer);
    root->setMatrix(tgfx::Matrix::MakeScale(fitScale));
    tgfx::DisplayList displayList{};
    displayList.root()->addChild(root);
    displayList.setRenderMode(tgfx::RenderMode::Direct);
    displayList.render(surface.get(), true);

    _baseMapLayer->removeFromParent();

    //    auto RGBAInfo = tgfx::ImageInfo::Make(surface->width(), surface->height(), tgfx::ColorType::RGBA_8888, tgfx::AlphaType::Premultiplied);
    //    tgfx::Buffer buffer(RGBAInfo.byteSize());
    //    auto pixels = buffer.data();
    //
    //    if (!surface->readPixels(RGBAInfo, pixels)) {
    //        return false;
    //    }

    //    _baseMapImage = tgfx::Image::MakeFrom(RGBAInfo, buffer.release());

    _baseMapImage = surface->makeImageSnapshot();

    return _baseMapImage != nullptr;
}

bool SeatLayerTree::prebuildSeatStatusImage(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {

    return true;
}

void SeatLayerTree::loadSeatIfneeded(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    if (!_enableLoadSeat) {
        return;
    }

    return;
}

void SeatLayerTree::buildLayerTree(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    if (_seatRootLayer) {
        _seatRootLayer->removeFromParent();
        _seatRootLayer = nullptr;
    }

    if (_root) {
        _root->removeFromParent();
        _root = nullptr;
    }

    if (_baseMapLayer == nullptr) {
        return;
    }

    _root = tgfx::Layer::Make();

    _seatRootLayer = kk::layer::SeatRootLayer::Make();
    auto path = _baseMapLayer->path();
    auto matrix = _baseMapLayer->matrix();
    _seatRootLayer->setPath(std::move(path));
    _seatRootLayer->setMatrix(matrix);
    //        _seatRootLayer->setFillStyle(tgfx::SolidColor::Make(tgfx::Color(0.0f, 0.0f, 0.0f, 0.5)));

    if (_baseMapImage) {
        auto imageLayer = tgfx::ImageLayer::Make();
        imageLayer->setImage(_baseMapImage);

        auto scaleX = _baseMapSize.width / static_cast<float>(_baseMapImage->width());
        auto scaleY = _baseMapSize.height / static_cast<float>(_baseMapImage->height());
        auto fitScale = std::min(scaleX, scaleY);

        imageLayer->setMatrix(tgfx::Matrix::MakeScale(fitScale));
        _root->addChild(imageLayer);
    } else {
        _root->addChild(_baseMapLayer);
    }

    _root->addChild(_seatRootLayer);
    _root->setVisible(visible());

    updateRootMatrix(canvas, state);

    _displayList->root()->addChild(_root);

    _rebuildLayer = false;
}

void SeatLayerTree::updateRootMatrix(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto limitSize = state->getContentSize();
    if (_root == nullptr) {
        return;
    }

    if (limitSize.isEmpty() || _baseMapSize.isEmpty()) {
        _root->setMatrix(tgfx::Matrix::I());
        return;
    }

    // 计算缩放比例（等比例缩放，保证完整显示）
    const float scaleX = limitSize.width / _baseMapSize.width;
    const float scaleY = limitSize.height / _baseMapSize.height;
    const float fitScale = std::min(scaleX, scaleY);

    tgfx::Matrix finalMatrix = tgfx::Matrix::I();
    finalMatrix.postScale(fitScale, fitScale);

    _root->setMatrix(finalMatrix);
}

void SeatLayerTree::onVisible(bool visible) {
    if (_root) {
        _root->setVisible(visible);
    }
}

void SeatLayerTree::onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {

    loadSeatIfneeded(canvas, state);

    auto surface = canvas->getSurface();
    _displayList->render(surface, false);
}

};  // namespace kk::drawers
