//
//  SeatCanvasCoreRenderer.cpp
//  SeatCraftCore
//
//  Created by king on 2025/11/11.
//

#include "SeatCanvasCoreRenderer.hpp"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <random>

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/Device.h>
#include <tgfx/gpu/Window.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>
#include <tgfx/svg/TextShaper.h>

#include "RenderFrameMetrics.hpp"
#include "RendererBackend.hpp"
#include "SeatCanvasCoreRendererState.hpp"
#include "core/BaseMapConfig.hpp"
#include "core/BaseMapLayerManager.hpp"
#include "core/DeviceLockGuard.hpp"
#include "core/FontManager.hpp"
#include "core/Platform.hpp"
#include "core/RegionInfo.hpp"
#include "core/SeatInfo.hpp"
#include "core/UniqueID.h"
#include "core/animation/Animator.hpp"
#include "core/drawers/SeatLayerTree.hpp"
#include "core/drawers/SeatOverlayLayerTree.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/layers/SeatAtlasLayer.hpp"
#include "core/layers/SeatItemLayer.hpp"
#include "core/layers/SeatRegionLayer.hpp"
#include "core/layers/SeatTextLayer.hpp"
#include "core/renderer/SeatItemCircleImageProvider.hpp"
#include "core/utils/DisplayLink.hpp"
#include "core/utils/TimeProfiler.hpp"

namespace {
constexpr double kMinimapFadeInDurationMs = 120.0;
constexpr double kMinimapFadeOutDurationMs = 240.0;
}  // namespace

namespace kk::renderer {
SeatCanvasCoreRenderer::SeatCanvasCoreRenderer(std::unique_ptr<RendererBackend> backend, std::unique_ptr<kk::gesture::ElasticZoomPanController> zoomPanController)
    : _coreID(kk::UniqueID::Next())
    , _delegate(nullptr)
    , _backend(std::move(backend))
    , _zoomPanController(std::move(zoomPanController))
    , _state(std::make_unique<SeatCanvasCoreRendererState>())
    , _seatLayer(std::make_unique<kk::drawers::SeatLayerTree>())
    , _overlayLayer(std::make_unique<kk::drawers::SeatOverlayLayerTree>())
    , _animator(std::make_unique<kk::animation::Animator>())
    , _frameMetrics(std::make_unique<RenderFrameMetrics>()) {

    _textShaper = tgfx::TextShaper::Make(FontManager::GetFallbackTypefaces());
    _seatImageProvider = std::make_unique<kk::renderer::SeatItemCircleImageProvider>();

    _seatLayer->setVisible(false);
    _overlayLayer->setVisible(false);
    _overlayLayer->setMinimapAlpha(0.0f);

    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    updateSize();
}

SeatCanvasCoreRenderer::~SeatCanvasCoreRenderer() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

uint32_t SeatCanvasCoreRenderer::coreID() const {
    return _coreID;
}

const SeatCanvasCoreRendererState *SeatCanvasCoreRenderer::state() {
    return _state.get();
}

void SeatCanvasCoreRenderer::setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate) {
    _delegate = std::move(delegate);
}

const kk::ZoomLevelConfig &SeatCanvasCoreRenderer::zoomLevelConfig() const {
    return _zoomLevelConfig;
}

void SeatCanvasCoreRenderer::replaceBackend(std::unique_ptr<RendererBackend> backend) {
    _backend = std::move(backend);
}

bool SeatCanvasCoreRenderer::updateSize() {
    if (_backend == nullptr) {
        return false;
    }

    auto size = _backend->getSize();
    auto density = _backend->getDensity();
    auto sizeChanged = _state->updateScreen(size.width, size.height, density);
    if (sizeChanged) {
        _backend->invalidSize();
        _zoomPanController->setBounds(tgfx::Size::Make(size));
        updateContentSize();
        invalidateContent();
    }
    return sizeChanged;
}

float SeatCanvasCoreRenderer::getSvgScale() const {
    return _svgScale;
}

float SeatCanvasCoreRenderer::getFPS() const {
    return _frameMetrics->currentFPS();
}

float SeatCanvasCoreRenderer::getMinimumZoomScale() const {
    return _zoomPanController->getMinimumZoomScale();
}

float SeatCanvasCoreRenderer::getMaximumZoomScale() const {
    return _zoomPanController->getMaximumZoomScale();
}

float SeatCanvasCoreRenderer::getZoomScale() const {
    return _zoomPanController->getZoomScale();
}

void SeatCanvasCoreRenderer::setZoomScale(float zoomScale) {
    _zoomPanController->setZoomScale(zoomScale);
    updateZoomPanControllerState();
}

const tgfx::Point &SeatCanvasCoreRenderer::getContentOffset() const {
    return _zoomPanController->getContentOffset();
}

void SeatCanvasCoreRenderer::setContentOffset(const tgfx::Point &contentOffset) {
    _zoomPanController->setContentOffset(contentOffset);
    updateZoomPanControllerState();
}

bool SeatCanvasCoreRenderer::isSmallVenue() const {
    return _zoomLevelConfig.zoomScale50 < 1.0f;
}

float SeatCanvasCoreRenderer::showBackZoomThreshold() const {
    if (isSmallVenue()) {
        return _zoomLevelConfig.zoomScale30;
    }
    return _zoomLevelConfig.zoomScale50;
}

void SeatCanvasCoreRenderer::setBackgroundColor(const tgfx::Color &color) {
    if (_backgroundColor == color) {
        return;
    }
    _backgroundColor = color;
    invalidateContent();
}

kk::SeatRenderMode SeatCanvasCoreRenderer::seatRenderMode() const {
    return _renderMode;
}

// 手势处理方法，由平台层调用

void SeatCanvasCoreRenderer::handleTap(const tgfx::Point &location) {
    if (_overlayLayer) {
        if (_overlayLayer->hitTestInMinimap(location)) {
            return;
        }

        if (_overlayLayer->hitTestInBack(location)) {
            handleZoomBack();
            return;
        }
    }

    // 1. 首先处理座位选择
    handleSeatSelectionAtLocation(location);

    // 2. 然后处理自动缩放
    handleAutoZoomOnTap(location);
}

void SeatCanvasCoreRenderer::handlePan(kk::gesture::GestureState state, const tgfx::Point &translation, double timestampMs) {
    switch (state) {
        case kk::gesture::GestureState::BEGAN: {
            showMinimapWithoutAnimation();
            break;
        }
        case kk::gesture::GestureState::CHANGED: {
            invalidateContent();
            break;
        }
        case kk::gesture::GestureState::ENDED:
        case kk::gesture::GestureState::CANCELLED: {
            PROFILE_POINT("Gesture Pan ended");
            hideMinimapWithAnimation();
            break;
        }
        default:
            break;
    }

    _zoomPanController->handlePan(state, translation, timestampMs);

    // 在 ENDED/CANCELLED 状态时，如果即将开始动画，延迟状态更新
    if ((state == kk::gesture::GestureState::ENDED || state == kk::gesture::GestureState::CANCELLED) && _zoomPanController->hasPendingAnimation()) {
        return;
    }

    updateZoomPanControllerState();
}

void SeatCanvasCoreRenderer::handlePinch(kk::gesture::GestureState state, float scale, const tgfx::Point &center) {

    switch (state) {
        case kk::gesture::GestureState::BEGAN: {
            showMinimapWithoutAnimation();
            break;
        }
        case kk::gesture::GestureState::CHANGED: {
            invalidateContent();
            break;
        }
        case kk::gesture::GestureState::ENDED:
        case kk::gesture::GestureState::CANCELLED: {
            PROFILE_POINT("Gesture Pinch ended");
            hideMinimapWithAnimation();
            break;
        }
        default:
            break;
    }

    _zoomPanController->handlePinch(state, scale, center);
    updateZoomPanControllerState();
}

