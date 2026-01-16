//
//  ElasticZoomPanController.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "ElasticZoomPanController.hpp"

#include "PanProxy.hpp"
#include "ScrollProperties.hpp"
#include "Scroller.hpp"
#include "SpringBack.hpp"
#include "VelocityTracker.hpp"
#include "core/Platform.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include <tgfx/platform/Print.h>

namespace kk::gesture {

template <typename T>
static T signum(T x) {
    if (x > 0) {
        return 1;
    } else if (x < 0) {
        return -1;
    } else {
        return 0;
    }
}

ElasticZoomPanController::ElasticZoomPanController() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);

    _panProxy = std::make_unique<PanProxy>();
    _panProxy->setCallback([this] {
        this->handlePanProxy();
    });

    _scrollPropertiesX = std::make_unique<ScrollProperties>();
    _scrollPropertiesY = std::make_unique<ScrollProperties>();
    _zoomScaleProperties = std::make_unique<ScrollProperties>();
}

ElasticZoomPanController::~ElasticZoomPanController() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void ElasticZoomPanController::setBounds(const tgfx::Size &bounds) {
    _bounds = bounds;
    updateOffsetBounds();
}

const tgfx::Size &ElasticZoomPanController::getBounds() const {
    return _bounds;
}

void ElasticZoomPanController::setContentSize(const tgfx::Size &contentSize) {
    _contentSize = contentSize;
    updateOffsetBounds();
}

const tgfx::Size &ElasticZoomPanController::getContentSize() const {
    return _contentSize;
}

void ElasticZoomPanController::setContentInset(const EdgeInsets &contentInset) {
    _contentInset = contentInset;
    updateOffsetBounds();
}

const EdgeInsets &ElasticZoomPanController::getContentInset() const {
    return _contentInset;
}

void ElasticZoomPanController::setMinimumZoomScale(float minimumZoomScale) {
    _minimumZoomScale = minimumZoomScale;
}

float ElasticZoomPanController::getMinimumZoomScale() const {
    return _minimumZoomScale;
}

void ElasticZoomPanController::setMaximumZoomScale(float maximumZoomScale) {
    _maximumZoomScale = maximumZoomScale;
}

float ElasticZoomPanController::getMaximumZoomScale() const {
    return _maximumZoomScale;
}

void ElasticZoomPanController::setZoomScale(float zoomScale, bool revalidate) {
    _zoomScale = std::clamp(zoomScale, _minimumZoomScale, _maximumZoomScale);
    updateOffsetBounds();
    if (revalidate) {
        revalidateContentOffset();
    }
}

float ElasticZoomPanController::getZoomScale() const {
    return _zoomScale;
}

void ElasticZoomPanController::setContentOffset(const tgfx::Point &contentOffset, bool revalidate) {
    _contentOffset = contentOffset;
    if (revalidate) {
        revalidateContentOffset();
    }
}

const tgfx::Point &ElasticZoomPanController::getContentOffset() const {
    return _contentOffset;
}

// --- Transformation Matrix ---
tgfx::Matrix ElasticZoomPanController::getMatrix() const {
    tgfx::Matrix matrix;
    matrix.setScale(_zoomScale, _zoomScale);
    matrix.preTranslate(_contentOffset.x, _contentOffset.y);
    return matrix;
}

// --- Gesture Handlers (Simulating User Input) ---
void ElasticZoomPanController::handlePan(kk::gesture::GestureState state, const tgfx::Point &translation, double timestampMs) {
    _panProxy->handle(state, translation, timestampMs);
}

