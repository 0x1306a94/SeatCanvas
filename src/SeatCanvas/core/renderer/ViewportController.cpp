//
//  ViewportController.cpp
//  SeatCanvas
//
//  Created by king on 2026/05/23.
//

#include "ViewportController.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include <tgfx/core/Rect.h>
#include <tgfx/core/Size.h>
#include <tgfx/platform/Print.h>

#include "core/EdgeInsets.h"
#include "core/Platform.hpp"
#include "core/ZoomScaleConfig.hpp"
#include "core/animation/Animator.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/renderer/ZoneMeshInfo.hpp"

namespace kk::renderer {

static tgfx::Point ComputeClampedOffset(const tgfx::Point &screenPoint, const tgfx::Point &contentPoint, float zoom, const tgfx::Size &viewportSize, const tgfx::Size &contentSize, const EdgeInsets &contentInset) {
    const float effectiveWidth = viewportSize.width - contentInset.left - contentInset.right;
    const float effectiveHeight = viewportSize.height - contentInset.top - contentInset.bottom;

    float ox = screenPoint.x - contentPoint.x * zoom;
    float oy = screenPoint.y - contentPoint.y * zoom;

    const float scaledW = contentSize.width * zoom;
    const float scaledH = contentSize.height * zoom;

    float minX, maxX, minY, maxY;

    if (scaledW <= effectiveWidth) {
        minX = maxX = contentInset.left + (effectiveWidth - scaledW) * 0.5f;
    } else {
        minX = viewportSize.width - scaledW;
        maxX = contentInset.left;
    }

    if (scaledH <= effectiveHeight) {
        minY = maxY = contentInset.top + (effectiveHeight - scaledH) * 0.5f;
    } else {
        minY = viewportSize.height - scaledH;
        maxY = contentInset.top;
    }

    return {
        std::clamp(ox, minX, maxX),
        std::clamp(oy, minY, maxY)};
}

static kk::animation::AnimationOptions makeEaseInOutOptions(double durationMs) {
    kk::animation::AnimationOptions options{};
    options.durationMs = durationMs;
    options.delayMs = 0.0;
    options.curve = kk::animation::AnimationCurve::EaseInOut;
    return options;
}

ViewportController::ViewportController(kk::gesture::ElasticZoomPanController *zoomPanController,
                                       SeatCanvasCoreRendererState *state,
                                       kk::animation::Animator *animator,
                                       std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate,
                                       kk::ZoomLevelConfig *zoomLevelConfig,
                                       ViewportControllerCallback *callback)
    : _zoomPanController(zoomPanController)
    , _state(state)
    , _animator(animator)
    , _delegate(std::move(delegate))
    , _zoomLevelConfig(zoomLevelConfig)
    , _callback(callback) {
}

void ViewportController::setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate) {
    _delegate = std::move(delegate);
}

void ViewportController::setCoreID(uint32_t coreID) {
    _coreID = coreID;
}

// ---- 访问器 ----

float ViewportController::getMaxWidth() const {
    return _maxWidth;
}

void ViewportController::setMaxWidth(float maxWidth) {
    _maxWidth = maxWidth;
}

bool ViewportController::isAutoDrawSeatDisabled() const {
    return _disableAutoDrawSeat;
}

kk::ZoomLevelConfig &ViewportController::zoomLevelConfig() {
    return *_zoomLevelConfig;
}

bool ViewportController::isPanAnimationActive() const {
    return _panAnimationActive;
}

bool ViewportController::isScrollingAnimationActive() const {
    return _scrollingAnimationActive;
}

void ViewportController::setPanAnimationActive(bool active) {
    _panAnimationActive = active;
}

void ViewportController::resetAnimationState() {
    _panAnimationActive = false;
    _scrollingAnimationActive = false;
}

// ---- 内部辅助 ----

bool ViewportController::isSmallVenue() const {
    return kk::isSmallVenue(*_zoomLevelConfig);
}

float ViewportController::showBackZoomThreshold() const {
    return kk::showBackZoomThreshold(*_zoomLevelConfig);
}

SeatCanvasViewportEvent ViewportController::makeViewportEvent() const {
    return MakeViewportEvent(_state);
}

void ViewportController::stopAllViewportAnimations() {
    _zoomPanController->stopAllAnimations();
    _callback->onHideMinimapWithoutAnimation();
    _animator->cancelAll();
}

