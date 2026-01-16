//
//  SeatOverlayLayerTree.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "SeatOverlayLayerTree.hpp"

#include "core/FontManager.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/layers/SeatTextLayer.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/utils/UnitConverter.hpp"

#include <algorithm>
#include <cmath>

#include <tgfx/core/Canvas.h>
#include <tgfx/core/TextBlob.h>
#include <tgfx/layers/DisplayList.h>
#include <tgfx/layers/ImageLayer.h>
#include <tgfx/layers/ShapeLayer.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>
#include <tgfx/svg/TextShaper.h>

namespace kk::drawers {
SeatOverlayLayerTree::SeatOverlayLayerTree()
    : kk::drawers::Drawer("SeatOverlayLayerTree")
    , _root(nullptr)
    , _lineBox(nullptr)
    , _areaCacheImage(nullptr) {

    _inset = kk::EdgeInsets{
        kk::utils::vp2px(10.0f),
        kk::utils::vp2px(10.0f),
        kk::utils::vp2px(10.0f),
        kk::utils::vp2px(10.0f)};

    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    _displayList.setRenderMode(tgfx::RenderMode::Direct);
}

SeatOverlayLayerTree::~SeatOverlayLayerTree() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void SeatOverlayLayerTree::setBaseMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &baseMapSize) {
    if (_baseMapLayer != nullptr && _baseMapLayer == layer) {
        return;
    }

    _baseMapLayer = std::move(layer);
    if (_root != nullptr) {
        _root->removeFromParent();
        _root = nullptr;
    }
    _rebuildAreaCacheImage = true;

    _baseMapSize = baseMapSize;
}

void SeatOverlayLayerTree::setBackVisible(bool visible) {
    if (_backLayer) {
        _backLayer->setVisible(visible);
    }
    _backVisible = visible;
}

void SeatOverlayLayerTree::setBackAlpha(float alpha) {
    auto clamped = std::clamp(alpha, 0.0f, 1.0f);
    if (std::abs(_backAlpha - clamped) < 0.001f) {
        return;
    }
    _backAlpha = clamped;

    if (_backLayer) {
        _backLayer->setAlpha(_backAlpha);
    }
}

void SeatOverlayLayerTree::setMinimapVisible(bool visible) {
    if (_minimapContainer) {
        _minimapContainer->setVisible(visible);
    }
    _minimapVisible = visible;
}

void SeatOverlayLayerTree::setMinimapAlpha(float alpha) {
    auto clamped = std::clamp(alpha, 0.0f, 1.0f);
    if (std::abs(_minimapAlpha - clamped) < 0.001f) {
        return;
    }
    _minimapAlpha = clamped;

    if (_minimapContainer) {
        _minimapContainer->setAlpha(_minimapAlpha);
    }
}

bool SeatOverlayLayerTree::hitTestInBack(const tgfx::Point &location) const {
    if (!_backLayer) {
        return false;
    }
    if (!_backLayer->visible()) {
        return false;
    }
    auto hitTest = _backLayer->hitTestPoint(location.x, location.y);
    return hitTest;
}

bool SeatOverlayLayerTree::hitTestInMinimap(const tgfx::Point &location) const {
    if (!_minimapContainer) {
        return false;
    }
    if (!_minimapContainer->visible()) {
        return false;
    }
    auto hitTest = _minimapContainer->hitTestPoint(location.x, location.y);
    return hitTest;
}

void SeatOverlayLayerTree::invalidateAreaCacheImage() {
    _rebuildAreaCacheImage = true;
}

bool SeatOverlayLayerTree::hasContentChanged() const {
    if (_root == nullptr) {
        return true;
    }
    return _displayList.hasContentChanged();
}

void SeatOverlayLayerTree::prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) {

    bool rebuild = updateContainerSize(state);
    auto viewSize = state->getBoundsSize();
    if (_viewSize != viewSize) {
        _viewSize = viewSize;
        rebuild = true;
    }

    if (rebuild || _rebuildAreaCacheImage) {
        prebuildAreaImage(canvas, state);
        _rebuildAreaCacheImage = false;
        if (!rebuild && _imageLayer) {
            _imageLayer->setImage(_areaCacheImage);
        }
    }

    if (rebuild) {
        if (_lineBox) {
            _lineBox->removeFromParent();
            _lineBox = nullptr;
        }

        if (_imageLayer) {
            _imageLayer->removeFromParent();
            _imageLayer = nullptr;
        }

        if (_minimapContainer) {
            _minimapContainer->removeFromParent();
            _minimapContainer = nullptr;
        }

        if (_backLayer) {
            _backLayer->removeFromParent();
            _backLayer = nullptr;
        }

        if (_root) {
            _root->removeFromParent();
            _root = nullptr;
        }
    }

    updateLineBox(state);
}