void ElasticZoomPanController::handlePinch(kk::gesture::GestureState state, float scale, const tgfx::Point &center) {
    switch (state) {
        case GestureState::BEGAN: {
            // 手势开始，记录下所有初始状态作为计算基准
            _pinchStartZoomScale = _zoomScale;
            // 重置惯性/回弹滑动
            _scrollPropertiesX->clear();
            _scrollPropertiesY->clear();
            _zoomScaleProperties->clear();
            break;
        }

        case GestureState::CHANGED: {
            float logicalScale = _pinchStartZoomScale * scale;
            float visualScale = zoomScaleForRubberBandScale(logicalScale);

            float contentPointX = (center.x - _contentOffset.x) / _zoomScale;
            float contentPointY = (center.y - _contentOffset.y) / _zoomScale;

            _zoomScale = visualScale;
            updateOffsetBounds();
            float newOffsetX = center.x - contentPointX * _zoomScale;
            float newOffsetY = center.y - contentPointY * _zoomScale;

            _contentOffset.x = newOffsetX;
            _contentOffset.y = newOffsetY;

            break;
        }

        case GestureState::ENDED:
        case GestureState::CANCELLED: {
            float contentPointX = (center.x - _contentOffset.x) / _zoomScale;
            float contentPointY = (center.y - _contentOffset.y) / _zoomScale;

            auto clampZoomScale = std::clamp(_zoomScale, _minimumZoomScale, _maximumZoomScale);

            // 检查是否需要回弹（参考 UIScrollView 的实现）
            if (std::abs(clampZoomScale - _zoomScale) > FLT_EPSILON) {
                // 缩放比例超出边界，需要回弹
                // 保存当前的内容点和中心点，用于回弹过程中保持触摸点位置
                _pinchEndContentPoint = {contentPointX, contentPointY};
                _pinchEndCenter = center;

                // 准备回弹（不依赖 velocity，基于超出边界的距离）
                prepareBouncingForZoomScale(clampZoomScale);
            } else {
                // 缩放比例在边界内，直接设置
                _zoomScale = clampZoomScale;
                updateOffsetBounds();

                float newOffsetX = center.x - contentPointX * _zoomScale;
                float newOffsetY = center.y - contentPointY * _zoomScale;

                _contentOffset.x = newOffsetX;
                _contentOffset.y = newOffsetY;
                revalidateContentOffset();
            }
            break;
        }

        default: {
            break;
        }
    }
}

bool ElasticZoomPanController::handleDisplayLinkFire() {
    // 处理水平方向的动画（惯性滑动和回弹）
    auto horizontalChanged = handleDisplayLinkFireForAxis(_scrollPropertiesX.get(), true);

    // 处理垂直方向的动画（惯性滑动和回弹）
    auto verticalChanged = handleDisplayLinkFireForAxis(_scrollPropertiesY.get(), false);

    // 处理缩放的回弹动画
    auto zoomChanged = handleDisplayLinkFireForZoomScale();

    // 如果任何方向的动画仍在进行中，返回 true，表示需要继续更新
    return horizontalChanged || verticalChanged || zoomChanged;
}

// --- Internal Logic ---
void ElasticZoomPanController::handlePanProxy() {
    auto state = _panProxy->state();
    const auto viewportSize = _bounds;
    const auto offsetBounds = getOffsetBounds();
    const auto minContentOffset = tgfx::Point{offsetBounds.left, offsetBounds.top};
    const auto maxContentOffset = tgfx::Point{offsetBounds.right, offsetBounds.bottom};
    switch (state) {
        case GestureState::BEGAN: {
            _isTracking = true;
            _isDragging = false;

            // 重置状态
            _scrollPropertiesX->clear();
            _scrollPropertiesY->clear();
            _zoomScaleProperties->clear();

            _touchBeganTranslation = std::nullopt;

            _lastContentOffset.x = rubberBandForOffset(_contentOffset.x, minContentOffset.x, maxContentOffset.x, viewportSize.width, true);
            _lastContentOffset.y = rubberBandForOffset(_contentOffset.y, minContentOffset.y, maxContentOffset.y, viewportSize.height, true);

            break;
        }
        case GestureState::CHANGED: {
            _isTracking = false;
            _isDragging = true;

            auto translation = _panProxy->translation();
            if (!_touchBeganTranslation.has_value()) {
                _touchBeganTranslation = translation;
            }

            translation = translation - *_touchBeganTranslation;
            const auto x = canHorizontalScroll() ? translation.x : 0;
            const auto y = canVerticalScroll() ? translation.y : 0;
            auto targetContentOffsetX = _lastContentOffset.x - x;
            auto targetContentOffsetY = _lastContentOffset.y - y;

            targetContentOffsetX = rubberBandForOffset(targetContentOffsetX, minContentOffset.x, maxContentOffset.x, viewportSize.width, false);
            targetContentOffsetY = rubberBandForOffset(targetContentOffsetY, minContentOffset.y, maxContentOffset.y, viewportSize.height, false);
            _contentOffset.x = targetContentOffsetX;
            _contentOffset.y = targetContentOffsetY;
            break;
        }
        case GestureState::ENDED:
        case GestureState::CANCELLED: {
            _isDragging = false;
            auto velocity = _panProxy->velocity();
            if (VelocityTracker::ApproachingHalt(velocity.x, velocity.y)) {
                velocity = tgfx::Point::Zero();
            }

            if (!canHorizontalScroll()) {
                velocity.x = 0.0;
            }

            if (!canVerticalScroll()) {
                velocity.y = 0.0;
            }

            // 惯性/回弹
            handleEndPanWithVelocity(velocity);
            break;
        }
        default:
            break;
    }
}

