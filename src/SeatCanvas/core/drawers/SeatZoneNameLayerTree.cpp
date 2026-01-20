//
//  SeatZoneNameLayerTree.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "SeatZoneNameLayerTree.hpp"

#include "core/layers/SeatZoneLayer.hpp"
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

SeatZoneNameLayerTree::SeatZoneNameLayerTree()
    : kk::drawers::Drawer("SeatZoneNameLayerTree")
    , _root(nullptr)
    , _textRootLayer(nullptr)
    , _displayList(std::make_unique<tgfx::DisplayList>()) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);

    _displayList->setRenderMode(tgfx::RenderMode::Partial);
};

SeatZoneNameLayerTree::~SeatZoneNameLayerTree() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void SeatZoneNameLayerTree::setTextRootLayer(std::shared_ptr<tgfx::Layer> layer, const tgfx::Size &baseMapSize) {
    if (_textRootLayer != nullptr && _textRootLayer == layer) {
        return;
    }

    _textRootLayer = std::move(layer);
    _baseMapSize = baseMapSize;
    _rebuildLayer = true;
}

tgfx::Point SeatZoneNameLayerTree::globalToLocal(const tgfx::Point &globalPoint) const {
    if (_root == nullptr) {
        return tgfx::Point::Zero();
    }
    auto local = _root->globalToLocal(globalPoint);
    return local;
}

tgfx::Point SeatZoneNameLayerTree::localToGlobal(const tgfx::Point &localPoint) const {
    if (_root == nullptr) {
        return tgfx::Point::Zero();
    }
    auto global = _root->localToGlobal(localPoint);
    return global;
}

bool SeatZoneNameLayerTree::hasContentChanged() const {
    return _displayList->hasContentChanged();
}

void SeatZoneNameLayerTree::prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) {

    if (updateContentSize(state) || force || _rebuildLayer) {
        buildLayerTree(canvas, state);
    }

    auto zoomScale = state->getZoomScale();
    auto contentOffset = state->getContentOffset();
    _displayList->setZoomScale(zoomScale);
    _displayList->setContentOffset(contentOffset.x, contentOffset.y);
}

bool SeatZoneNameLayerTree::updateContentSize(const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto normalizedContentSize = state->getNormalizedContentSize();
    if (_contentSize == normalizedContentSize) {
        return false;
    }

    _contentSize = normalizedContentSize;
    return true;
}

void SeatZoneNameLayerTree::buildLayerTree(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {

    if (_root) {
        _root->removeFromParent();
        _root = nullptr;
    }

    if (_textRootLayer == nullptr) {
        return;
    }

    _root = tgfx::Layer::Make();

    _root->addChild(_textRootLayer);
    _root->setVisible(visible());

    updateRootMatrix(canvas, state);

    _displayList->root()->addChild(_root);

    _rebuildLayer = false;
}

void SeatZoneNameLayerTree::updateRootMatrix(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto limitSize = state->getNormalizedContentSize();
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

void SeatZoneNameLayerTree::onVisible(bool visible) {
    if (_root) {
        _root->setVisible(visible);
    }
}

void SeatZoneNameLayerTree::onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto surface = canvas->getSurface();
    _displayList->render(surface, false);
}

};  // namespace kk::drawers
