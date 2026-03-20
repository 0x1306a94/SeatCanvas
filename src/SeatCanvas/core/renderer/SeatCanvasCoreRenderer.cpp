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
#include <unordered_map>
#include <unordered_set>

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/CommandEncoder.h>
#include <tgfx/gpu/CommandQueue.h>
#include <tgfx/gpu/Device.h>
#include <tgfx/gpu/GPU.h>
#include <tgfx/gpu/Texture.h>
#include <tgfx/gpu/Window.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>
#include <tgfx/svg/TextShaper.h>

#include "PlatformView.hpp"
#include "RenderFrameMetrics.hpp"
#include "SeatCanvasCoreRendererState.hpp"
#include "core/BaseMapConfig.hpp"
#include "core/DeviceLockGuard.hpp"
#include "core/FontManager.hpp"
#include "core/Platform.hpp"
#include "core/UniqueID.h"
#include "core/animation/Animator.hpp"
#include "core/drawers/SeatOverlayLayerTree.hpp"
#include "core/drawers/SeatZoneNameLayerTree.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/layers/SeatTextLayer.hpp"
#include "core/layers/SeatZoneLayer.hpp"
#include "core/renderer/BaseMapMeshBuilder.hpp"
#include "core/renderer/pass/CustomBaseMapPass.hpp"
#include "core/renderer/pass/CustomSeatPass.hpp"
#include "core/style/SeatStyleAtlasManager.hpp"
#include "core/style/SeatStyleConfig.hpp"
#include "core/style/SeatStyleConfigJSONHelper.hpp"
#include "core/style/SeatStyleKey.hpp"
#include "core/style/SeatStyleType.hpp"
#include "core/utils/DisplayLink.hpp"
#include "core/utils/TimeProfiler.hpp"
#include "core/utils/UnitConverter.hpp"

namespace {
constexpr double MINIMAP_FADE_IN_DURATION_MS = 120.0;
constexpr double MINIMAP_FADE_OUT_DURATION_MS = 240.0;
}  // namespace