void ViewportController::prepareScrollAnimationState(float currentZoomScale, float seatRenderZoomThreshold) {
    autoChangeBaseMapColorState = false;
    _disableAutoDrawSeat = (currentZoomScale <= seatRenderZoomThreshold);
}

bool ViewportController::setupShowBackAndOverlay(float targetZoomScale) {
    auto showBack = targetZoomScale >= showBackZoomThreshold();
    if (showBack) {
        _callback->onApplyBaseMapColorState(kk::BaseMapColorState::Original);
        if (_callback->onIsOverlayBackVisible()) {
            showBack = false;
        } else {
            _callback->onSetOverlayBackVisible(true);
        }
    } else {
        _callback->onApplyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        _callback->onSetOverlayBackVisible(false);
    }
    return showBack;
}

void ViewportController::beginViewportScrollingAnimation() {
    _scrollingAnimationActive = true;
}

void ViewportController::notifyViewportDidEndScrollingAnimation() {
    if (!_scrollingAnimationActive) {
        return;
    }

    _scrollingAnimationActive = false;
    if (_delegate) {
        _delegate->viewportDidEndScrollingAnimation(_coreID, makeViewportEvent());
    }
}

void ViewportController::notifyViewportDidEndDeceleratingIfNeeded() {
    if (!_panAnimationActive || _zoomPanController->hasPendingAnimation()) {
        return;
    }

    _panAnimationActive = false;
    if (_delegate) {
        _delegate->viewportDidEndDecelerating(_coreID, makeViewportEvent());
    }
}

// ---- 底图生命周期配置 ----

void ViewportController::updateContentScale(float maxWidth) {
    auto originSize = _state->getOriginSize();
    if (!originSize.isEmpty() && originSize.width > maxWidth) {
        auto scale = maxWidth / originSize.width;
        _state->updateContentScale(scale);
    } else {
        _state->updateContentScale(1.0f);
    }
}

void ViewportController::updateContentSize() {
    auto originSize = _state->getOriginSize();
    auto density = _state->getDensity();
    auto contentScale = _state->getContentScale();
    if (originSize.isEmpty()) {
        _zoomPanController->setContentSize({});
        _state->updateNormalizedContentSize({});
    } else {
        tgfx::Size normalizedContentSize{
            static_cast<float>(originSize.width * contentScale * density),
            static_cast<float>(originSize.height * contentScale * density),
        };
        _zoomPanController->setContentSize(normalizedContentSize);
        _state->updateNormalizedContentSize(normalizedContentSize);
    }
    updateMaxMinZoomScalesForCurrentBounds();
}

void ViewportController::updateMaxMinZoomScalesForCurrentBounds() {
    auto boundsSize = _state->getBoundsSize();
    auto normalizedContentSize = _state->getNormalizedContentSize();
    auto contentScale = _state->getContentScale();
    auto density = _state->getDensity();
    if (boundsSize.isEmpty() || normalizedContentSize.isEmpty()) {

        _zoomPanController->setMinimumZoomScale(1.0f);
        _zoomPanController->setMaximumZoomScale(1.0f);
        _zoomPanController->setZoomScale(1.0f);

        _zoomLevelConfig->seat = 1.0f;
        _zoomLevelConfig->row = 1.0f;
        _zoomLevelConfig->zone = 1.0f;
        _zoomLevelConfig->venue = 1.0f;

        _callback->onUpdateZoomPanControllerState(true);
        return;
    }

    const auto &contentInset = _zoomPanController->getContentInset();

    auto viewWidth = boundsSize.width - contentInset.left - contentInset.right;
    auto minimumZoomScale = viewWidth / normalizedContentSize.width;
    if (boundsSize.width > boundsSize.height) {
        viewWidth = boundsSize.height - contentInset.top - contentInset.bottom;
        minimumZoomScale = viewWidth / normalizedContentSize.height;
    }

    auto unitWidth = (contentScale * kk::ZoomScaleConfig::SEAT_BASE_SIZE) / _svgModelScale;

    _zoomLevelConfig->seat = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_SMALL)) / density;
    _zoomLevelConfig->row = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_MEDIUM)) / density;
    _zoomLevelConfig->zone = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_LARGE)) / density;
    _zoomLevelConfig->venue = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_XLARGE)) / density;

    _zoomLevelConfig->venue = std::max(_zoomLevelConfig->venue, minimumZoomScale);

    float baseScale = 1.0f / (contentScale / _svgModelScale);
    float maximumZoomScale = std::max(_zoomLevelConfig->seat, baseScale);

    _zoomPanController->setMinimumZoomScale(static_cast<float>(minimumZoomScale));
    _zoomPanController->setMaximumZoomScale(static_cast<float>(maximumZoomScale));
    _zoomPanController->setZoomScale(static_cast<float>(minimumZoomScale));

    tgfx::PrintLog("updateMaxMinZoomScalesForCurrentBounds: min %f max %f seat %f row %f zone %f venue %f",
                   minimumZoomScale,
                   maximumZoomScale,
                   _zoomLevelConfig->seat,
                   _zoomLevelConfig->row,
                   _zoomLevelConfig->zone,
                   _zoomLevelConfig->venue);
    _callback->onUpdateZoomPanControllerState(true);
}