bool ElasticZoomPanController::handleDisplayLinkFireForAxis(ScrollProperties *properties, bool horizontal) {
    if (properties == nullptr) {
        return false;
    }

    // 记录处理前的动画状态，用于判断这一帧是否有变化
    bool hadAnimation = properties->is_decelerating || properties->is_bouncing;

    const auto mediaTime = Platform::Current()->currentMediaTime();

    if (properties->is_decelerating) {
        const auto interval = (mediaTime - properties->animation_begin_time);
        float velocity = 0.0;
        properties->is_decelerating = !handleDeceleratingWithIntervalForAxis(properties, interval, velocity, horizontal);
        const auto overflow = overflowOffsetForAxis(horizontal);
        if (properties->is_decelerating && overflow != 0.0) {
            properties->is_decelerating = false;
            prepareBouncingWithVelocityForAxis(properties, velocity, false, horizontal);
        }
    }

    if (properties->is_bouncing) {
        const auto interval = (mediaTime - properties->animation_begin_time);
        properties->is_bouncing = !handleBouncingWithIntervalForAxis(properties, interval, horizontal);

        /*
        if (!horizontal) {
            if (scrollsToTopCallback) {
                const auto offset = offsetForAxis(horizontal);
                const auto minOffset = minimumOffsetForAxis(horizontal);
                // When performing the scroll-to-top animation, it is necessary to allow some extra time
                // for the navigation bar to complete its alpha animation.
                if (std::abs(offset - minOffset) <= 2) {
                    // call scrollsToTo
                }
            }

            if (!properties->is_bouncing) {
                // ignoreScrollObserver = false
            }
        }
         */
    }

    // 返回 true 表示这一帧有状态变化（包括动画完成的情况）
    // 这样确保最后一帧的状态变化也能被正确处理
    bool hasAnimation = properties->is_decelerating || properties->is_bouncing;
    return hadAnimation || hasAnimation;
}

void ElasticZoomPanController::handleEndPanWithVelocity(const tgfx::Point &velocity) {
    handleEndPanWithVelocityForAxis(_scrollPropertiesX.get(), velocity.x, true);
    handleEndPanWithVelocityForAxis(_scrollPropertiesY.get(), velocity.y, false);
}

void ElasticZoomPanController::handleEndPanWithVelocityForAxis(ScrollProperties *properties, float velocity, bool horizontal) {
    if (properties == nullptr) {
        return;
    }

    const auto overflow = overflowOffsetForAxis(horizontal);
    if (overflow != 0.0) {
        prepareBouncingWithVelocityForAxis(properties, velocity, true, horizontal);
        return;
    }

    if (velocity != 0.0) {
        const auto offset = offsetForAxis(horizontal);
        properties->prepareScroller(DecelerationRateNormal);
        properties->reset(velocity, offset);
        properties->scroller->fling(velocity);
        properties->is_decelerating = true;
    }
}

void ElasticZoomPanController::prepareBouncingWithVelocityForAxis(ScrollProperties *properties, float velocity, bool overflowVelocity, bool horizontal) {
    if (properties == nullptr) {
        return;
    }

    const auto overV = overflowOffsetForAxis(horizontal) / 100.0f;
    if (overflowVelocity) {
        if (std::signbit(overV) != std::signbit(velocity)) {
            velocity += overV;
        } else {
            velocity = overV;
        }
    }

    properties->prepareSpringBack();
    properties->reset(velocity, 0);

    const auto minOffset = minimumOffsetForAxis(horizontal);
    const auto maxContentOffset = maximumOffsetForAxis(horizontal);

    float targetOffset = 0;
    if (overV < 0) {
        properties->bounce_edge = BounceEdge::MIN;
        targetOffset = minOffset;
    } else if (overV > 0) {
        properties->bounce_edge = BounceEdge::MAX;
        targetOffset = maxContentOffset;
    }

    if (properties->bounce_edge != BounceEdge::NONE) {
        const auto offset = offsetForAxis(horizontal);
        properties->springBack->absorbWithResponse(velocity, targetOffset - offset, DefaultSpringBackResponse);
        properties->is_bouncing = true;
    }
}