namespace kk::renderer {
SeatCanvasCoreRenderer::SeatCanvasCoreRenderer(
    std::unique_ptr<PlatformView> platformView,
    std::unique_ptr<kk::gesture::ElasticZoomPanController> zoomPanController,
    const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleKeyToConfig)
    : _coreID(kk::UniqueID::Next())
    , _delegate(nullptr)
    , _platformView(std::move(platformView))
    , _zoomPanController(std::move(zoomPanController))
    , _state(std::make_unique<SeatCanvasCoreRendererState>())
    , _customBaseMapPass(std::make_unique<CustomBaseMapPass>())
    , _customSeatPass(std::make_unique<CustomSeatPass>())
    , _seatAtlasManager(nullptr)
    , _seatZoneNameLayer(std::make_unique<kk::drawers::SeatZoneNameLayerTree>())
    , _overlayLayer(std::make_unique<kk::drawers::SeatOverlayLayerTree>())
    , _animator(std::make_unique<kk::animation::Animator>())
    , _frameMetrics(std::make_unique<RenderFrameMetrics>()) {

    _textShaper = tgfx::TextShaper::Make(FontManager::GetFallbackTypefaces());

    _overlayLayer->setVisible(false);
    _overlayLayer->setMinimapAlpha(0.0f);

    _seatAtlasManager = std::make_unique<SeatStyleAtlasManager>();
    _seatAtlasManager->setOnAtlasGenerated([this](const SeatStyleAtlasManager *atlasManager) {
        _customSeatPass->updateUVOffset(atlasManager->getUVOffsets());
    });

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

void SeatCanvasCoreRenderer::setSeatRenderZoomThreshold(float zoomThreshold) {
    if (zoomThreshold <= 0.0f) {
        _seatRenderZoomThreshold = 0.0f;
        return;
    }
    _seatRenderZoomThreshold = zoomThreshold;
}

float SeatCanvasCoreRenderer::getSeatRenderZoomThreshold() const {
    if (_seatRenderZoomThreshold > 0.0f) {
        return _seatRenderZoomThreshold;
    }
    return _zoomLevelConfig.venue;
}

void SeatCanvasCoreRenderer::replacePlatformView(std::unique_ptr<PlatformView> platformView) {
    _platformView = std::move(platformView);
}

bool SeatCanvasCoreRenderer::updateSize() {
    if (_platformView == nullptr) {
        return false;
    }

    auto size = _platformView->getSize();
    auto density = _platformView->getDensity();
    auto sizeChanged = _state->updateScreen(size.width, size.height, density);
    if (sizeChanged) {
        _platformView->invalidSize();
        _zoomPanController->setBounds(tgfx::Size::Make(size));
        updateContentSize();
        invalidateContent();
    }
    return sizeChanged;
}

float SeatCanvasCoreRenderer::getContentScale() const {
    return _state->getContentScale();
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
    return _zoomLevelConfig.venue < 1.0f;
}

float SeatCanvasCoreRenderer::showBackZoomThreshold() const {
    if (isSmallVenue()) {
        return _zoomLevelConfig.zone;
    }
    return _zoomLevelConfig.venue;
}

const tgfx::Color &SeatCanvasCoreRenderer::getBackgroundColor() const {
    return _backgroundColor;
}

void SeatCanvasCoreRenderer::setBackgroundColor(const tgfx::Color &color) {
    if (_backgroundColor == color) {
        return;
    }
    _backgroundColor = color;
    invalidateContent();
}

float SeatCanvasCoreRenderer::getSeatSize() const {
    return _seatSize;
}

void SeatCanvasCoreRenderer::setSeatSize(float seatSize) {
    if (_seatSize == seatSize) {
        return;
    }
    _seatSize = seatSize;
    _customSeatPass->setSeatSize(_seatSize);
    invalidateContent();
}

void SeatCanvasCoreRenderer::setStyleKeyToConfig(const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleKeyToConfig) {
    auto changed = _seatAtlasManager->setStyleKeyToConfigs(styleKeyToConfig);
    if (changed) {
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::setStyleKeyToConfigFromJSON(const void *bytes, size_t len) {
    if (!bytes || len == 0) {
        setStyleKeyToConfig({});
        return;
    }

    std::string jsonString(reinterpret_cast<const char *>(bytes), len);

    if (!nlohmann::json::accept(jsonString)) {
        tgfx::PrintError("Invalid JSON format");
        return;
    }

    auto json = nlohmann::json::parse(jsonString, nullptr, false);
    if (json.is_discarded()) {
        tgfx::PrintError("Failed to parse JSON");
        return;
    }

    if (!json.is_array()) {
        tgfx::PrintError("Invalid JSON: expected array");
        return;
    }

    std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> styleKeyToConfig;

    for (const auto &entry : json) {
        if (!entry.contains("key") || !entry.contains("config")) {
            tgfx::PrintError("Invalid JSON entry: missing 'key' or 'config'");
            continue;
        }

        if (!entry["key"].is_object()) {
            tgfx::PrintError("Invalid JSON entry: 'key' is not an object");
            continue;
        }

        kk::SeatStyleKey key;
        entry["key"].get_to(key);

        if (!entry["config"].is_object()) {
            tgfx::PrintError("Invalid JSON entry: 'config' is not an object");
            continue;
        }

        std::shared_ptr<SeatStyleConfig> config;
        entry["config"].get_to(config);
        if (!config) {
            tgfx::PrintError("Failed to parse config");
            continue;
        }

        styleKeyToConfig[key] = config;
    }

    setStyleKeyToConfig(styleKeyToConfig);
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

    auto window = _platformView ? _platformView->getWindow() : nullptr;
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

    _seatAtlasManager->attachContext(context);
    auto canvas = surface->getCanvas();
    if (canvas == nullptr) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group);
        return;
    }

    auto statePtr = _state.get();

    PROFILE_STAGE_START(group, prepareZoneName, "Prepare Zone Name");
    _seatZoneNameLayer->prepare(canvas, statePtr, force);
    PROFILE_STAGE_END(group, prepareZoneName);

    PROFILE_STAGE_START(group, prepareMini, "Prepare Overlay");
    _overlayLayer->prepare(canvas, statePtr, force);
    PROFILE_STAGE_END(group, prepareMini);

    bool hasContentChanged = _seatZoneNameLayer->hasContentChanged() || _overlayLayer->hasContentChanged();
    auto skipCurrentFrame = (!hasContentChanged && !force && !_invalidate);
    if (skipCurrentFrame) {
        PROFILE_GROUP_DISABLE_AUTO_LOG(group)
        return;
    }

    PROFILE_STAGE_START(group, prepareSeat, "Prepare Seat");
    _seatAtlasManager->update(statePtr->getDensity(), {_seatSize, _seatSize});
    prepareSeatIfNeeded();
    PROFILE_STAGE_END(group, prepareSeat);

    canvas->clear(_backgroundColor);

    PROFILE_STAGE_START(group, customPass, "Custom Pass");
    executeCustomRenderPass(context, statePtr);
    PROFILE_STAGE_END(group, customPass);
    if (auto baseMapImage = _customBaseMapPass->outputImage(); baseMapImage) {
        canvas->drawImage(baseMapImage);
    }

    PROFILE_STAGE_START(group, canvasZoneName, "Canvas Zone Name");
    canvas->save();
    _seatZoneNameLayer->draw(canvas, statePtr);
    canvas->restore();
    PROFILE_STAGE_END(group, canvasZoneName);

    if (shouldAutoDrawSeat() && _customSeatPass->hasData()) {
        if (auto seatImage = _customSeatPass->outputImage(); seatImage) {
            canvas->drawImage(seatImage);
        }
    }

    PROFILE_STAGE_START(group, canvasOverlay, "Canvas Overlay");
    canvas->save();
    _overlayLayer->draw(canvas, statePtr);
    drawFPS(canvas);
    canvas->restore();
    PROFILE_STAGE_END(group, canvasOverlay);

    PROFILE_STAGE_START(group, flush, "Canvas Flush");
    auto recording = context->flush();
    PROFILE_STAGE_END(group, flush);
    if (recording) {
        PROFILE_STAGE_START(group, Submit, "Canvas Submit");
        context->submit(std::move(recording));
        PROFILE_STAGE_END(group, Submit);
    }

    PROFILE_STAGE_START(group, present, "Present");
    window->present(context);
    PROFILE_STAGE_END(group, present);

    _invalidate = false;
}

bool SeatCanvasCoreRenderer::executeCustomRenderPass(tgfx::Context *context, const SeatCanvasCoreRendererState *state) {
    if (!context || !state) {
        return false;
    }

    // 通过 IContextAware 接口统一更新 Context
    _customBaseMapPass->attachContext(context);
    _customSeatPass->attachContext(context);

    auto gpu = context->gpu();
    if (gpu == nullptr) {
        return false;
    }

    auto encoder = gpu->createCommandEncoder();
    if (!encoder) {
        return false;
    }

    auto result = _customBaseMapPass->onDraw(encoder.get(), state);
    if (shouldAutoDrawSeat() && _customSeatPass->hasData()) {
        auto atlasTexture = _seatAtlasManager->getAtlasTexture();
        _customSeatPass->setAtlasTexture(atlasTexture);
        _customSeatPass->setSeatSize(_seatSize);
        result &= _customSeatPass->onDraw(encoder.get(), state);
    }

    auto commandBuffer = encoder->finish();
    if (commandBuffer) {
        gpu->queue()->submit(commandBuffer);
    }
    return result;
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
    auto fontSize = kk::utils::fp2px(24.0f);
    auto textBlob = _textShaper->shape(text, nullptr, fontSize);
    if (!textBlob) {
        return;
    }

    auto bounds = textBlob->getTightBounds();
    tgfx::Paint paint;
    if (fps < 30) {
        paint.setColor(tgfx::Color::Red());
    } else {
        paint.setColor(tgfx::Color::Green());
    }
    // drawTextBlob 的 Y 坐标是基线位置，需要调整以使文本顶部在期望位置
    // getTightBounds() 返回的 bounds 是相对于基线的，bounds.y() 通常是负数（文本顶部在基线上方）
    // 所以要让文本顶部在 desiredTopY，基线应该在 desiredTopY - bounds.y()
    auto desiredTopY = kk::utils::vp2px(10.0f);
    auto baselineY = desiredTopY - bounds.y();
    canvas->drawTextBlob(std::move(textBlob), kk::utils::vp2px(10.0f), baselineY, paint);
}

tgfx::Rect SeatCanvasCoreRenderer::getVisibleOriginalRect() const {
    return _state->getVisibleOriginalRect();
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

tgfx::Point SeatCanvasCoreRenderer::convertNormalizedContentToOriginal(const tgfx::Point &normalizedContentLocation) const {
    auto density = _state->getDensity();
    auto contentScale = _state->getContentScale();
    auto scaleFactor = contentScale * density;
    return tgfx::Point::Make(
        normalizedContentLocation.x / scaleFactor,
        normalizedContentLocation.y / scaleFactor);
}

tgfx::Point SeatCanvasCoreRenderer::convertOriginalToNormalizedContent(const tgfx::Point &originalLocation) const {
    auto density = _state->getDensity();
    auto contentScale = _state->getContentScale();
    auto scaleFactor = contentScale * density;
    return tgfx::Point::Make(
        originalLocation.x * scaleFactor,
        originalLocation.y * scaleFactor);
}

tgfx::Point SeatCanvasCoreRenderer::convertScreenToOriginal(const tgfx::Point &screenLocation) const {
    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();
    auto normalizedContentLocation = convertScreenToContent(screenLocation, contentOffset, zoomScale);
    return convertNormalizedContentToOriginal(normalizedContentLocation);
}

tgfx::Point SeatCanvasCoreRenderer::convertOriginalToScreen(const tgfx::Point &originalLocation) const {
    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();
    auto normalizedContentLocation = convertOriginalToNormalizedContent(originalLocation);
    return convertContentToScreen(normalizedContentLocation, contentOffset, zoomScale);
}

bool SeatCanvasCoreRenderer::isPointInContentArea(const tgfx::Point &screenLocation) const {
    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();
    auto normalizedContentSize = _zoomPanController->getContentSize();

    // 将屏幕坐标转换为规范化内容坐标
    auto contentLocation = convertScreenToContent(screenLocation, contentOffset, zoomScale);

    // 判断是否在内容区域内
    tgfx::Rect contentRect = tgfx::Rect::MakeWH(normalizedContentSize.width, normalizedContentSize.height);
    return contentRect.contains(contentLocation.x, contentLocation.y);
}

void SeatCanvasCoreRenderer::zoomToRect(const tgfx::Rect &rect, bool animated, float padding, double durationMs) {
    if (_zoomPanController == nullptr) {
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

    // rect 是原始坐标系中的包围盒，需要转换为规范化内容坐标系（像素单位）
    // 转换公式：normalizedContentCoord = originalCoord * contentScale * density
    tgfx::Rect rectInContentCoords = rect;
    rectInContentCoords.scale(contentScale * density, contentScale * density);

    // 添加边距（边距也是在原始坐标系中，需要转换）
    float paddingInContentCoords = padding * contentScale * density;
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
        // getVisibleOriginalRect() 返回的是原始坐标系，rect 也是原始坐标系，可以直接比较
        tgfx::Rect visibleRect = getVisibleOriginalRect();

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
        setBaseMapLayer(tgfx::Size::MakeEmpty());
        setMiniMapLayer(nullptr);
        _seatZoneNameLayer->setTextRootLayer(nullptr, {});
        _useBaseMapConfig.reset();

        applyBaseMapColorState(kk::BaseMapColorState::Original);
        _customBaseMapPass->updateMeshBuilder(nullptr);
        _customSeatPass->clearSeats();
    } else {
        setBaseMapLayer(config->baseMapSize());
        _seatZoneNameLayer->setTextRootLayer(config->textLayer(), config->baseMapSize());
        setMiniMapLayer(config->minimapLayer());
        _useBaseMapConfig = config;

        applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        applySavedSeatZoneAlternateColors(config->meshBuilder());
        _customBaseMapPass->updateMeshBuilder(config->meshBuilder());

        _customSeatPass->clearSeats();
    }

    handleBaseMapChanged();
}

void SeatCanvasCoreRenderer::handleBaseMapChanged() {
    updateContentScale();
    updateContentSize();
}

void SeatCanvasCoreRenderer::setBaseMapLayer(const tgfx::Size &baseMapSize) {
    _state->updateOriginSize(baseMapSize);
}

void SeatCanvasCoreRenderer::setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer) {
    // minimap 使用与 baseMap 相同的尺寸
    tgfx::Size baseMapSize = _state->getOriginSize();
    _overlayLayer->setBaseMapLayer(std::move(layer), baseMapSize);
}

/// 设置选中的区域ID（仅对 ClickToEnter 模式有效）
/// @param zoneId 区域ID，为空表示取消选择，恢复到全区域视图
void SeatCanvasCoreRenderer::setSelectedzoneId(const std::string &zoneId) {
    if (zoneId.empty()) {
        updateUseBaseMapConfig(_baseMapConfig);
        return;
    }

    // TODO: 构建虚拟BaseMapLayer
}

/// 为区域创建虚拟的 BaseMapLayer
/// @param zoneId 区域ID
std::shared_ptr<kk::layer::BaseMapRootLayer> SeatCanvasCoreRenderer::buildVirtualBaseMapLayerForZone(const std::string &zoneId) {
    return nullptr;
}

void SeatCanvasCoreRenderer::updateContentScale() {
    auto originSize = _state->getOriginSize();
    if (!originSize.isEmpty() && originSize.width > _maxWidth) {
        auto scale = _maxWidth / originSize.width;
        _state->updateContentScale(scale);
    } else {
        _state->updateContentScale(1.0f);
    }
}

void SeatCanvasCoreRenderer::updateContentSize() {
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

void SeatCanvasCoreRenderer::updateMaxMinZoomScalesForCurrentBounds() {
    auto boundsSize = _state->getBoundsSize();
    auto normalizedContentSize = _state->getNormalizedContentSize();
    auto contentScale = _state->getContentScale();
    auto density = _state->getDensity();
    if (boundsSize.isEmpty() || normalizedContentSize.isEmpty()) {

        _zoomPanController->setMinimumZoomScale(1.0f);
        _zoomPanController->setMaximumZoomScale(1.0f);
        _zoomPanController->setZoomScale(1.0f);

        _zoomLevelConfig.seat = 1.0f;
        _zoomLevelConfig.row = 1.0f;
        _zoomLevelConfig.zone = 1.0f;
        _zoomLevelConfig.venue = 1.0f;

        updateZoomPanControllerState();
        return;
    }

    const auto &contentInset = _zoomPanController->getContentInset();

    auto viewWidth = boundsSize.width - contentInset.left - contentInset.right;
    auto minimumZoomScale = viewWidth / normalizedContentSize.width;
    // 适配横屏
    if (boundsSize.width > boundsSize.height) {
        viewWidth = boundsSize.height - contentInset.top - contentInset.bottom;
        minimumZoomScale = viewWidth / normalizedContentSize.height;
    }

    auto unitWidth = (contentScale * kk::ZoomScaleConfig::SEAT_BASE_SIZE) / _svgModelScale;

    // viewWidth 是像素单位 所以最后需要转为 pt 单位
    _zoomLevelConfig.seat = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_SMALL)) / density;
    _zoomLevelConfig.row = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_MEDIUM)) / density;
    _zoomLevelConfig.zone = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_LARGE)) / density;
    _zoomLevelConfig.venue = (viewWidth / (unitWidth * kk::ZoomScaleConfig::ZOOM_LEVEL_XLARGE)) / density;

    _zoomLevelConfig.venue = std::max(_zoomLevelConfig.venue, minimumZoomScale);

    float baseScale = 1.0f / (contentScale / _svgModelScale);
    float maximumZoomScale = std::max(_zoomLevelConfig.seat, baseScale);

    _zoomPanController->setMinimumZoomScale(static_cast<float>(minimumZoomScale));
    _zoomPanController->setMaximumZoomScale(static_cast<float>(maximumZoomScale));
    _zoomPanController->setZoomScale(static_cast<float>(minimumZoomScale));

    tgfx::PrintLog("updateMaxMinZoomScalesForCurrentBounds: min %f max %f seat %f row %f zone %f venue %f",
                   minimumZoomScale,
                   maximumZoomScale,
                   _zoomLevelConfig.seat,
                   _zoomLevelConfig.row,
                   _zoomLevelConfig.zone,
                   _zoomLevelConfig.venue);
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

