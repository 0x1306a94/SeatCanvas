//
//  SeatCanvasCoreRendererState.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "SeatCanvasCoreRendererState.hpp"

#include <tgfx/platform/Print.h>

namespace kk::renderer {
SeatCanvasCoreRendererState::SeatCanvasCoreRendererState(int width, int height, float density)
    : _width(width)
    , _height(height)
    , _density(density) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

SeatCanvasCoreRendererState::~SeatCanvasCoreRendererState() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

tgfx::Size SeatCanvasCoreRendererState::getBoundsSize() const {
    return tgfx::Size::Make({_width, _height});
}

tgfx::Size SeatCanvasCoreRendererState::getContentSize() const {
    return _contentSize;
}

/// svg原始尺寸
tgfx::Size SeatCanvasCoreRendererState::getOriginSize() const {
    return _originSize;
}

float SeatCanvasCoreRendererState::density() const {
    return _density;
}

/// 当前缩放比例
float SeatCanvasCoreRendererState::zoomScale() const {
    return _zoomScale;
}

/// 当前滑动偏移
const tgfx::Point &SeatCanvasCoreRendererState::contentOffset() const {
    return _contentOffset;
}

bool SeatCanvasCoreRendererState::updateContentSize(const tgfx::Size &contentSize) {
    if (contentSize.width <= 0 || contentSize.height <= 0) {
        tgfx::PrintError("%s width or height is invalid!", __PRETTY_FUNCTION__);
        return false;
    }

    if (contentSize == _contentSize) {
        return false;
    }
    _contentSize = contentSize;
    return true;
}

bool SeatCanvasCoreRendererState::updateOriginSize(const tgfx::Size &originSize) {
    _originSize = originSize;
    return true;
}

bool SeatCanvasCoreRendererState::updateScreen(int width, int height, float density) {
    if (width <= 0 || height <= 0) {
        tgfx::PrintError("%s width or height is invalid!", __PRETTY_FUNCTION__);
        return false;
    }
    if (density < 1.0) {
        tgfx::PrintError("%s density is invalid!", __PRETTY_FUNCTION__);
        return false;
    }
    if (width == _width && height == _height && density == _density) {
        return false;
    }
    _width = width;
    _height = height;
    _density = density;
    return true;
}

bool SeatCanvasCoreRendererState::updateZoomAndOffset(float zoomScale, const tgfx::Point &contentOffset) {
    if (zoomScale == _zoomScale && contentOffset == _contentOffset) {
        return false;
    }
    _zoomScale = zoomScale;
    _contentOffset = contentOffset;
    return true;
}

};  // namespace kk::renderer