// ---- 缩放动画 ----

void ViewportController::zoomToRect(const tgfx::Rect &rect, bool animated, float padding, double durationMs) {
    if (_zoomPanController == nullptr || _state == nullptr) {
        return;
    }

    auto bounds = _zoomPanController->getBounds();
    auto normalizedContentSize = _zoomPanController->getContentSize();
    auto contentInset = _zoomPanController->getContentInset();
    auto density = _state->getDensity();
    auto contentScale = _state->getContentScale();

    if (bounds.isEmpty() || normalizedContentSize.isEmpty() || rect.isEmpty()) {
        return;
    }

    tgfx::Rect rectInContentCoords = rect;
    rectInContentCoords.scale(contentScale * density, contentScale * density);

    float paddingInContentCoords = padding * contentScale * density;
    rectInContentCoords.inset(-paddingInContentCoords, -paddingInContentCoords);

    float effectiveWidth = bounds.width - contentInset.left - contentInset.right;
    float effectiveHeight = bounds.height - contentInset.top - contentInset.bottom;

    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return;
    }

    float currentZoomScale = _zoomPanController->getZoomScale();

    float scaleX = effectiveWidth / rectInContentCoords.width();
    float scaleY = effectiveHeight / rectInContentCoords.height();
    float targetZoomScale = std::min(scaleX, scaleY);

    float minZoom = _zoomPanController->getMinimumZoomScale();
    float maxZoom = _zoomPanController->getMaximumZoomScale();

    if (targetZoomScale < currentZoomScale) {
        tgfx::Rect visibleRect = {};
        if (_state) {
            visibleRect = _state->getVisibleOriginalRect();
        }

        if (!visibleRect.isEmpty() && visibleRect.contains(rect)) {
            targetZoomScale = currentZoomScale;
        } else {
            targetZoomScale = std::max(targetZoomScale, minZoom);
        }
    } else {
        targetZoomScale = std::min(targetZoomScale, maxZoom);
    }

    targetZoomScale = std::clamp(targetZoomScale, minZoom, maxZoom);

    float targetCenterX = rectInContentCoords.centerX();
    float targetCenterY = rectInContentCoords.centerY();

    float viewportCenterX = contentInset.left + effectiveWidth * 0.5f;
    float viewportCenterY = contentInset.top + effectiveHeight * 0.5f;

    float targetOffsetX = viewportCenterX - targetCenterX * targetZoomScale;
    float targetOffsetY = viewportCenterY - targetCenterY * targetZoomScale;

    tgfx::Point currentOffset = _zoomPanController->getContentOffset();

    _zoomPanController->setZoomScale(targetZoomScale);

    tgfx::Point targetOffset{targetOffsetX, targetOffsetY};
    _zoomPanController->setContentOffset(targetOffset);

    _callback->onUpdateZoomPanControllerState(false);

    tgfx::Point finalOffset = _zoomPanController->getContentOffset();

    if (!animated || durationMs <= 0.0) {
        return;
    }

    _zoomPanController->stopAllAnimations();

    _zoomPanController->setZoomScale(currentZoomScale);
    _zoomPanController->setContentOffset(currentOffset);
    _callback->onUpdateZoomPanControllerState(false);

    auto options = makeEaseInOutOptions(durationMs);
    const auto platform = Platform::Current();
    const auto currentMediaTime = platform->currentMediaTime();

    auto update = [this, currentZoomScale, targetZoomScale, currentOffset, finalOffset](float progress) {
        if (_zoomPanController == nullptr) {
            return;
        }

        float newZoomScale = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;
        float newOffsetX = currentOffset.x + (finalOffset.x - currentOffset.x) * progress;
        float newOffsetY = currentOffset.y + (finalOffset.y - currentOffset.y) * progress;

        _zoomPanController->setZoomScale(newZoomScale);
        _zoomPanController->setContentOffset(tgfx::Point{newOffsetX, newOffsetY});

        _callback->onUpdateZoomPanControllerState(true);
    };

    auto completion = [this, targetZoomScale, finalOffset](bool finish) {
        if (finish && _zoomPanController != nullptr) {
            _zoomPanController->setZoomScale(targetZoomScale);
            _zoomPanController->setContentOffset(finalOffset);
            _callback->onUpdateZoomPanControllerState(true);
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
    _animator->play(options, currentMediaTime, std::move(update), std::move(completion));
}

void ViewportController::scrollViewWithLocation(const tgfx::Point &location, float seatRenderZoomThreshold) {
    if (!_state) {
        return;
    }

    auto zoomScale = _state->getZoomScale();
    if (zoomScale >= _zoomLevelConfig->seat) {
        return;
    }

    if (zoomScale < seatRenderZoomThreshold) {
        float target = zoomScale;
        if (isSmallVenue()) {
            target = _zoomLevelConfig->row;
        } else {
            target = _zoomLevelConfig->zone;
        }
        zoomToPoint(location, target, true, 20.0f, 300.0, seatRenderZoomThreshold);
        return;
    }

    if (zoomScale < _zoomLevelConfig->zone) {
        zoomToPoint(location, _zoomLevelConfig->row, true, 20.0f, 300.0, seatRenderZoomThreshold);
        return;
    }

    auto diff = std::fabs(_zoomLevelConfig->seat - zoomScale);
    if (diff > FLT_EPSILON) {
        zoomToPoint(location, _zoomLevelConfig->seat, true, 20.0f, 300.0, seatRenderZoomThreshold);
    }
}

void ViewportController::scrollViewWithZone(const std::shared_ptr<ZoneMeshInfo> &zoneInfo, float seatRenderZoomThreshold) {
    if (!zoneInfo) {
        return;
    }

    if (_zoomPanController == nullptr || _state == nullptr) {
        return;
    }

    auto bounds = zoneInfo->fillBounds;
    if (bounds.isEmpty()) {
        return;
    }

    auto viewport = _zoomPanController->getBounds();
    auto normalizedContentSize = _zoomPanController->getContentSize();
    auto contentInset = _zoomPanController->getContentInset();
    float currentZoomScale = _zoomPanController->getZoomScale();
    const auto &currentOffset = _zoomPanController->getContentOffset();
    auto density = _state->getDensity();
    auto contentScale = _state->getContentScale();

    if (viewport.isEmpty() || normalizedContentSize.isEmpty() || bounds.isEmpty()) {
        return;
    }

    auto contentBounds = bounds;
    contentBounds.scale(contentScale * density, contentScale * density);

    float contentCenterX = contentBounds.centerX();
    float contentCenterY = contentBounds.centerY();

    float effectiveWidth = viewport.width - contentInset.left - contentInset.right;
    float effectiveHeight = viewport.height - contentInset.top - contentInset.bottom;
    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return;
    }

    float contentZoneWidth = std::fmax(1.0f, contentBounds.width());
    float contentZoneHeight = std::fmax(1.0f, contentBounds.height());

    float scaleX = effectiveWidth / contentZoneWidth;
    float scaleY = effectiveHeight / contentZoneHeight;
    float targetZoomScale = std::min(scaleX, scaleY);
    targetZoomScale = std::max(seatRenderZoomThreshold, targetZoomScale);

    const float minZoom = _zoomPanController->getMinimumZoomScale();
    const float maxZoom = _zoomPanController->getMaximumZoomScale();
    targetZoomScale = std::clamp(targetZoomScale, minZoom, maxZoom);

    float scaledZoneWidth = contentZoneWidth * targetZoomScale;
    float scaledZoneHeight = contentZoneHeight * targetZoomScale;

    float centerX = contentInset.left + effectiveWidth / 2;
    float centerY = contentInset.top + effectiveHeight / 2;

    float targetScreenX = centerX;
    float targetScreenY = centerY;

    float minScreenX = contentInset.left + scaledZoneWidth / 2;
    float maxScreenX = viewport.width - contentInset.right - scaledZoneWidth / 2;
    float minScreenY = contentInset.top + scaledZoneHeight / 2;
    float maxScreenY = viewport.height - contentInset.bottom - scaledZoneHeight / 2;

    if (minScreenX < maxScreenX) {
        targetScreenX = std::clamp(targetScreenX, minScreenX, maxScreenX);
    }
    if (minScreenY < maxScreenY) {
        targetScreenY = std::clamp(targetScreenY, minScreenY, maxScreenY);
    }

    float scaledContentWidth = normalizedContentSize.width * targetZoomScale;
    float scaledContentHeight = normalizedContentSize.height * targetZoomScale;

    float targetOffsetX = targetScreenX - contentCenterX * targetZoomScale;
    float targetOffsetY = targetScreenY - contentCenterY * targetZoomScale;

    float minOffsetX, maxOffsetX, minOffsetY, maxOffsetY;

    if (scaledContentWidth <= viewport.width) {
        minOffsetX = maxOffsetX = (viewport.width - scaledContentWidth) * 0.5f;
    } else {
        minOffsetX = viewport.width - scaledContentWidth;
        maxOffsetX = 0.0f;
    }

    if (scaledContentHeight <= viewport.height) {
        minOffsetY = maxOffsetY = (viewport.height - scaledContentHeight) * 0.5f;
    } else {
        minOffsetY = viewport.height - scaledContentHeight;
        maxOffsetY = 0.0f;
    }

    targetOffsetX = std::clamp(targetOffsetX, minOffsetX, maxOffsetX);
    targetOffsetY = std::clamp(targetOffsetY, minOffsetY, maxOffsetY);

    tgfx::Point finalOffset{targetOffsetX, targetOffsetY};

    stopAllViewportAnimations();
    prepareScrollAnimationState(currentZoomScale, seatRenderZoomThreshold);
    auto showBack = setupShowBackAndOverlay(targetZoomScale);
    _callback->onInvalidateContent();

    auto options = makeEaseInOutOptions(300.0);
    const auto platform = Platform::Current();
    const auto startTime = platform->currentMediaTime();

    auto update = [this, currentZoomScale, targetZoomScale, currentOffset, finalOffset, showBack](float progress) {
        if (!_zoomPanController) {
            return;
        }

        float zoom = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;
        float offsetX = currentOffset.x + (finalOffset.x - currentOffset.x) * progress;
        float offsetY = currentOffset.y + (finalOffset.y - currentOffset.y) * progress;

        _zoomPanController->setZoomScale(zoom, false);
        _zoomPanController->setContentOffset(tgfx::Point{offsetX, offsetY}, false);

        if (showBack) {
            _callback->onSetOverlayBackAlpha(progress);
        }

        _callback->onUpdateZoomPanControllerState(true);
    };

    auto completion = [this, targetZoomScale, finalOffset](bool finished) {
        autoChangeBaseMapColorState = true;
        _disableAutoDrawSeat = false;
        if (finished && _zoomPanController) {
            _zoomPanController->setZoomScale(targetZoomScale);
            _zoomPanController->setContentOffset(finalOffset);
            _callback->onUpdateZoomPanControllerState(true);
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void ViewportController::zoomToPoint(const tgfx::Point &location, float scale, bool animated, float padding, double durationMs, float seatRenderZoomThreshold) {
    if (!_zoomPanController || !_state) {
        return;
    }

    const auto viewport = _zoomPanController->getBounds();
    const auto normalizedContentSize = _zoomPanController->getContentSize();

    if (viewport.isEmpty() || normalizedContentSize.isEmpty()) {
        return;
    }

    const float minZoom = _zoomPanController->getMinimumZoomScale();
    const float maxZoom = _zoomPanController->getMaximumZoomScale();
    const float targetZoomScale = std::clamp(scale, minZoom, maxZoom);

    const float currentZoomScale = _zoomPanController->getZoomScale();
    const tgfx::Point currentOffset = _zoomPanController->getContentOffset();
    const auto contentInset = _zoomPanController->getContentInset();

    const tgfx::Point contentPoint{
        (location.x - currentOffset.x) / currentZoomScale,
        (location.y - currentOffset.y) / currentZoomScale};

    const tgfx::Point startOffset = ComputeClampedOffset(location, contentPoint, currentZoomScale, viewport, normalizedContentSize, contentInset);
    const tgfx::Point finalOffset = ComputeClampedOffset(location, contentPoint, targetZoomScale, viewport, normalizedContentSize, contentInset);

    stopAllViewportAnimations();
    prepareScrollAnimationState(currentZoomScale, seatRenderZoomThreshold);
    auto showBack = setupShowBackAndOverlay(targetZoomScale);
    _callback->onInvalidateContent();

    if (!animated || durationMs <= 0.0) {
        autoChangeBaseMapColorState = true;
        _disableAutoDrawSeat = false;
        _zoomPanController->setZoomScale(targetZoomScale);
        _zoomPanController->setContentOffset(finalOffset);
        _callback->onUpdateZoomPanControllerState(true);
        return;
    }

    auto options = makeEaseInOutOptions(durationMs);
    const auto platform = Platform::Current();
    const auto startTime = platform->currentMediaTime();

    auto update = [this, currentZoomScale, targetZoomScale, startOffset, finalOffset, showBack](float progress) {
        if (!_zoomPanController) {
            return;
        }

        const float zoom = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;

        const tgfx::Point interpolatedOffset{
            startOffset.x + (finalOffset.x - startOffset.x) * progress,
            startOffset.y + (finalOffset.y - startOffset.y) * progress};

        _zoomPanController->setZoomScale(zoom, false);
        _zoomPanController->setContentOffset(interpolatedOffset, false);

        if (showBack) {
            _callback->onSetOverlayBackAlpha(progress);
        }

        _callback->onUpdateZoomPanControllerState(true);
    };

    auto completion = [this](bool finished) {
        autoChangeBaseMapColorState = true;
        _disableAutoDrawSeat = false;
        if (finished && _zoomPanController) {
            _callback->onUpdateZoomPanControllerState(true);
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void ViewportController::handleZoomBack() {
    if (!_zoomPanController) {
        return;
    }

    auto viewport = _zoomPanController->getBounds();
    auto normalizedContentSize = _zoomPanController->getContentSize();
    auto contentInset = _zoomPanController->getContentInset();

    if (viewport.isEmpty() || normalizedContentSize.isEmpty()) {
        return;
    }

    float currentZoom = _zoomPanController->getZoomScale();
    tgfx::Point currentOffset = _zoomPanController->getContentOffset();

    float minZoom = _zoomPanController->getMinimumZoomScale();
    float targetZoom = minZoom;

    float effectiveWidth = viewport.width - contentInset.left - contentInset.right;
    float effectiveHeight = viewport.height - contentInset.top - contentInset.bottom;

    float scaledW = normalizedContentSize.width * targetZoom;
    float scaledH = normalizedContentSize.height * targetZoom;

    float minX, maxX, minY, maxY;

    if (scaledW <= effectiveWidth) {
        minX = maxX = contentInset.left + (effectiveWidth - scaledW) * 0.5f;
    } else {
        minX = viewport.width - scaledW - contentInset.right;
        maxX = contentInset.left;
    }

    if (scaledH <= effectiveHeight) {
        minY = maxY = contentInset.top + (effectiveHeight - scaledH) * 0.5f;
    } else {
        minY = viewport.height - scaledH - contentInset.bottom;
        maxY = contentInset.top;
    }

    tgfx::Point targetOffset{std::clamp(0.0f, minX, maxX), std::clamp(0.0f, minY, maxY)};

    stopAllViewportAnimations();

    autoChangeBaseMapColorState = false;
    _disableAutoDrawSeat = true;
    _callback->onApplyBaseMapColorState(kk::BaseMapColorState::Rainbow);
    _callback->onSetOverlayBackVisible(false);

    _callback->onInvalidateContent();

    auto options = makeEaseInOutOptions(300.0);
    const auto platform = Platform::Current();
    const auto startTime = platform->currentMediaTime();

    auto update = [this, currentZoom, targetZoom, currentOffset, targetOffset](float progress) {
        if (!_zoomPanController) {
            return;
        }

        float zoom = currentZoom + (targetZoom - currentZoom) * progress;

        float offsetX = currentOffset.x + (targetOffset.x - currentOffset.x) * progress;
        float offsetY = currentOffset.y + (targetOffset.y - currentOffset.y) * progress;

        _zoomPanController->setZoomScale(zoom, false);
        _zoomPanController->setContentOffset(tgfx::Point{offsetX, offsetY}, false);
        _callback->onUpdateZoomPanControllerState(true);
    };

    auto completion = [this, targetZoom, targetOffset](bool finish) {
        autoChangeBaseMapColorState = true;
        _disableAutoDrawSeat = false;
        if (finish && _zoomPanController) {
            _zoomPanController->setZoomScale(targetZoom);
            _zoomPanController->setContentOffset(targetOffset);
            _callback->onUpdateZoomPanControllerState(true);
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
    _animator->play(options, startTime, std::move(update), std::move(completion));
}

};  // namespace kk::renderer