bool ElasticZoomPanController::handleDeceleratingWithIntervalForAxis(ScrollProperties *properties, double interval, float &velocity, bool horizontal) {
    if (properties == nullptr) {
        return true;
    }

    bool finish = true;
    auto targetOffset = offsetForAxis(horizontal);
    float finalVelocity = 0.0;
    if (properties->scroller) {
        const auto value = properties->scroller->value(static_cast<float>(interval));
        if (value.has_value()) {
            finish = false;
            finalVelocity = value->velocity;
            targetOffset = properties->animation_begin_offset - value->offset;
        }
    }

    velocity = finalVelocity;
    setContentOffsetValueForAxis(targetOffset, horizontal);
    return finish;
}

bool ElasticZoomPanController::handleBouncingWithIntervalForAxis(ScrollProperties *properties, double interval, bool horizontal) {
    if (properties == nullptr) {
        return true;
    }

    const auto minContentOffset = minimumOffsetForAxis(horizontal);
    const auto maxContentOffset = maximumOffsetForAxis(horizontal);
    float targetContentOffset = offsetForAxis(horizontal);
    bool finish = true;
    if (properties->bounce_edge != BounceEdge::NONE) {
        const auto target = properties->bounce_edge == BounceEdge::MIN ? minContentOffset : maxContentOffset;
        const auto value = properties->springBack->value(interval);
        if (value.has_value()) {
            finish = false;
            targetContentOffset = target - value.value();
        }
    }
    setContentOffsetValueForAxis(targetContentOffset, horizontal);
    return finish;
}

void ElasticZoomPanController::setContentOffsetValueForAxis(float offset, bool horizontal) {
    if (horizontal) {
        _contentOffset.x = offset;
        return;
    }
    _contentOffset.y = offset;
}

float ElasticZoomPanController::rubberBandForOffset(float offset, float minOffset, float maxOffset, float range, bool inverse) const {
    if (std::abs(range) < FLT_EPSILON) {
        return offset;
    }

    const auto min = minOffset;
    const auto max = std::max(minOffset, maxOffset);

    if (min <= offset && offset <= max) {
        return offset;
    }

    const auto target = offset < min ? min : max;
    const auto distance = offset - target;

    const auto transformed = inverse ? CalculateRubberBandOffsetInv(std::abs(distance), range) : CalculateRubberBandOffset(std::abs(distance), range);
    const auto result = target + transformed * signum(distance);
    return result;
}

// float ElasticZoomPanController::zoomScaleForRubberBandScale(float scale) const {
//     // 参考 CalculateRubberBandOffset 的逻辑，对超出边界的缩放比例应用橡皮筋效果
//     if (scale > _maximumZoomScale) {
//         // 计算超出最大缩放比例的距离
//         float distance = scale - _maximumZoomScale;
//         // 使用最大缩放比例作为橡皮筋范围
//         float range = _maximumZoomScale;
//         // 应用橡皮筋效果
//         float rubberBandDistance = CalculateRubberBandOffset(distance, range);
//         return _maximumZoomScale + rubberBandDistance;
//     }
//
//     if (scale < _minimumZoomScale) {
//         // 计算低于最小缩放比例的距离
//         float distance = _minimumZoomScale - scale;
//         // 使用最小缩放比例作为橡皮筋范围
//         float range = _minimumZoomScale;
//         // 应用橡皮筋效果
//         float rubberBandDistance = CalculateRubberBandOffset(distance, range);
//         return _minimumZoomScale - rubberBandDistance;
//     }
//
//     return scale;
// }

float ElasticZoomPanController::zoomScaleForRubberBandScale(float scale) const {
    // 使用 UIScrollView 的缩放橡皮筋算法实现
    // 参考: -[UIScrollView _zoomRubberBandScaleForScale:]
    if (_maximumZoomScale >= scale) {
        // scale <= _maximumZoomScale
        if (_minimumZoomScale <= scale) {
            // _minimumZoomScale <= scale <= _maximumZoomScale，在边界内，直接返回
            return scale;
        } else {
            // scale < _minimumZoomScale，低于最小缩放比例，应用橡皮筋效果
            float v4 = 1.0f / (2.0f - scale / _minimumZoomScale);
            return _minimumZoomScale * v4;
        }
    } else {
        // scale > _maximumZoomScale，超过最大缩放比例，应用橡皮筋效果
        float v4 = -1.0f / (scale / _maximumZoomScale) + 2.0f;
        return _maximumZoomScale * v4;
    }
}