void SeatCanvasCoreRenderer::setBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> baseMapConfig, kk::SeatRenderMode renderMode) {
    _renderMode = renderMode;
    if (baseMapConfig == nullptr) {
        _baseMapConfig = nullptr;
        _virtualBaseMapConfig.clear();
        updateUseBaseMapConfig(nullptr);
        return;
    }

    if (_baseMapConfig == baseMapConfig) {
        return;
    }

    _baseMapConfig = std::move(baseMapConfig);
    _virtualBaseMapConfig.clear();
    updateUseBaseMapConfig(_baseMapConfig);
}

void SeatCanvasCoreRenderer::enableTiled(bool enable) {
    _seatLayer->enableTiled(enable);
}

void SeatCanvasCoreRenderer::enableZoomBlur(bool enable) {
    _seatLayer->enableZoomBlur(enable);
}

void SeatCanvasCoreRenderer::invalidateSeatStatusImage() {
}

void SeatCanvasCoreRenderer::invalidateContent() {
    _invalidate = true;
}

void SeatCanvasCoreRenderer::start() {
    if (_displayLink == nullptr) {
        auto callback = [this] {
            PROFILE_GROUP_START(group, "Draw Frame Prepare");
            PROFILE_GROUP_SET_THRESHOLD(group, 2);
            PROFILE_GROUP_DISABLE_AUTO_LOG(group);

            {
                PROFILE_STAGE_START(group, animator, "Schedule Animator");
                scheduleAnimator();
                PROFILE_STAGE_END(group, animator);
            }

            {
                PROFILE_STAGE_START(group, gestureAnimator, "Handle Gesture Animator");
                if (_zoomPanController->handleDisplayLinkFire()) {
                    updateZoomPanControllerState();
                }
                PROFILE_STAGE_END(group, gestureAnimator);
            }

            if (_invalidate) {
                PROFILE_GROUP_ENABLE_AUTO_LOG(group);
            }
            PROFILE_GROUP_END(group);

            // FPS 统计
            const auto currentTime = static_cast<int64_t>(Platform::Current()->currentMediaTime());

            draw();

            auto drawTime = static_cast<int64_t>(Platform::Current()->currentMediaTime()) - currentTime;
            _frameMetrics->recordFrame(drawTime);
        };

        _displayLink = Platform::Current()->createDisplayLink(std::move(callback));
    }

    if (_displayLink) {
        _displayLink->start();
    }
}

void SeatCanvasCoreRenderer::stop() {
    _zoomPanController->stopAllAnimations();
    if (_animator) {
        _animator->cancelAll();
    }
    _minimapAnimationId = 0;
    if (_displayLink) {
        _displayLink->stop();
    }
}

void SeatCanvasCoreRenderer::draw(bool force) {
    if (_state == nullptr) {
        return;
    }

    PROFILE_GROUP_START(group, "Draw Frame");
    PROFILE_GROUP_SET_THRESHOLD(group, 16.7);

    PROFILE_STAGE_START(group, window, "Create Window");

    auto window = _backend ? _backend->getWindow() : nullptr;
    if (!window) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group);
        return;
    }

    PROFILE_STAGE_END(group, window);

    auto device = window->getDevice();
    DeviceLockGuard lockGuard(device.get());
    if (!lockGuard) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group);
        return;
    }

    PROFILE_STAGE_START(group, surface, "Create Surface");

    auto context = lockGuard.context();
    auto surface = window->getSurface(context);
    if (surface == nullptr) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group);
        return;
    }
    PROFILE_STAGE_END(group, surface);

    auto canvas = surface->getCanvas();
    if (canvas == nullptr) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group);
        return;
    }

    //    if (_seatImageProvider) {
    //        PROFILE_STAGE_START(group, prepareSeatImage, "Prepare Seat Image");
    //        _seatImageProvider->attachContext(context, _state->density());
    //        PROFILE_STAGE_END(group, prepareSeatImage);
    //    }

    auto statePtr = _state.get();

    PROFILE_STAGE_START(group, prepareBase, "Prepare BaseMap");
    _seatLayer->prepare(canvas, statePtr, force);
    PROFILE_STAGE_END(group, prepareBase);

    PROFILE_STAGE_START(group, prepareMini, "Prepare Overlay");
    _overlayLayer->prepare(canvas, statePtr, force);
    PROFILE_STAGE_END(group, prepareMini);

    if ((_seatLayer->hasContentChanged() || force) && _autoDrawSeat) {
        PROFILE_STAGE_START(group, prepareSeat, "Prepare Seat");
        drawSeatIfNeeded(canvas);
        PROFILE_STAGE_END(group, prepareSeat);
    }

    bool hasContentChanged = _seatLayer->hasContentChanged() || _overlayLayer->hasContentChanged();

    auto skipCurrentFrame = (!hasContentChanged && !force && !_invalidate);
    if (skipCurrentFrame) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group)
        return;
    }

    PROFILE_STAGE_START(group, draw, "Draw");
    canvas->clear(_backgroundColor);
    canvas->save();

    _seatLayer->draw(canvas, statePtr);
    _overlayLayer->draw(canvas, statePtr);

    drawFPS(canvas);

    canvas->restore();

    PROFILE_STAGE_END(group, draw);

#if 1
    PROFILE_STAGE_START(group, flush, "Flush");
    auto recording = context->flush();
    PROFILE_STAGE_END(group, flush);

    if (recording) {
        PROFILE_STAGE_START(group, Submit, "Submit");
        context->submit(std::move(recording));
        PROFILE_STAGE_END(group, Submit);
    }

#else
    PROFILE_STAGE_START(group, flush, "Flush");
    auto recording = context->flush();
    PROFILE_STAGE_END(group, flush);
    std::swap(_lastRecording, recording);
    if (recording != nullptr) {
        PROFILE_STAGE_START(group, Submit, "Submit");
        context->submit(std::move(recording));
        PROFILE_STAGE_END(group, Submit);
    }
#endif

    PROFILE_STAGE_START(group, present, "Present");
    window->present(context);
    PROFILE_STAGE_END(group, present);

    _invalidate = false;
}

void SeatCanvasCoreRenderer::drawFPS(tgfx::Canvas *canvas) {
    if (canvas == nullptr || !_textShaper) {
        return;
    }

    if (_frameMetrics->isFirstFrame()) {
        return;
    }

    auto fps = static_cast<int>(_frameMetrics->currentFPS());
    auto text = "FPS: " + std::to_string(fps);
    auto textBlob = _textShaper->shape(text, nullptr, 80.0f);
    if (!textBlob) {
        return;
    }

    tgfx::Paint paint;
    if (fps < 30) {
        paint.setColor(tgfx::Color::Red());
    } else {
        paint.setColor(tgfx::Color::Green());
    }
    canvas->drawTextBlob(std::move(textBlob), 40.0f, 80.0f, paint);
}