void SeatOverlayLayerTree::updateLineBox(const kk::renderer::SeatCanvasCoreRendererState *state) {
    if (_lineBox == nullptr) {
        return;
    }
    auto viewSize = state->getBoundsSize();
    auto normalizedContentSize = state->getNormalizedContentSize();
    if (viewSize.isEmpty() || normalizedContentSize.isEmpty()) {
        return;
    }

    // 计算考虑 inset 后的 minimapSize
    tgfx::Size minimapSize{
        _containerSize.width - _inset.left - _inset.right,
        _containerSize.height - _inset.top - _inset.bottom,
    };

    auto zoomScale = state->getZoomScale();
    auto contentOffset = state->getContentOffset();

    float rectWidth = viewSize.width / normalizedContentSize.width / zoomScale * minimapSize.width;
    float rectHeight = viewSize.height / normalizedContentSize.height / zoomScale * minimapSize.height;

    // 计算指示框在 minimap 中的位置（考虑 _inset）
    float rectX = -contentOffset.x / normalizedContentSize.width / zoomScale * minimapSize.width + _inset.left;
    float rectY = -contentOffset.y / normalizedContentSize.height / zoomScale * minimapSize.height + _inset.top;

    // 边界处理
    if (rectWidth > minimapSize.width) {
        rectWidth = minimapSize.width;
        rectX = _inset.left;
    } else {
        rectWidth = std::max(kk::utils::vp2px(6.0f), rectWidth);
        rectX = std::max(_inset.left, std::min(rectX, minimapSize.width - rectWidth + _inset.left));
    }

    if (rectHeight > minimapSize.height) {
        rectHeight = minimapSize.height;
        rectY = _inset.top;
    } else {
        rectHeight = std::max(kk::utils::vp2px(6.0f), rectHeight);
        rectY = std::max(_inset.top, std::min(rectY, minimapSize.height - rectHeight + _inset.top));
    }

    auto mapRect = tgfx::Rect::MakeXYWH(rectX, rectY, rectWidth, rectHeight);
    tgfx::Path rectPath{};
    rectPath.addRect(mapRect);
    _lineBox->setPath(rectPath);
}

bool SeatOverlayLayerTree::updateContainerSize(const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto normalizedContentSize = state->getNormalizedContentSize();
    float width = std::ceil(kk::utils::vp2px(160.0));
    tgfx::Size newSize{width, width};
    if (!normalizedContentSize.isEmpty()) {
        newSize.height = std::ceil(normalizedContentSize.height / normalizedContentSize.width * newSize.width);
    }

    if (_containerSize == newSize) {
        return false;
    }

    _containerSize = newSize;
    return true;
}