bool ElasticZoomPanController::canHorizontalScroll() const {
    return _alwaysBounceHorizontal || _contentSize.width > _bounds.width;
}

bool ElasticZoomPanController::canVerticalScroll() const {
    return _alwaysBounceVertical || _contentSize.height > _bounds.height;
}

bool ElasticZoomPanController::canScrollForAxis(bool horizontal) const {
    if (horizontal) {
        return canHorizontalScroll();
    }
    return canVerticalScroll();
}

float ElasticZoomPanController::overflowOffsetForAxis(bool horizontal) const {
    if (!canScrollForAxis(horizontal)) {
        return 0.0f;
    }
    const auto offset = offsetForAxis(horizontal);
    const auto minOffset = minimumOffsetForAxis(horizontal);
    const auto maxOffset = maximumOffsetForAxis(horizontal);
    if (offset < minOffset) {
        // The offset is less than the minimum offset.
        // The value is negative.
        return offset - minOffset;
    } else if (offset > maxOffset) {
        // The offset is greater than the maximum offset.
        // The value is positive.
        return offset - maxOffset;
    } else {
        // The offset is within the range of the minimum and maximum offsets.
        return 0;
    }
}

float ElasticZoomPanController::offsetForAxis(bool horizontal) const {
    if (horizontal) {
        return _contentOffset.x;
    }
    return _contentOffset.y;
}

float ElasticZoomPanController::minimumOffsetForAxis(bool horizontal) const {
    const auto offsetBounds = getOffsetBounds();
    if (horizontal) {
        return offsetBounds.left;
    }
    return offsetBounds.top;
}

float ElasticZoomPanController::maximumOffsetForAxis(bool horizontal) const {
    const auto offsetBounds = getOffsetBounds();
    if (horizontal) {
        return offsetBounds.right;
    }
    return offsetBounds.bottom;
}

const tgfx::Rect &ElasticZoomPanController::getOffsetBounds() const {
    return _cachedOffsetBounds;
}

void ElasticZoomPanController::updateOffsetBounds() {
    const float scaledWidth = _contentSize.width * _zoomScale;
    const float scaledHeight = _contentSize.height * _zoomScale;

    // 考虑 contentInset 后的视图有效边界
    const float effectiveBoundsWidth = _bounds.width - _contentInset.left - _contentInset.right;
    const float effectiveBoundsHeight = _bounds.height - _contentInset.top - _contentInset.bottom;

    // 计算水平方向的边界
    float minX, maxX;
    if (scaledWidth < effectiveBoundsWidth) {
        // 内容小于有效边界，居中对齐，并考虑 left/right inset
        minX = (_bounds.width - scaledWidth) * 0.5f;
        maxX = minX;
    } else {
        // 内容大于有效边界，允许滚动
        // 注意：平移的边界应该加上 contentInset
        minX = -scaledWidth + effectiveBoundsWidth + _contentInset.left;
        maxX = _contentInset.left;
    }

    // 计算垂直方向的边界
    float minY, maxY;
    if (scaledHeight < effectiveBoundsHeight) {
        // 内容小于有效边界，居中对齐，并考虑 top/bottom inset
        minY = (_bounds.height - scaledHeight) * 0.5f;
        maxY = minY;
    } else {
        // 内容大于有效边界，允许滚动
        minY = -scaledHeight + effectiveBoundsHeight + _contentInset.top;
        maxY = _contentInset.top;
    }

    _cachedOffsetBounds = tgfx::Rect::MakeLTRB(minX, minY, maxX, maxY);
}

void ElasticZoomPanController::revalidateContentOffset() {
    const auto offsetBounds = getOffsetBounds();
    _contentOffset.x = std::clamp(_contentOffset.x, offsetBounds.left, offsetBounds.right);
    _contentOffset.y = std::clamp(_contentOffset.y, offsetBounds.top, offsetBounds.bottom);
}

void ElasticZoomPanController::stopAllAnimations() {

    // 清除 Pan 的惯性滑动和回弹状态
    _scrollPropertiesX->clear();
    _scrollPropertiesY->clear();

    // 清除 Pinch 的回弹状态
    _zoomScaleProperties->clear();

    // 重置 PanProxy 的状态
    _panProxy->reset();

    // 重置滑动相关的状态
    _isTracking = false;
    _isDragging = false;
    _touchBeganTranslation = std::nullopt;

    revalidateContentOffset();
}