tgfx::Rect SeatCanvasCoreRenderer::getVisibleContentRect() const {
    if (_zoomPanController == nullptr) {
        return tgfx::Rect::MakeEmpty();
    }

    auto bounds = _zoomPanController->getBounds();
    auto contentSize = _zoomPanController->getContentSize();
    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();
    auto contentInset = _zoomPanController->getContentInset();
    auto density = _state->density();
    if (bounds.isEmpty() || contentSize.isEmpty() || zoomScale <= 0.0f) {
        return tgfx::Rect::MakeEmpty();
    }

    // 根据变换矩阵：screenX = contentX * zoomScale + contentOffset.x
    // 反变换：contentX = (screenX - contentOffset.x) / zoomScale

    // 屏幕可见区域的左上角和右下角（考虑 contentInset）
    float screenLeft = contentInset.left;
    float screenTop = contentInset.top;
    float screenRight = bounds.width - contentInset.right;
    float screenBottom = bounds.height - contentInset.bottom;

    // 转换为内容坐标系中的位置（像素单位）
    // contentSize = svgSize * svgScale * density
    // 所以 contentCoord = svgCoord * svgScale * density
    float contentLeft = (screenLeft - contentOffset.x) / zoomScale;
    float contentTop = (screenTop - contentOffset.y) / zoomScale;
    float contentRight = (screenRight - contentOffset.x) / zoomScale;
    float contentBottom = (screenBottom - contentOffset.y) / zoomScale;

    // 转换为 SVG 坐标系（region.bounds 使用的是 SVG 原始坐标）
    // contentSize = svgSize * svgScale * density
    // 所以 svgCoord = contentCoord / (svgScale * density)
    float svgScale = _svgScale;
    float scaleFactor = svgScale * density;

    float svgContentWidth = contentSize.width / scaleFactor;
    float svgContentHeight = contentSize.height / scaleFactor;

    float minX = std::max(0.0f, std::min(contentLeft, contentRight)) / scaleFactor;
    float minY = std::max(0.0f, std::min(contentTop, contentBottom)) / scaleFactor;
    float maxX = std::min(svgContentWidth, std::max(contentLeft, contentRight) / scaleFactor);
    float maxY = std::min(svgContentHeight, std::max(contentTop, contentBottom) / scaleFactor);

    if (maxX <= minX || maxY <= minY) {
        return tgfx::Rect::MakeEmpty();
    }

    return tgfx::Rect::MakeLTRB(minX, minY, maxX, maxY);
}

tgfx::Point SeatCanvasCoreRenderer::convertScreenToContent(const tgfx::Point &location, const tgfx::Point &contentOffset, float scale) const {
    assert(scale != 0.0f);
    auto x = (location.x - contentOffset.x) / scale;
    auto y = (location.y - contentOffset.y) / scale;
    return tgfx::Point::Make(x, y);
}

tgfx::Point SeatCanvasCoreRenderer::convertContentToScreen(const tgfx::Point &location, const tgfx::Point &contentOffset, float scale) const {
    assert(scale != 0.0f);
    auto x = location.x * scale + contentOffset.x;
    auto y = location.y * scale + contentOffset.y;
    return tgfx::Point::Make(x, y);
}

const kk::RegionInfo *SeatCanvasCoreRenderer::getSeatRegionDataByPoint(float x, float y) const {
    auto config = _useBaseMapConfig.lock();
    if (!config) {
        return nullptr;
    }
    const auto layerManager = config->layerManager();
    if (!layerManager) {
        return nullptr;
    }
    auto regionId = _seatLayer->getSeatRegionIdAt(x, y);
    if (!regionId) {
        return nullptr;
    }
    auto info = layerManager->findRegionById(regionId.value());
    return info;
}

void SeatCanvasCoreRenderer::zoomToRect(const tgfx::Rect &rect, bool animated, float padding, double durationMs) {
    if (_zoomPanController == nullptr) {
        return;
    }

    auto bounds = _zoomPanController->getBounds();
    auto contentSize = _zoomPanController->getContentSize();
    auto contentInset = _zoomPanController->getContentInset();
    auto density = _state->density();
    auto svgScale = _svgScale;

    if (bounds.isEmpty() || contentSize.isEmpty() || rect.isEmpty()) {
        return;
    }

    // rect 是原始 SVG 坐标系中的包围盒，需要转换为内容坐标系（像素单位）
    // 转换公式：contentCoord = svgCoord * svgScale * density
    tgfx::Rect rectInContentCoords = rect;
    rectInContentCoords.scale(svgScale * density, svgScale * density);

    // 添加边距（边距也是在 SVG 坐标系中，需要转换）
    float paddingInContentCoords = padding * svgScale * density;
    rectInContentCoords.inset(-paddingInContentCoords, -paddingInContentCoords);

    // 计算有效视口大小（减去 contentInset）
    float effectiveWidth = bounds.width - contentInset.left - contentInset.right;
    float effectiveHeight = bounds.height - contentInset.top - contentInset.bottom;

    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return;
    }

    // 获取当前缩放级别
    float currentZoomScale = _zoomPanController->getZoomScale();

    // 计算合适的缩放级别，使目标区域能够完整显示在视口中
    float scaleX = effectiveWidth / rectInContentCoords.width();
    float scaleY = effectiveHeight / rectInContentCoords.height();
    float targetZoomScale = std::min(scaleX, scaleY);

    // 限制在最小和最大缩放级别之间
    float minZoom = _zoomPanController->getMinimumZoomScale();
    float maxZoom = _zoomPanController->getMaximumZoomScale();

    // 确保不会意外缩小：如果计算出的缩放级别小于当前缩放级别，
    // 且目标区域在当前视口中已经可见，则保持当前缩放级别不变
    if (targetZoomScale < currentZoomScale) {
        // 检查目标区域是否已经在当前视口中可见
        // getVisibleContentRect() 返回的是 SVG 坐标系，rect 也是 SVG 坐标系，可以直接比较
        tgfx::Rect visibleRect = getVisibleContentRect();

        if (!visibleRect.isEmpty() && visibleRect.contains(rect)) {
            // 区域已经可见，只移动位置，不改变缩放级别
            targetZoomScale = currentZoomScale;
        } else {
            // 区域不可见，允许缩小，但确保不小于最小缩放级别
            targetZoomScale = std::max(targetZoomScale, minZoom);
        }
    } else {
        // 需要放大，限制在最大缩放级别内
        targetZoomScale = std::min(targetZoomScale, maxZoom);
    }

    // 最终确保在有效范围内
    targetZoomScale = std::clamp(targetZoomScale, minZoom, maxZoom);

    // 计算目标区域的中心点（在内容坐标系中，像素单位）
    float targetCenterX = rectInContentCoords.centerX();
    float targetCenterY = rectInContentCoords.centerY();

    // 计算视口中心点（在视口坐标系中）
    float viewportCenterX = contentInset.left + effectiveWidth * 0.5f;
    float viewportCenterY = contentInset.top + effectiveHeight * 0.5f;

    // 计算目标偏移量，使目标区域中心对齐到视口中心
    // 变换公式：screenX = contentX * zoomScale + contentOffset.x
    // 因此：contentOffset.x = screenX - contentX * zoomScale
    float targetOffsetX = viewportCenterX - targetCenterX * targetZoomScale;
    float targetOffsetY = viewportCenterY - targetCenterY * targetZoomScale;

    // 获取当前偏移量
    tgfx::Point currentOffset = _zoomPanController->getContentOffset();

    // 更新偏移边界（因为缩放级别改变了）
    _zoomPanController->setZoomScale(targetZoomScale);
    updateZoomPanControllerState();

    // 重新计算偏移量，确保在有效范围内
    tgfx::Point targetOffset{targetOffsetX, targetOffsetY};
    _zoomPanController->setContentOffset(targetOffset);
    updateZoomPanControllerState();

    // 获取最终的有效偏移量（可能被 clamp 了）
    tgfx::Point finalOffset = _zoomPanController->getContentOffset();

    // 如果不需要动画，直接设置
    if (!animated || durationMs <= 0.0) {
        return;
    }

    // 停止所有正在进行的动画
    _zoomPanController->stopAllAnimations();

    // 恢复当前状态，准备动画
    _zoomPanController->setZoomScale(currentZoomScale);
    _zoomPanController->setContentOffset(currentOffset);
    updateZoomPanControllerState();

    // 使用 Animator 进行平滑动画
    kk::animation::AnimationOptions options{};
    options.durationMs = durationMs;
    options.delayMs = 0.0;
    options.curve = kk::animation::AnimationCurve::EaseInOut;

    const auto platform = Platform::Current();
    const auto currentMediaTime = platform->currentMediaTime();

    auto update = [this, currentZoomScale, targetZoomScale, currentOffset, finalOffset](float progress) {
        if (_zoomPanController == nullptr) {
            return;
        }

        // 插值计算当前的缩放级别和偏移量
        float newZoomScale = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;
        float newOffsetX = currentOffset.x + (finalOffset.x - currentOffset.x) * progress;
        float newOffsetY = currentOffset.y + (finalOffset.y - currentOffset.y) * progress;

        // 更新缩放级别
        _zoomPanController->setZoomScale(newZoomScale);

        // 更新偏移量
        _zoomPanController->setContentOffset(tgfx::Point{newOffsetX, newOffsetY});

        // 通知状态更新
        updateZoomPanControllerState();
    };

    auto completion = [this, targetZoomScale, finalOffset](bool finish) {
        if (finish && _zoomPanController != nullptr) {
            // 确保最终状态正确
            _zoomPanController->setZoomScale(targetZoomScale);
            _zoomPanController->setContentOffset(finalOffset);
            updateZoomPanControllerState();
        }
    };

    _animator->play(options, currentMediaTime, std::move(update), std::move(completion));
}

