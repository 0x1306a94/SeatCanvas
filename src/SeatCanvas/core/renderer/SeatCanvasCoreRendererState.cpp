//
//  SeatCanvasCoreRendererState.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "SeatCanvasCoreRendererState.hpp"

#include "core/Log.hpp"
#include <algorithm>

namespace kk::renderer {
SeatCanvasCoreRendererState::SeatCanvasCoreRendererState(int width, int height, float density)
    : width(width)
    , height(height)
    , density(density) {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

SeatCanvasCoreRendererState::~SeatCanvasCoreRendererState() {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

tgfx::Size SeatCanvasCoreRendererState::getBoundsSize() const {
    return tgfx::Size::Make({width, height});
}

tgfx::Size SeatCanvasCoreRendererState::getNormalizedContentSize() const {
    return normalizedContentSize;
}

tgfx::Size SeatCanvasCoreRendererState::getOriginSize() const {
    return originSize;
}

float SeatCanvasCoreRendererState::getDensity() const {
    return density;
}

float SeatCanvasCoreRendererState::getContentScale() const {
    return contentScale;
}

float SeatCanvasCoreRendererState::getZoomScale() const {
    return zoomScale;
}

const tgfx::Point &SeatCanvasCoreRendererState::getContentOffset() const {
    return contentOffset;
}

tgfx::Rect SeatCanvasCoreRendererState::getVisibleOriginalRect() const {
    // 获取状态信息
    auto viewport = getBoundsSize();
    auto normalizedContentSize = getNormalizedContentSize();
    auto baseMapSize = getOriginSize();
    auto zoomScale = getZoomScale();
    auto contentOffset = getContentOffset();
    auto density = getDensity();
    auto contentScale = getContentScale();

    if (viewport.isEmpty() || normalizedContentSize.isEmpty() || baseMapSize.isEmpty() || zoomScale <= 0.0f) {
        return tgfx::Rect::MakeEmpty();
    }

    // 屏幕可见区域（假设 contentInset 为 0，简化处理）
    float screenLeft = 0.0f;
    float screenTop = 0.0f;
    float screenRight = viewport.width;
    float screenBottom = viewport.height;

    // 转换为规范化内容坐标系中的位置（像素单位）
    // contentX = (screenX - contentOffset.x) / zoomScale
    float contentLeft = (screenLeft - contentOffset.x) / zoomScale;
    float contentTop = (screenTop - contentOffset.y) / zoomScale;
    float contentRight = (screenRight - contentOffset.x) / zoomScale;
    float contentBottom = (screenBottom - contentOffset.y) / zoomScale;

    // 转换为原始坐标系（zone.bounds 使用的是原始坐标）
    // normalizedContentSize = baseMapSize * contentScale * density
    // 所以 originalCoord = contentCoord / (contentScale * density)
    float scaleFactor = contentScale * density;

    float originalContentWidth = normalizedContentSize.width / scaleFactor;
    float originalContentHeight = normalizedContentSize.height / scaleFactor;

    float minX = std::max(0.0f, std::min(contentLeft, contentRight)) / scaleFactor;
    float minY = std::max(0.0f, std::min(contentTop, contentBottom)) / scaleFactor;
    float maxX = std::min(originalContentWidth, std::max(contentLeft, contentRight) / scaleFactor);
    float maxY = std::min(originalContentHeight, std::max(contentTop, contentBottom) / scaleFactor);

    if (maxX <= minX || maxY <= minY) {
        return tgfx::Rect::MakeEmpty();
    }

    return tgfx::Rect::MakeLTRB(minX, minY, maxX, maxY);
}

tgfx::Matrix SeatCanvasCoreRendererState::getMVMatrix() const {
    auto baseMapSize = getOriginSize();
    auto normalizedContentSize = getNormalizedContentSize();
    auto zoomScale = getZoomScale();
    auto contentOffset = getContentOffset();

    if (normalizedContentSize.isEmpty() || baseMapSize.isEmpty() || zoomScale <= 0.0f) {
        return tgfx::Matrix::I();
    }

    // Model: 原始坐标 → 规范化内容坐标
    // 顶点数据是原始坐标，需要缩放到规范化内容尺寸
    float originalToNormalizedX = normalizedContentSize.width / baseMapSize.width;
    float originalToNormalizedY = normalizedContentSize.height / baseMapSize.height;
    tgfx::Matrix modelMatrix = tgfx::Matrix::MakeScale(originalToNormalizedX, originalToNormalizedY);

    // View: 规范化内容坐标 → 屏幕坐标
    // 先缩放 (zoomScale)，再平移 (contentOffset)
    tgfx::Matrix viewMatrix = tgfx::Matrix::MakeScale(zoomScale, zoomScale);
    viewMatrix.postTranslate(contentOffset.x, contentOffset.y);

    // MV = View × Model
    tgfx::Matrix mvMatrix{tgfx::Matrix::I()};
    mvMatrix.postConcat(modelMatrix);
    mvMatrix.postConcat(viewMatrix);
    return mvMatrix;
}

tgfx::Matrix SeatCanvasCoreRendererState::getMVPMatrix() const {
    auto viewport = getBoundsSize();
    if (viewport.isEmpty() || getNormalizedContentSize().isEmpty() || getOriginSize().isEmpty() || getZoomScale() <= 0.0f) {
        return tgfx::Matrix::I();
    }

    auto mvMatrix = getMVMatrix();

    // Projection: 屏幕坐标 → NDC [-1, 1]
    // x' = x * (2/width) - 1
    // y' = -y * (2/height) + 1  (Y轴翻转，屏幕Y向下，NDC Y向上)
    tgfx::Matrix projectionMatrix = tgfx::Matrix::MakeAll(
        2.0f / viewport.width, 0.0f, -1.0f,
        0.0f, -2.0f / viewport.height, 1.0f);

    // MVP = Projection × MV
    mvMatrix.postConcat(projectionMatrix);
    return mvMatrix;
}

bool SeatCanvasCoreRendererState::updateNormalizedContentSize(const tgfx::Size &normalizedContentSize) {
    if (normalizedContentSize.width <= 0 || normalizedContentSize.height <= 0) {
        SC_LOG_ERROR("%s width or height is invalid!", __PRETTY_FUNCTION__);
        return false;
    }

    if (this->normalizedContentSize == normalizedContentSize) {
        return false;
    }
    this->normalizedContentSize = normalizedContentSize;
    return true;
}

bool SeatCanvasCoreRendererState::updateOriginSize(const tgfx::Size &originSize) {
    this->originSize = originSize;
    return true;
}

bool SeatCanvasCoreRendererState::updateContentScale(float scale) {
    contentScale = scale;
    return true;
}

bool SeatCanvasCoreRendererState::updateScreen(int width, int height, float density) {
    if (width <= 0 || height <= 0) {
        SC_LOG_ERROR("%s width or height is invalid!", __PRETTY_FUNCTION__);
        return false;
    }
    if (density < 1.0) {
        SC_LOG_ERROR("%s density is invalid!", __PRETTY_FUNCTION__);
        return false;
    }
    if (this->width == width && this->height == height && this->density == density) {
        return false;
    }
    this->width = width;
    this->height = height;
    this->density = density;
    return true;
}

bool SeatCanvasCoreRendererState::updateZoomAndOffset(float zoomScale, const tgfx::Point &contentOffset) {
    if (this->zoomScale == zoomScale && this->contentOffset == contentOffset) {
        return false;
    }
    this->zoomScale = zoomScale;
    this->contentOffset = contentOffset;
    return true;
}

};  // namespace kk::renderer