bool ElasticZoomPanController::hasPendingAnimation() const {
    // 检查是否有正在进行的动画
    if (_scrollPropertiesX && (_scrollPropertiesX->is_decelerating || _scrollPropertiesX->is_bouncing)) {
        return true;
    }
    if (_scrollPropertiesY && (_scrollPropertiesY->is_decelerating || _scrollPropertiesY->is_bouncing)) {
        return true;
    }
    if (_zoomScaleProperties && _zoomScaleProperties->is_bouncing) {
        return true;
    }
    return false;
}

bool ElasticZoomPanController::handleDisplayLinkFireForZoomScale() {
    if (_zoomScaleProperties == nullptr) {
        return false;
    }

    // 记录处理前的动画状态，用于判断这一帧是否有变化
    bool hadAnimation = _zoomScaleProperties->is_bouncing;

    const auto mediaTime = Platform::Current()->currentMediaTime();

    if (_zoomScaleProperties->is_bouncing) {
        const auto interval = (mediaTime - _zoomScaleProperties->animation_begin_time);
        _zoomScaleProperties->is_bouncing = !handleBouncingWithIntervalForZoomScale(_zoomScaleProperties.get(), interval);
    }

    // 返回 true 表示这一帧有状态变化（包括动画完成的情况）
    // 这样确保最后一帧的状态变化也能被正确处理
    bool hasAnimation = _zoomScaleProperties->is_bouncing;
    return hadAnimation || hasAnimation;
}

void ElasticZoomPanController::prepareBouncingForZoomScale(float targetScale) {
    if (_zoomScaleProperties == nullptr) {
        return;
    }

    // 确定回弹方向
    if (_zoomScale < _minimumZoomScale) {
        _zoomScaleProperties->bounce_edge = BounceEdge::MIN;
    } else if (_zoomScale > _maximumZoomScale) {
        _zoomScaleProperties->bounce_edge = BounceEdge::MAX;
    } else {
        _zoomScaleProperties->bounce_edge = BounceEdge::NONE;
        return;
    }

    // 保存回弹的起始和目标缩放比例
    _zoomBounceStartScale = _zoomScale;
    _zoomBounceTargetScale = targetScale;

    // 准备回弹动画（不使用 SpringBack，使用简单的缓动动画）
    // 参考 UIScrollView，使用简单的 ease-out 曲线，避免振荡
    _zoomScaleProperties->reset(0.0f, 0.0f);
    _zoomScaleProperties->is_bouncing = true;
}

bool ElasticZoomPanController::handleBouncingWithIntervalForZoomScale(ScrollProperties *properties, double interval) {
    if (properties == nullptr) {
        return true;
    }

    // 使用简单的 ease-out 缓动动画，避免 SpringBack 的振荡
    // 动画持续时间（秒）
    const float animationDuration = 0.25f;

    // 将时间转换为 0-1 的进度
    float progress = static_cast<float>(interval) / 1000.0f / animationDuration;

    if (progress >= 1.0f) {
        // 动画完成
        _zoomScale = _zoomBounceTargetScale;
        updateOffsetBounds();

        float newOffsetX = _pinchEndCenter.x - _pinchEndContentPoint.x * _zoomScale;
        float newOffsetY = _pinchEndCenter.y - _pinchEndContentPoint.y * _zoomScale;

        _contentOffset.x = newOffsetX;
        _contentOffset.y = newOffsetY;
        revalidateContentOffset();
        return true;
    }

    // 使用 ease-out cubic 缓动函数：1 - (1 - t)^3
    // 这会产生平滑的减速效果，没有振荡
    float easedProgress = 1.0f - std::pow(1.0f - progress, 3.0f);

    // 插值计算当前的缩放比例
    float newZoomScale = _zoomBounceStartScale + (_zoomBounceTargetScale - _zoomBounceStartScale) * easedProgress;

    // 更新缩放比例
    _zoomScale = newZoomScale;
    updateOffsetBounds();

    // 基于保存的内容点和中心点重新计算偏移量，保持触摸点位置
    float newOffsetX = _pinchEndCenter.x - _pinchEndContentPoint.x * _zoomScale;
    float newOffsetY = _pinchEndCenter.y - _pinchEndContentPoint.y * _zoomScale;

    _contentOffset.x = newOffsetX;
    _contentOffset.y = newOffsetY;
    revalidateContentOffset();

    return false;
}

};  // namespace kk::gesture