// MARK: - Private method
void SeatCanvasCoreRenderer::updateUseBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> config) {
    auto old = _useBaseMapConfig.lock();
    if (old && old == config) {
        return;
    }

    if (config == nullptr) {
        setBaseMapLayer(nullptr, tgfx::Size::MakeEmpty());
        setMiniMapLayer(nullptr);
        _useBaseMapConfig.reset();
    } else {
        setBaseMapLayer(config->baseMapLayer(), config->baseMapSize());
        setMiniMapLayer(config->minimapLayer());
        _useBaseMapConfig = config;
    }

    handleBaseMapChanged();
}

void SeatCanvasCoreRenderer::handleBaseMapChanged() {
    updateSvgScale();
    updateContentSize();
}

void SeatCanvasCoreRenderer::setBaseMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &baseMapSize) {
    _seatLayer->setVisible(layer != nullptr);
    _seatLayer->setBaseMapLayer(std::move(layer), baseMapSize);
    _seatLayer->clearSeatAtlas();
    _state->updateOriginSize(baseMapSize);
}

void SeatCanvasCoreRenderer::setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer) {
    // minimap 使用与 baseMap 相同的尺寸
    tgfx::Size baseMapSize = _state->getOriginSize();
    _overlayLayer->setBaseMapLayer(std::move(layer), baseMapSize);
}

/// 设置选中的区域ID（仅对 ClickToEnter 模式有效）
/// @param regionId 区域ID，为空表示取消选择，恢复到全区域视图
void SeatCanvasCoreRenderer::setSelectedRegionId(const std::string &regionId) {
    if (regionId.empty()) {
        updateUseBaseMapConfig(_baseMapConfig);
        return;
    }

    // TODO: 构建虚拟BaseMapLayer
}

/// 为区域创建虚拟的 BaseMapLayer
/// @param regionId 区域ID
std::shared_ptr<kk::layer::BaseMapRootLayer> SeatCanvasCoreRenderer::buildVirtualBaseMapLayerForRegion(const std::string &regionId) {
    return nullptr;
}

void SeatCanvasCoreRenderer::updateSvgScale() {
    auto svgSize = _state->getOriginSize();
    if (!svgSize.isEmpty() && svgSize.width > _maxWidth) {
        _svgScale = _maxWidth / svgSize.width;
    } else {
        _svgScale = 1.0f;
    }
}

void SeatCanvasCoreRenderer::updateContentSize() {
    auto svgSize = _state->getOriginSize();
    auto density = _state->density();
    if (svgSize.isEmpty()) {
        _zoomPanController->setContentSize({});
        _state->updateContentSize({});
    } else {
        tgfx::Size contentSize{
            static_cast<float>(svgSize.width * _svgScale * density),
            static_cast<float>(svgSize.height * _svgScale * density),
        };
        _zoomPanController->setContentSize(contentSize);
        _state->updateContentSize(contentSize);
    }
    updateMaxMinZoomScalesForCurrentBounds();
}

void SeatCanvasCoreRenderer::updateMaxMinZoomScalesForCurrentBounds() {
    auto boundsSize = _state->getBoundsSize();
    auto contentSize = _state->getContentSize();
    auto density = _state->density();
    if (boundsSize.isEmpty() || contentSize.isEmpty()) {

        _zoomPanController->setMinimumZoomScale(1.0f);
        _zoomPanController->setMaximumZoomScale(1.0f);
        _zoomPanController->setZoomScale(1.0f);

        _zoomLevelConfig.zoomScale9 = 1.0f;
        _zoomLevelConfig.zoomScale18 = 1.0f;
        _zoomLevelConfig.zoomScale30 = 1.0f;
        _zoomLevelConfig.zoomScale50 = 1.0f;

        updateZoomPanControllerState();
        return;
    }

    const auto &contentInset = _zoomPanController->getContentInset();

    auto viewWidth = boundsSize.width - contentInset.left - contentInset.right;
    auto minimumZoomScale = viewWidth / contentSize.width;
    // 适配横屏
    if (boundsSize.width > boundsSize.height) {
        viewWidth = boundsSize.height - contentInset.top - contentInset.bottom;
        minimumZoomScale = viewWidth / contentSize.height;
    }

    auto unitWidth = (_svgScale * kk::ZoomScaleConfig::SEAT_BASE_SIZE) / _svgModelScale;

    // viewWidth 是像素单位 所以最后需要转为 pt 单位
    _zoomLevelConfig.zoomScale9 = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_SMALL)) / density;
    _zoomLevelConfig.zoomScale18 = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_MEDIUM)) / density;
    _zoomLevelConfig.zoomScale30 = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_LARGE)) / density;
    _zoomLevelConfig.zoomScale50 = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_XLARGE)) / density;

    _zoomLevelConfig.zoomScale50 = std::max(_zoomLevelConfig.zoomScale50, minimumZoomScale);

    float baseScale = 1.0f / (_svgScale / _svgModelScale);
    float maximumZoomScale = std::max(_zoomLevelConfig.zoomScale9, baseScale);

    _zoomPanController->setMinimumZoomScale(static_cast<float>(minimumZoomScale));
    _zoomPanController->setMaximumZoomScale(static_cast<float>(maximumZoomScale));
    _zoomPanController->setZoomScale(static_cast<float>(minimumZoomScale));

    tgfx::PrintLog("updateMaxMinZoomScalesForCurrentBounds: min %f max %f zoomScale9 %f zoomScale18 %f zoomScale30 %f zoomScale50 %f", minimumZoomScale, maximumZoomScale, _zoomLevelConfig.zoomScale9, _zoomLevelConfig.zoomScale18, _zoomLevelConfig.zoomScale30, _zoomLevelConfig.zoomScale50);
    updateZoomPanControllerState();
}