bool SeatCanvasCoreRenderer::shouldAutoDrawSeat() const {
    if (_disableAutoDrawSeat) {
        return false;
    }
    auto currentZoomScale = _zoomPanController->getZoomScale();
    return currentZoomScale > getSeatRenderZoomThreshold();
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

    auto zoomScale = _state->getZoomScale();
    auto showBack = zoomScale >= showBackZoomThreshold();

    if (_animator && _minimapAnimationId != 0) {
        _animator->cancel(_minimapAnimationId);
        _minimapAnimationId = 0;
    }

    kk::animation::AnimationOptions options{};
    options.durationMs = 350.0;
    options.delayMs = 1500.0;
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

void SeatCanvasCoreRenderer::prepareSeatIfNeeded() {
    if (!_state) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto meshBuilder = baseMapConfig->meshBuilder();
    if (meshBuilder == nullptr) {
        return;
    }

    auto zoomScale = _state->getZoomScale();

    /*
     * 缩小到一定级别后，不显示座位
     * zoomScale 越小表示缩得越小，seatRenderZoomThreshold 是一个较小的缩放值
     * 所以当 zoomScale < seatRenderZoomThreshold 时，应该隐藏座位
     */
    if (zoomScale < getSeatRenderZoomThreshold()) {
        _customSeatPass->clearSeats();
        if (_autoChangeBaseMapColorState) {
            applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        }
        return;
    }

    if (_autoChangeBaseMapColorState) {
        applyBaseMapColorState(kk::BaseMapColorState::Original);
    }

    if (!shouldAutoDrawSeat()) {
        _customSeatPass->clearSeats();
        return;
    }

    auto visibleOriginalRect = getVisibleOriginalRect();
    if (visibleOriginalRect.isEmpty()) {
        _customSeatPass->clearSeats();
        return;
    }

    /*
     * 扩大一点点，避免出现刚好在边缘的隐藏/显示，视觉上体验不好
     */
    visibleOriginalRect.outset(60, 60);

    auto zones = meshBuilder->findZoneIntersectingRect(visibleOriginalRect);
    if (zones.empty()) {
        _customSeatPass->clearSeats();
        return;
    }

    std::vector<SeatInstanceData> instances{};
    for (const auto &zone : zones) {
        if (!zone) {
            continue;
        }

        auto iter = _seatDataMap.find(zone->zoneId);
        if (iter == _seatDataMap.end()) {
            continue;
        }

        auto partial = !visibleOriginalRect.contains(zone->fillBounds);
        for (const auto &seat : iter->second) {
            if (partial && !tgfx::Rect::Intersects(visibleOriginalRect, tgfx::Rect::MakeXYWH(seat.x, seat.y, _seatSize, _seatSize))) {
                continue;
            }

            auto uvOffsetIndex = _seatAtlasManager->getUVOffsetIndex(seat.status, seat.selected);
            if (uvOffsetIndex == -1) {
                continue;
            }
            float rotationRad = seat.rotation * (M_PI / 180.0f);
            instances.emplace_back(seat.x, seat.y, uvOffsetIndex, rotationRad);
        }
    }

    _customSeatPass->updateSeats(std::move(instances));
}

void SeatCanvasCoreRenderer::handleZoomBack() {
    if (!_zoomPanController) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
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

    // ------------------- 计算目标居中 offset -------------------
    float effectiveWidth = viewport.width - contentInset.left - contentInset.right;
    float effectiveHeight = viewport.height - contentInset.top - contentInset.bottom;

    float scaledW = normalizedContentSize.width * targetZoom;
    float scaledH = normalizedContentSize.height * targetZoom;

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
    _disableAutoDrawSeat = true;
    applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
    _overlayLayer->setBackVisible(false);

    invalidateContent();

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
        _disableAutoDrawSeat = false;
        if (finish && _zoomPanController) {
            _zoomPanController->setZoomScale(targetZoom);
            _zoomPanController->setContentOffset(targetOffset);
            updateZoomPanControllerState();
        }
    };

    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void SeatCanvasCoreRenderer::handleSeatSelectionAtLocation(const tgfx::Point &location) {
    if (!_delegate || !shouldAutoDrawSeat()) {
        return;
    }

    auto config = _useBaseMapConfig.lock();
    if (!config) {
        return;
    }

    auto meshBuilder = config->meshBuilder();
    if (!meshBuilder) {
        return;
    }

    auto zoomScale = _zoomPanController->getZoomScale();
    auto contentOffset = _zoomPanController->getContentOffset();

    // 转换为原始坐标系
    auto normalizedContentLocation = convertScreenToContent(location, contentOffset, zoomScale);
    auto originalLocation = convertNormalizedContentToOriginal(normalizedContentLocation);
    auto zoneInfo = meshBuilder->findZoneContainingPoint(originalLocation);
    if (!zoneInfo) {
        return;
    }

    auto iter = _seatDataMap.find(zoneInfo->zoneId);
    if (iter == _seatDataMap.end()) {
        return;
    }

    for (auto &seatInfo : iter->second) {
        auto rect = tgfx::Rect::MakeXYWH(seatInfo.x, seatInfo.y, _seatSize, _seatSize);
        if (!rect.contains(originalLocation.x, originalLocation.y)) {
            continue;
        }

        if (seatInfo.selected) {
            seatInfo.selected = false;
            _delegate->didDeselectSeat(_coreID, zoneInfo->zoneId, seatInfo.seatId);
            invalidateContent();
        } else {
            auto canSelected = _delegate->shouldSelectSeat(_coreID, zoneInfo->zoneId, seatInfo.seatId);
            if (!canSelected) {
                return;
            }
            seatInfo.selected = true;
            _delegate->didSelectSeat(_coreID, zoneInfo->zoneId, seatInfo.seatId);
            invalidateContent();
        }
        _customSeatPass->clearSeats();
        showMinimapWithoutAnimation();
        return;
    }
}

void SeatCanvasCoreRenderer::handleAutoZoomOnTap(const tgfx::Point &location) {
    if (!_state) {
        return;
    }

    // 检查点击位置是否在内容区域内，留白区域不做处理
    if (!isPointInContentArea(location)) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto meshBuilder = baseMapConfig->meshBuilder();
    if (!meshBuilder) {
        return;
    }

    // 将屏幕坐标转换为原始坐标
    auto originalLocation = convertScreenToOriginal(location);

    // 在 meshBuilder 中查找区域
    auto zoneInfo = meshBuilder->findZoneContainingPoint(originalLocation);
    if (!zoneInfo) {
        // 点击位置不在任何区域内
        scrollViewWithLocation(location);
        return;
    }

    // 如果已经缩放到座位级别（seatRenderZoomThreshold），则执行点击位置的渐进式缩放
    auto zoomScale = _zoomPanController->getZoomScale();
    if (zoomScale >= getSeatRenderZoomThreshold() || isSmallVenue()) {
        scrollViewWithLocation(location);
        return;
    }

    // 使用找到的区域进行缩放
    if (!zoneInfo->zoneId.empty()) {
        if (_delegate) {
            _delegate->didTapZone(_coreID, zoneInfo->zoneId);
        }
        scrollViewWithZone(zoneInfo);
        return;
    }

    scrollViewWithLocation(location);
}

static tgfx::Point ComputeClampedOffset(const tgfx::Point &screenPoint, const tgfx::Point &contentPoint, float zoom, const tgfx::Size &viewportSize, const tgfx::Size &contentSize, const EdgeInsets &contentInset) {
    // 计算有效视口范围（考虑 contentInset）
    const float effectiveWidth = viewportSize.width - contentInset.left - contentInset.right;
    const float effectiveHeight = viewportSize.height - contentInset.top - contentInset.bottom;

    float ox = screenPoint.x - contentPoint.x * zoom;
    float oy = screenPoint.y - contentPoint.y * zoom;

    const float scaledW = contentSize.width * zoom;
    const float scaledH = contentSize.height * zoom;

    float minX, maxX, minY, maxY;

    if (scaledW <= effectiveWidth) {
        // 内容比有效视口小，居中
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

void SeatCanvasCoreRenderer::scrollViewWithLocation(const tgfx::Point &location) {
    if (!_state) {
        return;
    }

    auto zoomScale = _state->getZoomScale();
    // === 情况1：已经是最大缩放（座位级别）===
    if (zoomScale >= _zoomLevelConfig.seat) {
        return;
    }

    // === 情况2：从远景放大到中景 ===
    if (zoomScale < getSeatRenderZoomThreshold()) {
        // 根据场馆类型选择目标缩放
        float target = zoomScale;
        if (isSmallVenue()) {
            target = _zoomLevelConfig.row;
        } else {
            target = _zoomLevelConfig.zone;
        }
        zoomToPoint(location, target, true, 20.0f, 300.0);
        return;
    }

    // === 情况3：从中景放大到中景+ ===
    if (zoomScale < _zoomLevelConfig.zone) {
        // 放大到 18格
        zoomToPoint(location, _zoomLevelConfig.row, true, 20.0f, 300.0);
        return;
    }

    // === 情况4：从中景+放大到近景 ===
    auto diff = std::fabs(_zoomLevelConfig.seat - zoomScale);
    if (diff > FLT_EPSILON) {
        zoomToPoint(location, _zoomLevelConfig.seat, true, 20.0f, 300.0);
    }
}

void SeatCanvasCoreRenderer::scrollViewWithZone(const std::shared_ptr<ZoneMeshInfo> &zoneInfo) {
    if (!zoneInfo) {
        return;
    }

    if (_zoomPanController == nullptr || _state == nullptr) {
        return;
    }

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return;
    }

    auto meshBuilder = baseMapConfig->meshBuilder();
    if (meshBuilder == nullptr) {
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

    // ---------------- 1. 计算规范化内容坐标下区域边界 ----------------
    auto contentBounds = bounds;
    contentBounds.scale(contentScale * density, contentScale * density);

    float contentCenterX = contentBounds.centerX();
    float contentCenterY = contentBounds.centerY();

    // ---------------- 2. 计算有效视口区域 ----------------
    float effectiveWidth = viewport.width - contentInset.left - contentInset.right;
    float effectiveHeight = viewport.height - contentInset.top - contentInset.bottom;
    if (effectiveWidth <= 0 || effectiveHeight <= 0) {
        return;
    }

    // ---------------- 3. 计算目标缩放比例 ----------------
    float contentZoneWidth = std::fmax(1.0f, contentBounds.width());
    float contentZoneHeight = std::fmax(1.0f, contentBounds.height());

    float scaleX = effectiveWidth / contentZoneWidth;
    float scaleY = effectiveHeight / contentZoneHeight;
    float targetZoomScale = std::min(scaleX, scaleY);
    targetZoomScale = std::max(getSeatRenderZoomThreshold(), targetZoomScale);

    // clamp 到有效范围
    const float minZoom = _zoomPanController->getMinimumZoomScale();
    const float maxZoom = _zoomPanController->getMaximumZoomScale();
    targetZoomScale = std::clamp(targetZoomScale, minZoom, maxZoom);

    // ---------------- 4. 计算区域在目标缩放下的尺寸 ----------------
    float scaledZoneWidth = contentZoneWidth * targetZoomScale;
    float scaledZoneHeight = contentZoneHeight * targetZoomScale;

    // ---------------- 5. 计算目标屏幕位置（优先居中，边界时clamp）----------------
    // 先尝试让区域中心对齐视口中心
    float centerX = contentInset.left + effectiveWidth / 2;
    float centerY = contentInset.top + effectiveHeight / 2;

    float targetScreenX = centerX;
    float targetScreenY = centerY;

    // 计算区域中心在屏幕上的有效范围（边界限制）
    float minScreenX = contentInset.left + scaledZoneWidth / 2;
    float maxScreenX = viewport.width - contentInset.right - scaledZoneWidth / 2;
    float minScreenY = contentInset.top + scaledZoneHeight / 2;
    float maxScreenY = viewport.height - contentInset.bottom - scaledZoneHeight / 2;

    // 如果区域太大无法居中，则clamp到有效范围
    if (minScreenX < maxScreenX) {
        targetScreenX = std::clamp(targetScreenX, minScreenX, maxScreenX);
    }
    if (minScreenY < maxScreenY) {
        targetScreenY = std::clamp(targetScreenY, minScreenY, maxScreenY);
    }

    // 计算缩放后规范化内容总尺寸（用于后续offset计算）
    float scaledContentWidth = normalizedContentSize.width * targetZoomScale;
    float scaledContentHeight = normalizedContentSize.height * targetZoomScale;

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
    _disableAutoDrawSeat = currentZoomScale <= getSeatRenderZoomThreshold();
    auto showBack = targetZoomScale >= showBackZoomThreshold();
    if (showBack) {
        applyBaseMapColorState(kk::BaseMapColorState::Original);
        if (_overlayLayer->backVisible()) {
            showBack = false;
        } else {
            _overlayLayer->setBackVisible(true);
        }
    } else {
        applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        _overlayLayer->setBackVisible(false);
    }

    invalidateContent();

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
        _disableAutoDrawSeat = false;
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

    const auto viewport = _zoomPanController->getBounds();
    const auto normalizedContentSize = _zoomPanController->getContentSize();

    if (viewport.isEmpty() || normalizedContentSize.isEmpty()) {
        return;
    }

    // ---- 1. clamp zoom ----
    const float minZoom = _zoomPanController->getMinimumZoomScale();
    const float maxZoom = _zoomPanController->getMaximumZoomScale();
    const float targetZoomScale = std::clamp(scale, minZoom, maxZoom);

    const float currentZoomScale = _zoomPanController->getZoomScale();
    const tgfx::Point currentOffset = _zoomPanController->getContentOffset();
    const auto contentInset = _zoomPanController->getContentInset();

    // ---- 2. screen -> normalized content（当前 zoom 下）----
    const tgfx::Point contentPoint{
        (location.x - currentOffset.x) / currentZoomScale,
        (location.y - currentOffset.y) / currentZoomScale};

    // ---- 3. 计算起始和目标 offset（都进行 clamp，考虑 contentInset）----
    // 计算起始 offset（基于当前 zoom），确保在有效范围内
    const tgfx::Point startOffset = ComputeClampedOffset(location, contentPoint, currentZoomScale, viewport, normalizedContentSize, contentInset);
    // 计算目标 offset（基于目标 zoom），确保在有效范围内
    const tgfx::Point finalOffset = ComputeClampedOffset(location, contentPoint, targetZoomScale, viewport, normalizedContentSize, contentInset);

    // ---- 4. 动画准备 ----
    _zoomPanController->stopAllAnimations();
    hideMinimapWithoutAnimation();
    _animator->cancelAll();

    _autoChangeBaseMapColorState = false;
    _disableAutoDrawSeat = currentZoomScale <= getSeatRenderZoomThreshold();
    auto showBack = targetZoomScale >= showBackZoomThreshold();
    if (showBack) {
        applyBaseMapColorState(kk::BaseMapColorState::Original);
        if (_overlayLayer->backVisible()) {
            showBack = false;
        } else {
            _overlayLayer->setBackVisible(true);
        }
    } else {
        applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        _overlayLayer->setBackVisible(false);
    }

    invalidateContent();

    if (!animated || durationMs <= 0.0) {
        _autoChangeBaseMapColorState = true;
        _disableAutoDrawSeat = false;
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

    // ---- 5. 动画更新（对 zoom 和 offset 都进行插值）----
    // 关键优化：对 offset 也进行插值，而不是每次都重新计算
    // 这样可以确保 offset 的变化是平滑的，避免在最后几帧出现大的跳跃
    auto update = [this, currentZoomScale, targetZoomScale, startOffset, finalOffset, showBack](float progress) {
        if (!_zoomPanController) {
            return;
        }

        // 对 zoom 进行插值
        const float zoom = currentZoomScale + (targetZoomScale - currentZoomScale) * progress;

        // 对 offset 也进行插值，确保变化平滑
        // 这样即使 offset 被 clamp 了，变化也是连续的
        const tgfx::Point interpolatedOffset{
            startOffset.x + (finalOffset.x - startOffset.x) * progress,
            startOffset.y + (finalOffset.y - startOffset.y) * progress};

        const float currentZoom = _zoomPanController->getZoomScale();
        const tgfx::Point currentOffset = _zoomPanController->getContentOffset();

        _zoomPanController->setZoomScale(zoom, false);
        _zoomPanController->setContentOffset(interpolatedOffset, false);

        if (showBack && _overlayLayer) {
            _overlayLayer->setBackAlpha(progress);
        }

        updateZoomPanControllerState();
    };

    // ---- 7. 动画结束，落到精确目标态 ----
    auto completion = [this](bool finished) {
        _autoChangeBaseMapColorState = true;
        _disableAutoDrawSeat = false;
        if (finished && _zoomPanController) {
            updateZoomPanControllerState();
        }
    };

    _animator->play(options, startTime, std::move(update), std::move(completion));
}

void SeatCanvasCoreRenderer::applyBaseMapColorState(BaseMapColorState toState) {
    if (_baseMapColorState == toState) {
        return;
    }

    auto config = _useBaseMapConfig.lock();
    if (!config) {
        return;
    }

    auto textLayer = config->textLayer();
    // 设置文本图层透明度
    if (toState == kk::BaseMapColorState::Original) {
        if (textLayer) {
            textLayer->setAlpha(0.35f);
        }
    } else if (toState == kk::BaseMapColorState::Rainbow) {
        if (textLayer) {
            textLayer->setAlpha(1.0f);
        }
    }

    _customBaseMapPass->updateColorState(toState);
    _baseMapColorState = toState;
}

void SeatCanvasCoreRenderer::updateSeatZoneAlternateColors(const std::unordered_map<std::string, tgfx::Color> &colors) {
    if (_zoneColorMap != colors) {
        _zoneColorMap = colors;
    }

    auto config = _useBaseMapConfig.lock();
    if (!config) {
        return;
    }

    auto meshBuilder = config->meshBuilder();
    if (!meshBuilder) {
        return;
    }

    const auto &zoneMeshInfos = meshBuilder->getZoneMeshInfos();
    const auto iterEnd = colors.end();
    for (auto &zone : zoneMeshInfos) {
        if (zone->zoneId.empty()) {
            zone->alternateColor.reset();
            continue;
        }
        const auto iter = colors.find(zone->zoneId);
        if (iter != iterEnd && iter->second != tgfx::Color::Transparent()) {
            zone->alternateColor = iter->second;
            continue;
        }
        zone->alternateColor.reset();
    }

    invalidateContent();
}

void SeatCanvasCoreRenderer::applySavedSeatZoneAlternateColors(std::shared_ptr<BaseMapMeshBuilder> meshBuilder) {
    if (!meshBuilder) {
        return;
    }

    updateSeatZoneAlternateColors(_zoneColorMap);
}

void SeatCanvasCoreRenderer::setSeatData(const std::string &zoneId, const std::vector<kk::SeatData> &seats) {
    if (zoneId.empty()) {
        return;
    }

    _seatDataMap[zoneId] = seats;
    _customSeatPass->clearSeats();
    invalidateContent();
}

void SeatCanvasCoreRenderer::updateSeatStatus(const std::string &zoneId, const std::string &seatId, uint32_t status) {
    if (zoneId.empty() || seatId.empty()) {
        return;
    }

    auto iter = _seatDataMap.find(zoneId);
    if (iter == _seatDataMap.end()) {
        return;
    }

    for (auto &seatInfo : iter->second) {
        if (seatInfo.seatId == seatId) {
            seatInfo.status = status;
            _customSeatPass->clearSeats();
            invalidateContent();
            return;
        }
    }
}

void SeatCanvasCoreRenderer::clearSeatData() {
    std::unordered_set<std::string> allzoneIds;
    allzoneIds.reserve(_seatDataMap.size());
    for (const auto &[zoneId, _] : _seatDataMap) {
        allzoneIds.insert(zoneId);
    }

    _seatDataMap.clear();

    _customSeatPass->clearSeats();

    invalidateContent();
}
};  // namespace kk::renderer