bool SeatOverlayLayerTree::prebuildAreaImage(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {
    if (_baseMapLayer == nullptr) {
        _areaCacheImage = nullptr;
        return false;
    }

    auto surfaceWidth = static_cast<int>(std::ceil(_containerSize.width));
    auto surfaceHeight = static_cast<int>(std::ceil(_containerSize.height));
    auto context = canvas->getSurface()->getContext();
    auto surface = tgfx::Surface::Make(context, surfaceWidth, surfaceHeight, tgfx::ColorType::RGBA_8888);
    auto scaleX = static_cast<float>(surfaceWidth) / _baseMapSize.width;
    auto scaleY = static_cast<float>(surfaceHeight) / _baseMapSize.height;
    auto fitScale = std::min(scaleX, scaleY);
    _baseMapLayer->setMatrix(tgfx::Matrix::MakeScale(fitScale));
    tgfx::DisplayList displayList{};
    displayList.root()->addChild(_baseMapLayer);
    displayList.render(surface.get(), true);
    _areaCacheImage = surface->makeImageSnapshot();
    return true;
}

std::shared_ptr<tgfx::Layer> SeatOverlayLayerTree::buildLayerTree(const kk::renderer::SeatCanvasCoreRendererState *state) {
    if (_areaCacheImage == nullptr) {
        return nullptr;
    }

    auto root = tgfx::Layer::Make();

    _backLayer = buildBackLayer(state);
    if (_backLayer) {
        _backLayer->setVisible(_backVisible);
        _backLayer->setAlpha(_backAlpha);
        root->addChild(_backLayer);
    }

    _minimapContainer = tgfx::ShapeLayer::Make();
    tgfx::Path mimimapPath{};
    auto radius = kk::utils::vp2px(6);
    mimimapPath.addRoundRect(tgfx::Rect::MakeSize(_containerSize), radius, radius);
    _minimapContainer->setPath(mimimapPath);
    _minimapContainer->setFillStyle(tgfx::ShapeStyle::Make(tgfx::Color{0.0f, 0.0f, 0.0f, 0.45f}));
    _minimapContainer->setVisible(_minimapVisible);
    _minimapContainer->setAlpha(_minimapAlpha);

    if (_areaCacheImage) {
        _imageLayer = tgfx::ImageLayer::Make();
        _imageLayer->setImage(_areaCacheImage);

        tgfx::Size minimapSize{
            _containerSize.width - _inset.left - _inset.right,
            _containerSize.height - _inset.top - _inset.bottom,
        };
        const float scaleX = minimapSize.width / static_cast<float>(_areaCacheImage->width());
        const float scaleY = minimapSize.height / static_cast<float>(_areaCacheImage->height());
        auto matrix = tgfx::Matrix::MakeScale(scaleX, scaleY);
        matrix.postTranslate(_inset.left, _inset.right);
        _imageLayer->setMatrix(matrix);
    }

    _lineBox = tgfx::ShapeLayer::Make();

    _lineBox->setStrokeAlign(tgfx::StrokeAlign::Inside);
    _lineBox->setStrokeStyle(tgfx::ShapeStyle::Make(tgfx::Color::Red()));
    _lineBox->setLineWidth(kk::utils::vp2px(2.0f));

    updateLineBox(state);
    _minimapContainer->addChild(_imageLayer);
    _minimapContainer->addChild(_lineBox);

    auto canvasBounds = state->getBoundsSize();
    auto transX = canvasBounds.width - _containerSize.width - _inset.right;
    auto matrix = tgfx::Matrix::MakeTrans(transX, _inset.top);
    _minimapContainer->setMatrix(matrix);

    root->addChild(_minimapContainer);
    return root;
}

std::shared_ptr<tgfx::ShapeLayer> SeatOverlayLayerTree::buildBackLayer(const kk::renderer::SeatCanvasCoreRendererState *state) {
    auto canvasBounds = state->getBoundsSize();
    if (canvasBounds.isEmpty()) {
        return nullptr;
    }

    auto fallbackTypefaces = FontManager::GetFallbackTypefaces();
    auto shaper = tgfx::TextShaper::Make(std::move(fallbackTypefaces));
    if (!shaper) {
        return nullptr;
    }

    auto textBlob = shaper->shape("返回全局", nullptr, kk::utils::fp2px(12));
    if (!textBlob) {
        return nullptr;
    }

    // 多套一层是为了扩大点击区域
    auto container = tgfx::ShapeLayer::Make();
    auto layer = tgfx::ShapeLayer::Make();

    kk::EdgeInsets textInset{
        kk::utils::vp2px(6.0f),
        kk::utils::vp2px(10.0f),
        kk::utils::vp2px(6.0f),
        kk::utils::vp2px(10.0f)};

    auto textBounds = textBlob->getTightBounds();
    auto backWidth = std::ceil(textInset.horizontal() + textBounds.width());
    auto backHeight = std::ceil(textInset.vertical() + textBounds.height());

    auto containerWidth = backWidth + _inset.right * 2.0f;
    auto containerHeight = backHeight + _inset.top * 2.0f;

    tgfx::Path path;
    path.addRect(tgfx::Rect::MakeXYWH(0.0f, 0.0f, containerWidth, containerHeight));
    container->setPath(path);

    path.reset();
    path.addRoundRect(tgfx::Rect::MakeXYWH(0.0f, 0.0f, backWidth, backHeight), backHeight * 0.5f, backHeight * 0.5f);
    layer->setPath(path);
    layer->setFillStyle(tgfx::ShapeStyle::Make(tgfx::Color{0.0f, 0.0f, 0.0f, 0.45f}));

    auto textLayer = kk::layer::SeatTextLayer::Make();
    textLayer->setTextBlob(std::move(textBlob));
    textLayer->setMatrix(tgfx::Matrix::MakeTrans(textInset.left, textInset.top - textBounds.top));
    textLayer->setTextColor(tgfx::Color::White());
    layer->addChild(textLayer);

    layer->setMatrix(tgfx::Matrix::MakeTrans(_inset.right, _inset.top));

    container->addChild(layer);
    container->setMatrix(tgfx::Matrix::MakeTrans(canvasBounds.width - containerWidth, 0));

    return container;
}

void SeatOverlayLayerTree::onVisible(bool visible) {
    if (_root) {
        _root->setVisible(visible);
    }
}

void SeatOverlayLayerTree::onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) {

    if (_root == nullptr) {
        _root = buildLayerTree(state);
        _displayList.root()->addChild(_root);
    }

    auto surface = canvas->getSurface();
    _displayList.render(surface, false);
}
};  // namespace kk::drawers