void SeatCanvasCoreRenderer::updateZoomPanControllerState() {
    auto currentZoom = _zoomPanController->getZoomScale();
    auto currentOffset = _zoomPanController->getContentOffset();
    auto changed = _state->updateZoomAndOffset(currentZoom, currentOffset);
    if (changed) {
        invalidateContent();
    }
}

bool SeatCanvasCoreRenderer::scheduleAnimator() {
    if (!_animator) {
        return false;
    }

    const auto platform = Platform::Current();
    auto hasAnimation = _animator->tick(platform->currentMediaTime());
    return hasAnimation;
}

void SeatCanvasCoreRenderer::showMinimapWithoutAnimation() {
    if (_overlayLayer == nullptr) {
        return;
    }

    if (_minimapAnimationId != 0) {
        _animator->cancel(_minimapAnimationId);
        _minimapAnimationId = 0;
    }

    _overlayLayer->setMinimapVisible(true);
    _overlayLayer->setMinimapAlpha(1.0f);
    _overlayLayer->setBackVisible(false);
    _overlayLayer->setBackAlpha(0.0f);
    invalidateContent();
}

void SeatCanvasCoreRenderer::hideMinimapWithAnimation() {
    if (_overlayLayer == nullptr) {
        return;
    }

    auto zoomScale = _state->zoomScale();
    auto showBack = zoomScale >= showBackZoomThreshold();

    if (_animator && _minimapAnimationId != 0) {
        _animator->cancel(_minimapAnimationId);
        _minimapAnimationId = 0;
    }

    kk::animation::AnimationOptions options{};
    options.durationMs = 350.0;
    options.delayMs = 400.0;
    options.curve = kk::animation::AnimationCurve::EaseOut;

    const auto platform = Platform::Current();
    const auto currentMediaTime = platform->currentMediaTime();

    _overlayLayer->setBackVisible(showBack);

    auto update = [this](float progress) {
        if (_overlayLayer) {
            _overlayLayer->setMinimapAlpha(1.0f - progress);
            _overlayLayer->setBackAlpha(progress);
            invalidateContent();
        }
    };

    auto completion = [this](bool finish) {
        if (_overlayLayer && finish) {
            _overlayLayer->setMinimapAlpha(0.0f);
            _overlayLayer->setMinimapVisible(false);
            invalidateContent();
        }
        _minimapAnimationId = 0;
    };

    _minimapAnimationId = _animator->play(options, currentMediaTime, std::move(update), std::move(completion));
}

void SeatCanvasCoreRenderer::hideMinimapWithoutAnimation() {
    if (_overlayLayer == nullptr) {
        return;
    }

    if (_animator && _minimapAnimationId != 0) {
        _animator->cancel(_minimapAnimationId);
        _minimapAnimationId = 0;
    }

    _overlayLayer->setMinimapVisible(false);
    _overlayLayer->setMinimapAlpha(0.0f);
}

std::vector<kk::SeatInfo> GenMockSeatInfos(const tgfx::Rect &rect) {

    std::vector<kk::SeatInfo> seats;
    float x = rect.x();
    float y = rect.y();
    float w = rect.width();
    float h = rect.height();
    float itemSize = 36.0f;
    float spacing = 10.f;

    // 计算每行和每列能放多少个座位
    int cols = static_cast<int>((w) / (itemSize + spacing));
    int rows = static_cast<int>((h) / (itemSize + spacing));

    // 生成网格状的座位数据
    int seatIndex = 0;
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float seatX = x + col * (itemSize + spacing);
            float seatY = y + row * (itemSize + spacing);

            std::string seatId = "seat_" + std::to_string(row) + "_" + std::to_string(col);
            tgfx::Rect rect = tgfx::Rect::MakeXYWH(seatX, seatY, itemSize, itemSize);

            kk::SeatInfo seat(seatId, rect);

            // 随机设置一些座位的状态，模拟真实场景
            int randValue = (row * cols + col) % 10;
            if (randValue < 6) {
                seat.status = kk::SeatStatus::Available;
            } else if (randValue < 8) {
                seat.status = kk::SeatStatus::Sold;
            } else if (randValue < 9) {
                seat.status = kk::SeatStatus::Locked;
            } else {
                seat.status = kk::SeatStatus::Disabled;
            }

            seats.push_back(seat);
            seatIndex++;
        }
    }

    return seats;
}

const std::unordered_map<std::string, std::vector<kk::SeatInfo>> &MockSeatInfos() {

    static std::unordered_map<std::string, std::vector<kk::SeatInfo>> seatInfoMap;
    static std::once_flag flag;
    std::call_once(flag, [&] {
        std::vector<std::pair<std::string, tgfx::Rect>> regions{
            std::make_pair("20011", tgfx::Rect::MakeXYWH(850, 2800, 550, 320)),
            std::make_pair("20012", tgfx::Rect::MakeXYWH(850, 3170, 550, 320)),
            std::make_pair("20013", tgfx::Rect::MakeXYWH(850, 3540, 550, 320)),
            std::make_pair("20014", tgfx::Rect::MakeXYWH(850, 3910, 550, 320)),
            std::make_pair("20031", tgfx::Rect::MakeXYWH(2000, 2800, 450, 280)),
            std::make_pair("20032", tgfx::Rect::MakeXYWH(2000, 3130, 450, 280)),
            std::make_pair("20034", tgfx::Rect::MakeXYWH(2000, 3790, 450, 280)),
            std::make_pair("10085", tgfx::Rect::MakeXYWH(1700, 1520, 700, 230)),
            std::make_pair("10086", tgfx::Rect::MakeXYWH(2450, 1520, 700, 230)),
            std::make_pair("30031", tgfx::Rect::MakeXYWH(9550, 2800, 450, 280)),
            std::make_pair("30032", tgfx::Rect::MakeXYWH(9550, 3130, 450, 280)),
            std::make_pair("30033", tgfx::Rect::MakeXYWH(9550, 3460, 450, 280)),
            std::make_pair("10096", tgfx::Rect::MakeXYWH(9600, 1800, 700, 220)),
            std::make_pair("10098", tgfx::Rect::MakeXYWH(11100, 1800, 700, 220)),
            std::make_pair("40109", tgfx::Rect::MakeXYWH(6950, 9300, 700, 250)),
            std::make_pair("40110", tgfx::Rect::MakeXYWH(7700, 9300, 700, 250)),
            std::make_pair("40123", tgfx::Rect::MakeXYWH(6600, 9600, 750, 300)),
            std::make_pair("40124", tgfx::Rect::MakeXYWH(7400, 9600, 750, 300)),
            std::make_pair("37492", tgfx::Rect::MakeXYWH(33, 411, 289, 329)),
            std::make_pair("74148", tgfx::Rect::MakeXYWH(400, 76, 332, 232)),
        };

        for (const auto &item : regions) {
            seatInfoMap.insert_or_assign(item.first, GenMockSeatInfos(item.second));
        }
    });

    return seatInfoMap;
}

void SeatCanvasCoreRenderer::drawSeatIfNeeded(tgfx::Canvas *canvas) {
    if (!_state) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto layerManager = baseMapConfig->layerManager();
    if (layerManager == nullptr) {
        return;
    }

    auto zoomScale = _state->zoomScale();

    /*
     * 缩小到一定级别后，不显示座位
     * zoomScale 越小表示缩得越小，zoomScale50 是一个较小的缩放值
     * 所以当 zoomScale < zoomScale50 时，应该隐藏座位
     */
    if (zoomScale < _zoomLevelConfig.zoomScale50) {
        // 全部隐藏
        _seatLayer->hiddenSeatAtlas();
        if (_autoChangeBaseMapColorState) {
            applyBaseMapColorState(layerManager, BaseMapColorState::Rainbow);
        }
        return;
    }

    if (_autoChangeBaseMapColorState) {
        applyBaseMapColorState(layerManager, BaseMapColorState::Original);
    }

    auto visibleContentRect = getVisibleContentRect();
    if (visibleContentRect.isEmpty()) {
        // 全部隐藏
        _seatLayer->hiddenSeatAtlas();
        return;
    }

    /*
     * 扩大一点点，避免出现刚好在边缘的隐藏/显示，视觉上体验不好
     */
    visibleContentRect.outset(60, 60);

    auto regions = layerManager->findRegionsIntersectingRect(visibleContentRect);
    if (regions.empty()) {
        // 全部隐藏
        _seatLayer->hiddenSeatAtlas();
        return;
    }

    //    if (!_seatImageProvider) {
    //        // 全部隐藏
    //        _seatLayer->hiddenSeatAtlas();
    //    }

    // 记录最终值，避免产生多余的脏标记
    std::unordered_map<std::string, bool> seatAtlasVisibleMap{};

    for (const auto &region : regions) {
        const auto &seatInfoMap = MockSeatInfos();
        auto iter = seatInfoMap.find(region->regionId);
        if (iter == seatInfoMap.end()) {
            continue;
        }

        const auto &seatInfos = iter->second;
        if (seatInfos.empty()) {
            continue;
        }

        auto atlas = _seatLayer->getSeatAtlasLayerOrCreate(region->regionId, [&region](kk::layer::SeatAtlasLayer *layer) {
            layer->setName(region->regionId);
        });

        //        atlas->hiddenAllSeat();

        bool showAtlas = false;
        auto fullAtlas = visibleContentRect.contains(region->bounds);
        for (const auto &info : seatInfos) {
            if (!fullAtlas && !tgfx::Rect::Intersects(visibleContentRect, info.rect)) {
                continue;
            }

            auto seatItemLayer = atlas->getSeatLayerOrCreate(info.seatId, [&info](kk::layer::SeatItemLayer *layer) {
                layer->setName(info.seatId);
                layer->setPosition({info.rect.x(), info.rect.y()});
                layer->setSeatSize({info.rect.width(), info.rect.height()});
            });

            //            auto image = _seatImageProvider->lookup(kk::SeatItemImageKey{info.status, seatItemLayer->selected()});
            //            seatItemLayer->setSeatImage(std::move(image));
            seatItemLayer->setSeatSatus(info.status);
            //            seatItemLayer->setVisible(true);
            showAtlas = true;
        }

        seatAtlasVisibleMap.insert_or_assign(region->regionId, showAtlas);
    }

    _seatLayer->hiddenSeatAtlas(seatAtlasVisibleMap);
}

void SeatCanvasCoreRenderer::handleZoomBack() {
    if (!_zoomPanController) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto layerManager = baseMapConfig->layerManager();
    if (layerManager == nullptr) {
        return;
    }

    auto viewport = _zoomPanController->getBounds();
    auto contentSize = _zoomPanController->getContentSize();
    auto contentInset = _zoomPanController->getContentInset();

    if (viewport.isEmpty() || contentSize.isEmpty()) {
        return;
    }

    float currentZoom = _zoomPanController->getZoomScale();
    tgfx::Point currentOffset = _zoomPanController->getContentOffset();

    float minZoom = _zoomPanController->getMinimumZoomScale();
    float targetZoom = minZoom;

    // ------------------- 计算目标居中 offset -------------------
    float effectiveWidth = viewport.width - contentInset.left - contentInset.right;
    float effectiveHeight = viewport.height - contentInset.top - contentInset.bottom;

    float scaledW = contentSize.width * targetZoom;
    float scaledH = contentSize.height * targetZoom;

    float minX, maxX, minY, maxY;

    if (scaledW <= effectiveWidth) {
        // 内容比 viewport 小 → 居中
        minX = maxX = contentInset.left + (effectiveWidth - scaledW) * 0.5f;
    } else {
        // 内容比 viewport 大 → clamp
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

    _zoomPanController->stopAllAnimations();
    hideMinimapWithoutAnimation();
    _animator->cancelAll();

    _autoChangeBaseMapColorState = false;
    _autoDrawSeat = false;
    applyBaseMapColorState(layerManager, BaseMapColorState::Rainbow);
    _overlayLayer->setBackVisible(false);
    _seatLayer->hiddenSeatAtlas();

    kk::animation::AnimationOptions options{};
    options.durationMs = 300;
    options.curve = kk::animation::AnimationCurve::EaseInOut;

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
        updateZoomPanControllerState();
    };

    auto completion = [this, targetZoom, targetOffset](bool finish) {
        _autoChangeBaseMapColorState = true;
        _autoDrawSeat = true;
        if (finish && _zoomPanController) {
            _zoomPanController->setZoomScale(targetZoom);
            _zoomPanController->setContentOffset(targetOffset);
            updateZoomPanControllerState();
        }
    };

    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void SeatCanvasCoreRenderer::handleSeatSelectionAtLocation(const tgfx::Point &location) {
    if (!_state || !_seatLayer) {
        return;
    }

    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();

    // 转换为内容坐标系
    auto contentLocation = convertScreenToContent(location, contentOffset, zoomScale);
    if (!_seatLayer->hitTestPoint(contentLocation)) {
        return;
    }

    // 检查是否点击到座位
    auto seatItemLayer = _seatLayer->getSeatItemAt(contentLocation.x, contentLocation.y);
    if (!seatItemLayer) {
        return;
    }

    if (!_delegate) {
        return;
    }

    auto seatId = seatItemLayer->name();
    if (seatId.empty()) {
        return;
    }

    auto canSelected = _delegate->shouldSelectSeat(_coreID, seatId);
    if (!canSelected) {
        return;
    }

    //    auto image = _seatImageProvider->lookup(kk::SeatItemImageKey{seatItemLayer->seatSatus(), !seatItemLayer->selected()});
    if (seatItemLayer->selected()) {
        seatItemLayer->setSelected(false);
        //        seatItemLayer->setSeatImage(std::move(image));
        _delegate->didDeselectSeat(_coreID, seatId);
    } else {
        seatItemLayer->setSelected(true);
        //        seatItemLayer->setSeatImage(std::move(image));
        _delegate->didSelectSeat(_coreID, seatId);
    }

    showMinimapWithoutAnimation();
}

void SeatCanvasCoreRenderer::handleAutoZoomOnTap(const tgfx::Point &location) {
    if (!_state || !_seatLayer) {
        return;
    }

    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();

    // 转换为内容坐标系
    auto contentLocation = convertScreenToContent(location, contentOffset, zoomScale);
    if (!_seatLayer->hitTestPoint(contentLocation)) {
        return;
    }

    // 如果已经缩放到座位级别（zoomScale50），则执行点击位置的渐进式缩放
    if (zoomScale >= _zoomLevelConfig.zoomScale50 || isSmallVenue()) {
        scrollViewWithLocation(location);
        return;
    }

    // 检查是否点击到座位集合（Atlas）
    if (auto atlasLayer = _seatLayer->getSeatAtlasAt(contentLocation.x, contentLocation.y); atlasLayer) {
        scrollViewWithRegion(atlasLayer->name());
        return;
    }

    // 检查是否点击到区域
    if (auto regionIdResult = _seatLayer->getSeatRegionIdAt(contentLocation.x, contentLocation.y); regionIdResult.has_value()) {
        scrollViewWithRegion(regionIdResult.value());
        return;
    }

    scrollViewWithLocation(location);
}

static tgfx::Point ComputeClampedOffset(const tgfx::Point &screenPoint, const tgfx::Point &contentPoint, float zoom, const tgfx::Size &viewportSize, const tgfx::Size &contentSize) {
    float ox = screenPoint.x - contentPoint.x * zoom;
    float oy = screenPoint.y - contentPoint.y * zoom;

    const float scaledW = contentSize.width * zoom;
    const float scaledH = contentSize.height * zoom;

    float minX, maxX, minY, maxY;

    if (scaledW <= viewportSize.width) {
        minX = maxX = (viewportSize.width - scaledW) * 0.5f;
    } else {
        minX = viewportSize.width - scaledW;
        maxX = 0.0f;
    }

    if (scaledH <= viewportSize.height) {
        minY = maxY = (viewportSize.height - scaledH) * 0.5f;
    } else {
        minY = viewportSize.height - scaledH;
        maxY = 0.0f;
    }

    return {
        std::clamp(ox, minX, maxX),
        std::clamp(oy, minY, maxY)};
}

void SeatCanvasCoreRenderer::scrollViewWithLocation(const tgfx::Point &location) {
    if (!_state) {
        return;
    }

    auto zoomScale = _state->zoomScale();
    // === 情况1：已经是最大缩放 ===
    if (zoomScale >= _zoomLevelConfig.zoomScale9) {
        return;
    }

    // === 情况2：从远景放大到中景 ===
    if (zoomScale < _zoomLevelConfig.zoomScale50) {
        // 根据场馆类型选择目标缩放
        float target = zoomScale;
        if (isSmallVenue()) {
            target = _zoomLevelConfig.zoomScale18;
        } else {
            target = _zoomLevelConfig.zoomScale30;
        }
        zoomToPoint(location, target, true, 20.0f, 300.0);
        return;
    }

    // === 情况3：从中景放大到中景+ ===
    if (zoomScale < _zoomLevelConfig.zoomScale30) {
        // 放大到 18格
        zoomToPoint(location, _zoomLevelConfig.zoomScale18, true, 20.0f, 300.0);
        return;
    }

    // === 情况4：从中景+放大到近景 ===
    auto diff = std::fabs(_zoomLevelConfig.zoomScale9 - zoomScale);
    if (diff > FLT_EPSILON) {
        zoomToPoint(location, _zoomLevelConfig.zoomScale9, true, 20.0f, 300.0);
    }
}

void SeatCanvasCoreRenderer::scrollViewWithRegion(const std::string &regionId) {
    if (regionId.empty()) {
        return;
    }

    if (_zoomPanController == nullptr || _state == nullptr) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto layerManager = baseMapConfig->layerManager();
    if (layerManager == nullptr) {
        return;
    }

    auto regionInfo = layerManager->findRegionById(regionId);
    if (regionInfo == nullptr) {
        return;
    }

    auto bounds = regionInfo->bounds;
    if (bounds.isEmpty()) {
        return;
    }

    auto viewport = _zoomPanController->getBounds();
    auto contentSize = _zoomPanController->getContentSize();
    auto contentInset = _zoomPanController->getContentInset();
    float currentZoomScale = _zoomPanController->getZoomScale();
    const auto &currentOffset = _zoomPanController->getContentOffset();
    auto density = _state->density();
    auto svgScale = _svgScale;

    if (viewport.isEmpty() || contentSize.isEmpty() || bounds.isEmpty()) {
        return;
    }

    // ---------------- 1. 计算 content 坐标下区域边界 ----------------
    auto contentBounds = bounds;
    contentBounds.scale(svgScale * density, svgScale * density);

    float contentCenterX = contentBounds.centerX();
    float contentCenterY = contentBounds.centerY();

    // ---------------- 2. 计算有效视口区域 ----------------
    float effectiveWidth = viewport.width - contentInset.left - contentInset.right;
    float effectiveHeight = viewport.height - contentInset.top - contentInset.bottom;
    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return;
    }

    // ---------------- 3. 计算目标缩放比例 ----------------
    float contentRegionWidth = std::fmax(1.0f, contentBounds.width());
    float contentRegionHeight = std::fmax(1.0f, contentBounds.height());

    float scaleX = effectiveWidth / contentRegionWidth;
    float scaleY = effectiveHeight / contentRegionHeight;
    float targetZoomScale = std::min(scaleX, scaleY);
    targetZoomScale = std::max(_zoomLevelConfig.zoomScale50, targetZoomScale);

    // clamp 到有效范围
    const float minZoom = _zoomPanController->getMinimumZoomScale();
    const float maxZoom = _zoomPanController->getMaximumZoomScale();
    targetZoomScale = std::clamp(targetZoomScale, minZoom, maxZoom);

    // ---------------- 4. 计算区域在目标缩放下的尺寸 ----------------
    float scaledRegionWidth = contentRegionWidth * targetZoomScale;
    float scaledRegionHeight = contentRegionHeight * targetZoomScale;

    // ---------------- 5. 根据区域位置计算目标屏幕位置 ----------------
    // 计算区域在内容中的相对位置 (0~1)
    float relativeX = contentCenterX / contentSize.width;
    float relativeY = contentCenterY / contentSize.height;

    // 计算缩放后内容总尺寸
    float scaledContentWidth = contentSize.width * targetZoomScale;
    float scaledContentHeight = contentSize.height * targetZoomScale;

    // 计算区域中心应该出现在屏幕上的目标位置
    // 策略：根据区域在内容中的相对位置，智能决定对齐方式
    float targetScreenX, targetScreenY;

    // X 轴：
    // - 区域靠左时（relativeX 小），将区域放在视口左侧
    // - 区域靠右时（relativeX 大），将区域放在视口右侧
    // - 区域居中时，将区域放在视口中心
    float leftBound = contentInset.left + scaledRegionWidth / 2 + contentInset.left;
    float rightBound = viewport.width - contentInset.right - scaledRegionWidth / 2 - contentInset.right;
    float centerX = contentInset.left + effectiveWidth / 2;

    // 如果区域太大无法留边距，则居中显示
    if (leftBound >= rightBound) {
        targetScreenX = centerX;
    } else {
        // 使用平滑插值：根据相对位置在左、中、右之间过渡
        targetScreenX = leftBound + (rightBound - leftBound) * relativeX;
    }

    // Y 轴：同样的逻辑
    float topBound = contentInset.top + scaledRegionHeight / 2 + contentInset.top;
    float bottomBound = viewport.height - contentInset.bottom - scaledRegionHeight / 2 - contentInset.bottom;
    float centerY = contentInset.top + effectiveHeight / 2;

    if (topBound >= bottomBound) {
        targetScreenY = centerY;
    } else {
        targetScreenY = topBound + (bottomBound - topBound) * relativeY;
    }

    // ---------------- 6. 计算目标 contentOffset ----------------
    // 公式：screenPos = contentPos * zoom + offset
    // 所以：offset = screenPos - contentPos * zoom
    float targetOffsetX = targetScreenX - contentCenterX * targetZoomScale;
    float targetOffsetY = targetScreenY - contentCenterY * targetZoomScale;

    // ---------------- 7. clamp offset 到有效范围 ----------------
    float minOffsetX, maxOffsetX, minOffsetY, maxOffsetY;

    if (scaledContentWidth <= viewport.width) {
        // 内容比视口小，居中
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

    // ---------------- 8. 执行缩放和偏移（带动画）----------------
    _zoomPanController->stopAllAnimations();
    hideMinimapWithoutAnimation();
    _animator->cancelAll();

    _autoChangeBaseMapColorState = false;
    _autoDrawSeat = false;

    auto showBack = targetZoomScale >= showBackZoomThreshold();
    if (showBack) {
        applyBaseMapColorState(layerManager, BaseMapColorState::Original);
        if (_overlayLayer->backVisible()) {
            showBack = false;
        } else {
            _overlayLayer->setBackVisible(true);
        }
    } else {
        applyBaseMapColorState(layerManager, BaseMapColorState::Rainbow);
        _overlayLayer->setBackVisible(false);
    }

    // 动画参数
    kk::animation::AnimationOptions options{};
    options.durationMs = 300.0;
    options.delayMs = 0.0;
    options.curve = kk::animation::AnimationCurve::EaseInOut;

    const auto platform = Platform::Current();
    const auto startTime = platform->currentMediaTime();

    // 动画更新：同时插值 zoom 和 offset
    auto update = [this, currentZoomScale, targetZoomScale, currentOffset, finalOffset, showBack](float progress) {
        if (!_zoomPanController) {
            return;
        }

        float zoom = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;
        float offsetX = currentOffset.x + (finalOffset.x - currentOffset.x) * progress;
        float offsetY = currentOffset.y + (finalOffset.y - currentOffset.y) * progress;

        _zoomPanController->setZoomScale(zoom, false);
        _zoomPanController->setContentOffset(tgfx::Point{offsetX, offsetY}, false);

        if (showBack && _overlayLayer) {
            _overlayLayer->setBackAlpha(progress);
        }

        updateZoomPanControllerState();
    };

    // 动画完成回调
    auto completion = [this, targetZoomScale, finalOffset](bool finished) {
        _autoChangeBaseMapColorState = true;
        _autoDrawSeat = true;
        if (finished && _zoomPanController) {
            _zoomPanController->setZoomScale(targetZoomScale);
            _zoomPanController->setContentOffset(finalOffset);
            updateZoomPanControllerState();
        }
    };

    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void SeatCanvasCoreRenderer::zoomToPoint(const tgfx::Point &location, float scale, bool animated, float padding, double durationMs) {
    if (!_zoomPanController || !_state) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto layerManager = baseMapConfig->layerManager();
    if (!layerManager) {
        return;
    }

    const auto viewport = _zoomPanController->getBounds();
    const auto contentSize = _zoomPanController->getContentSize();

    if (viewport.isEmpty() || contentSize.isEmpty()) {
        return;
    }

    // ---- 1. clamp zoom ----
    const float minZoom = _zoomPanController->getMinimumZoomScale();
    const float maxZoom = _zoomPanController->getMaximumZoomScale();
    const float targetZoomScale = std::clamp(scale, minZoom, maxZoom);

    const float currentZoomScale = _zoomPanController->getZoomScale();
    const tgfx::Point currentOffset = _zoomPanController->getContentOffset();

    // ---- 2. screen -> content（当前 zoom 下）----
    const tgfx::Point contentPoint{
        (location.x - currentOffset.x) / currentZoomScale,
        (location.y - currentOffset.y) / currentZoomScale};

    // ---- 3. 计算目标 offset（一次性 clamp）----
    const tgfx::Point finalOffset = ComputeClampedOffset(location, contentPoint, targetZoomScale, viewport, contentSize);

    // ---- 4. 动画准备 ----
    _zoomPanController->stopAllAnimations();
    hideMinimapWithoutAnimation();
    _animator->cancelAll();

    _autoChangeBaseMapColorState = false;
    _autoDrawSeat = false;
    auto showBack = targetZoomScale >= showBackZoomThreshold();
    if (showBack) {
        applyBaseMapColorState(layerManager, BaseMapColorState::Original);
        if (_overlayLayer->backVisible()) {
            showBack = false;
        } else {
            _overlayLayer->setBackVisible(true);
        }
    } else {
        applyBaseMapColorState(layerManager, BaseMapColorState::Rainbow);
        _overlayLayer->setBackVisible(false);
    }

    if (!animated || durationMs <= 0.0) {
        _autoChangeBaseMapColorState = true;
        _autoDrawSeat = true;
        _zoomPanController->setZoomScale(targetZoomScale);
        _zoomPanController->setContentOffset(finalOffset);
        updateZoomPanControllerState();
        return;
    }

    kk::animation::AnimationOptions options{};
    options.durationMs = durationMs;
    options.delayMs = 0.0;
    options.curve = kk::animation::AnimationCurve::EaseInOut;

    const auto platform = Platform::Current();
    const auto startTime = platform->currentMediaTime();

    // ---- 5. 动画更新（只插值 zoom）----
    auto update = [this, currentZoomScale, targetZoomScale, contentPoint, location, showBack](float progress) {
        if (!_zoomPanController) {
            return;
        }

        const float zoom = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;

        const tgfx::Point offset{
            location.x - contentPoint.x * zoom,
            location.y - contentPoint.y * zoom};

        _zoomPanController->setZoomScale(zoom, false);
        _zoomPanController->setContentOffset(offset, false);

        if (showBack && _overlayLayer) {
            _overlayLayer->setBackAlpha(progress);
        }

        updateZoomPanControllerState();
    };

    // ---- 7. 动画结束，落到精确目标态 ----
    auto completion = [this, targetZoomScale, finalOffset](bool finished) {
        _autoChangeBaseMapColorState = true;
        _autoDrawSeat = true;
        if (finished && _zoomPanController) {
            _zoomPanController->setZoomScale(targetZoomScale);
            _zoomPanController->setContentOffset(finalOffset);
            updateZoomPanControllerState();
        }
    };

    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void SeatCanvasCoreRenderer::applyBaseMapColorState(const kk::BaseMapLayerManager *layerManager, BaseMapColorState toState) {
    if (layerManager == nullptr) {
        return;
    }

    if (_baseMapColorState == toState) {
        return;
    }

    const auto &regionLayers = layerManager->getAllRegionLayers();
    const auto &textLayers = layerManager->getAllTextLayers();
    if (toState == BaseMapColorState::Original) {
        for (auto &[key, layer] : regionLayers) {
            layer->setFillStyle(tgfx::ShapeStyle::Make(tgfx::Color::White()));
        }

        for (auto &layer : textLayers) {
            layer->setTextColor(tgfx::Color{0.0f, 0.0f, 0.0f, 0.35f});
        }

        _baseMapColorState = toState;
    } else if (toState == BaseMapColorState::Rainbow) {
        std::mt19937 generator;
        std::uniform_int_distribution<int> distribution(0, 255);

        for (auto &[key, layer] : regionLayers) {
            layer->setFillStyle(tgfx::ShapeStyle::Make(tgfx::Color::FromRGBA(distribution(generator), distribution(generator), distribution(generator))));
        }

        for (auto &layer : textLayers) {
            layer->setTextColor(tgfx::Color::Black());
        }

        _baseMapColorState = toState;
    }
}

};  // namespace kk::renderer
