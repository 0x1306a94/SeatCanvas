//
//  SeatCanvasCoreRenderer.cpp
//  SeatCraftCore
//
//  Created by king on 2025/11/11.
//

#include "SeatCanvasCoreRenderer.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Stream.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/CommandEncoder.h>
#include <tgfx/gpu/CommandQueue.h>
#include <tgfx/gpu/Device.h>
#include <tgfx/gpu/GPU.h>
#include <tgfx/gpu/RenderPass.h>
#include <tgfx/gpu/Texture.h>
#include <tgfx/gpu/Window.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>
#include <tgfx/svg/TextShaper.h>

#include "PlatformView.hpp"
#include "RenderFrameMetrics.hpp"
#include "SeatCanvasCoreRendererState.hpp"
#include "ViewportController.hpp"
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
#include "core/renderer/SeatDataManager.hpp"
#include "core/renderer/pass/CustomBaseMapPass.hpp"
#include "core/renderer/pass/CustomSeatPass.hpp"
#include "core/style/SeatStyleAtlasManager.hpp"
#include "core/style/SeatStyleConfig.hpp"
#include "core/style/SeatStyleConfigJSONHelper.hpp"
#include "core/utils/DisplayLink.hpp"
#include "core/utils/TimeProfiler.hpp"
#include "core/utils/UnitConverter.hpp"

namespace kk::renderer {
SeatCanvasCoreRenderer::SeatCanvasCoreRenderer(
    std::unique_ptr<PlatformView> platformView,
    std::unique_ptr<kk::gesture::ElasticZoomPanController> zoomPanController,
    const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig)
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
    _seatDataManager = std::make_unique<SeatDataManager>(_seatAtlasManager.get());
    // atlas 生成完成后同步 UV 偏移，并重建样式键查表
    _seatAtlasManager->setOnAtlasGenerated([this](const SeatStyleAtlasManager *atlasManager) {
        _customSeatPass->updateUVOffset(atlasManager->getUVOffsets());
        _seatDataManager->rebuildStyleKeyLookup();
    });

    _viewportController = std::make_unique<ViewportController>(
        _zoomPanController.get(),
        _state.get(),
        _animator.get(),
        _delegate,
        &_zoomLevelConfig,
        this);

    _viewportController->setCoreID(_coreID);

    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    updateSize();
}

SeatCanvasCoreRenderer::~SeatCanvasCoreRenderer() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

uint32_t SeatCanvasCoreRenderer::coreID() const {
    return _coreID;
}

const SeatCanvasCoreRendererState *SeatCanvasCoreRenderer::state() const {
    return _state.get();
}

void SeatCanvasCoreRenderer::setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate) {
    _delegate = delegate;
    _viewportController->setDelegate(std::move(delegate));
}

const kk::ZoomLevelConfig &SeatCanvasCoreRenderer::zoomLevelConfig() const {
    return _viewportController->zoomLevelConfig();
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
        _viewportController->updateContentSize();
        invalidateContent();

        if (!_useBaseMapConfig.expired()) {
            dispatchZoomLevelConfigUpdate();
        }
    }
    return sizeChanged;
}

void SeatCanvasCoreRenderer::setMaxWidth(float maxWidth) {
    _viewportController->setMaxWidth(maxWidth);
}

float SeatCanvasCoreRenderer::getMaxWidth() const {
    return _viewportController->getMaxWidth();
}

float SeatCanvasCoreRenderer::getContentScale() const {
    return _state->getContentScale();
}

float SeatCanvasCoreRenderer::getFPS() const {
    return _frameMetrics->currentFPS();
}

bool SeatCanvasCoreRenderer::isDebugHUDEnabled() const {
    return _debugHUDEnabled;
}

void SeatCanvasCoreRenderer::setDebugHUDEnabled(bool enabled) {
    if (_debugHUDEnabled == enabled) {
        return;
    }
    _debugHUDEnabled = enabled;
    invalidateContent();
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
    return kk::isSmallVenue(_zoomLevelConfig);
}

float SeatCanvasCoreRenderer::showBackZoomThreshold() const {
    return kk::showBackZoomThreshold(_zoomLevelConfig);
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

void SeatCanvasCoreRenderer::setStyleIdToConfig(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig) {
    if (_seatDataManager->setStyleIdToConfig(styleIdToConfig)) {
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::setStyleKeyToConfigFromJSON(const void *bytes, size_t len) {
    if (_seatDataManager->setStyleKeyToConfigFromJSON(bytes, len)) {
        invalidateContent();
    }
}

// 手势处理方法，由平台层调用

void SeatCanvasCoreRenderer::handleTap(const tgfx::Point &location) {
    if (_overlayLayer) {
        if (_overlayLayer->hitTestInMinimap(location)) {
            return;
        }

        if (_overlayLayer->hitTestInBack(location)) {
            _viewportController->handleZoomBack();
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
            if (_delegate) {
                _delegate->viewportWillBeginDragging(_coreID, makeViewportEvent());
            }
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

    // 手势 ENDED/CANCELLED 且仍有惯性/回弹待播放：先把控制器中的缩放与偏移同步进 _state，
    // 再派发 didEndDragging(decelerate:true)，保证 makeViewportEvent 与松手瞬间一致；本帧不派发 didScroll/didZoom。
    if ((state == kk::gesture::GestureState::ENDED || state == kk::gesture::GestureState::CANCELLED) && _zoomPanController->hasPendingAnimation()) {
        _viewportController->setPanAnimationActive(true);
        updateZoomPanControllerState(false);
        if (_delegate) {
            _delegate->viewportDidEndDragging(_coreID, makeViewportEvent(), true);
        }
        return;
    }

    updateZoomPanControllerState();
    if (state == kk::gesture::GestureState::ENDED || state == kk::gesture::GestureState::CANCELLED) {
        if (_delegate) {
            _delegate->viewportDidEndDragging(_coreID, makeViewportEvent(), false);
        }
    }
}

void SeatCanvasCoreRenderer::handlePinch(kk::gesture::GestureState state, float scale, const tgfx::Point &center) {

    switch (state) {
        case kk::gesture::GestureState::BEGAN: {
            showMinimapWithoutAnimation();
            if (_delegate) {
                _delegate->viewportWillBeginZooming(_coreID, makeViewportEvent());
            }
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
    if (state == kk::gesture::GestureState::ENDED || state == kk::gesture::GestureState::CANCELLED) {
        if (_delegate) {
            _delegate->viewportDidEndZooming(_coreID, makeViewportEvent());
        }
    }
}

void SeatCanvasCoreRenderer::setBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> baseMapConfig) {
    if (baseMapConfig == nullptr) {
        _baseMapConfig = nullptr;
        updateUseBaseMapConfig(nullptr);
        return;
    }

    if (_baseMapConfig == baseMapConfig) {
        return;
    }

    _baseMapConfig = std::move(baseMapConfig);
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
                    _viewportController->notifyViewportDidEndDeceleratingIfNeeded();
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
    _viewportController->resetAnimationState();
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
    auto surface = _platformView ? _platformView->getSurface(context) : nullptr;
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

    PROFILE_STAGE_START(group, prepareSeatAtlas, "Prepare Seat Atlas");
    _seatAtlasManager->update(statePtr->getDensity(), {_seatSize, _seatSize});
    PROFILE_STAGE_END(group, prepareSeatAtlas);

    PROFILE_STAGE_START(group, prepareSeatInstances, "Prepare Seat Instances");
    prepareSeatIfNeeded();
    PROFILE_STAGE_END(group, prepareSeatInstances);

    canvas->clear(_backgroundColor);

    PROFILE_STAGE_START(group, customPass, "Custom Pass");
    executeCustomRenderPass(context, statePtr);
    PROFILE_STAGE_END(group, customPass);
    if (auto baseMapImage = _customBaseMapPass->outputImage(); baseMapImage) {
        canvas->drawImage(baseMapImage);
    }

    PROFILE_STAGE_START(group, canvasZoneName, "Canvas Zone Name");
    _seatZoneNameLayer->draw(canvas, statePtr);
    PROFILE_STAGE_END(group, canvasZoneName);

    if (shouldAutoDrawSeat() && _customSeatPass->hasData()) {
        if (auto seatImage = _customSeatPass->outputImage(); seatImage) {
            canvas->drawImage(seatImage);
        }
    }

    PROFILE_STAGE_START(group, canvasOverlay, "Canvas Overlay");
    _overlayLayer->draw(canvas, statePtr);
    drawDebugHUD(canvas);
    PROFILE_STAGE_END(group, canvasOverlay);

    PROFILE_STAGE_START(group, flush, "Canvas Flush");
    auto recording = context->flush();
    PROFILE_STAGE_END(group, flush);
    if (recording) {
        PROFILE_STAGE_START(group, Submit, "Canvas Submit");
        context->submit(std::move(recording));
        PROFILE_STAGE_END(group, Submit);
    }

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

void SeatCanvasCoreRenderer::drawDebugHUD(tgfx::Canvas *canvas) {
    if (!_debugHUDEnabled || canvas == nullptr || !_textShaper || _frameMetrics->isFirstFrame()) {
        return;
    }

    tgfx::AutoCanvasRestore restore(canvas);
    struct HudLine {
        std::string text = {};
        tgfx::Color color = {tgfx::Color::White()};
    };

    size_t zoneCount = _seatDataManager->getZoneCount();
    size_t seatCount = _seatDataManager->getTotalSeatCount();

    const int fps = static_cast<int>(_frameMetrics->currentFPS());
    char buffer[160] = {};

    std::vector<HudLine> lines = {};
    lines.reserve(9);
    lines.push_back(HudLine{
        "FPS: " + std::to_string(fps),
        fps < 30 ? tgfx::Color::Red() : tgfx::Color::Green(),
    });

    std::snprintf(buffer, sizeof(buffer), "Zoom: %.3f", getZoomScale());
    lines.push_back(HudLine{buffer, tgfx::Color::White()});

    std::snprintf(buffer, sizeof(buffer), "Loaded: %zu zones  %zu seats", zoneCount, seatCount);
    lines.push_back(HudLine{buffer, tgfx::Color::White()});

    std::snprintf(buffer, sizeof(buffer), "Drawn: %zu zones  %zu seats", _renderedSeatZoneCount, _renderedSeatCount);
    lines.push_back(HudLine{buffer, tgfx::Color::White()});

    std::snprintf(buffer, sizeof(buffer), "Seat thresh: %.3f", getSeatRenderZoomThreshold());
    lines.push_back(HudLine{buffer, tgfx::Color::White()});

    std::snprintf(buffer, sizeof(buffer), "Level seat: %.3f", _zoomLevelConfig.seat);
    lines.push_back(HudLine{buffer, tgfx::Color::White()});
    std::snprintf(buffer, sizeof(buffer), "Level row: %.3f", _zoomLevelConfig.row);
    lines.push_back(HudLine{buffer, tgfx::Color::White()});
    std::snprintf(buffer, sizeof(buffer), "Level zone: %.3f", _zoomLevelConfig.zone);
    lines.push_back(HudLine{buffer, tgfx::Color::White()});
    std::snprintf(buffer, sizeof(buffer), "Level venue: %.3f", _zoomLevelConfig.venue);
    lines.push_back(HudLine{buffer, tgfx::Color::White()});

    const float fontSize = kk::utils::fp2px(11.0f);
    const float padding = kk::utils::vp2px(8.0f);
    const float lineSpacing = fontSize * 1.35f;
    const float cornerRadius = kk::utils::vp2px(6.0f);
    const float originX = kk::utils::vp2px(10.0f);
    const float originY = kk::utils::vp2px(10.0f);

    struct ShapedHudLine {
        std::shared_ptr<tgfx::TextBlob> blob = {nullptr};
        tgfx::Color color = {tgfx::Color::White()};
        float topY = 0.0f;
    };

    std::vector<ShapedHudLine> shapedLines = {};
    shapedLines.reserve(lines.size());

    float maxTextWidth = 0.0f;
    float contentTop = originY + padding;
    for (const auto &line : lines) {
        auto blob = _textShaper->shape(line.text, nullptr, fontSize);
        if (!blob) {
            continue;
        }

        auto bounds = blob->getTightBounds();
        maxTextWidth = std::max(maxTextWidth, bounds.width());

        ShapedHudLine shapedLine = {};
        shapedLine.blob = blob;
        shapedLine.color = line.color;
        shapedLine.topY = contentTop;
        shapedLines.push_back(std::move(shapedLine));

        contentTop += lineSpacing;
    }

    if (shapedLines.empty()) {
        return;
    }

    const float panelWidth = maxTextWidth + padding * 2.0f;
    const float panelHeight = padding * 2.0f + lineSpacing * static_cast<float>(shapedLines.size() - 1) + fontSize * 1.15f;
    tgfx::Path panelPath = {};
    panelPath.addRoundRect(tgfx::Rect::MakeXYWH(originX, originY, panelWidth, panelHeight), cornerRadius, cornerRadius);

    tgfx::Paint panelPaint = {};
    panelPaint.setColor(tgfx::Color{0.0f, 0.0f, 0.0f, 0.55f});
    canvas->drawPath(panelPath, panelPaint);

    const float textX = originX + padding;
    tgfx::Paint textPaint = {};
    for (auto &shapedLine : shapedLines) {
        if (!shapedLine.blob) {
            continue;
        }
        auto bounds = shapedLine.blob->getTightBounds();
        textPaint.setColor(shapedLine.color);
        canvas->drawTextBlob(shapedLine.blob, textX, shapedLine.topY - bounds.y(), textPaint);
    }
}

tgfx::Rect SeatCanvasCoreRenderer::getVisibleOriginalRect() const {
    return _state->getVisibleOriginalRect();
}

std::vector<std::string> SeatCanvasCoreRenderer::getZoneIdsInOriginalRect(const tgfx::Rect &rect) const {
    std::vector<std::string> zoneIds = {};

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        return zoneIds;
    }

    auto meshBuilder = baseMapConfig->meshBuilder();
    if (!meshBuilder) {
        return zoneIds;
    }

    auto zones = meshBuilder->findZoneIntersectingRect(rect);
    for (const auto &zone : zones) {
        if (!zone || zone->zoneId.empty()) {
            continue;
        }
        zoneIds.push_back(zone->zoneId);
    }

    return zoneIds;
}

tgfx::Point SeatCanvasCoreRenderer::convertScreenToContent(const tgfx::Point &location, const tgfx::Point &contentOffset, float scale) const {
    return ConvertScreenToContent(location, contentOffset, scale);
}

tgfx::Point SeatCanvasCoreRenderer::convertContentToScreen(const tgfx::Point &location, const tgfx::Point &contentOffset, float scale) const {
    return ConvertContentToScreen(location, contentOffset, scale);
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
    _viewportController->zoomToRect(rect, animated, padding, durationMs);
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
        _renderedSeatZoneCount = 0;
        _renderedSeatCount = 0;
        _customSeatPass->clearSeats();
    } else {
        setBaseMapLayer(config->baseMapSize());
        _seatZoneNameLayer->setTextRootLayer(config->textLayer(), config->baseMapSize());
        setMiniMapLayer(config->minimapLayer());
        _useBaseMapConfig = config;

        applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        applySavedSeatZoneAlternateColors(config->meshBuilder());
        applySavedMiniMapZoneAlternateColors(config->meshBuilder());
        _customBaseMapPass->updateMeshBuilder(config->meshBuilder());

        _renderedSeatZoneCount = 0;
        _renderedSeatCount = 0;
        _customSeatPass->clearSeats();
    }

    handleBaseMapChanged();
}

void SeatCanvasCoreRenderer::handleBaseMapChanged() {
    _viewportController->updateContentScale(_viewportController->getMaxWidth());
    syncBoundsFromPlatformView();
    _viewportController->updateContentSize();

    if (!_delegate) {
        return;
    }

    auto config = _useBaseMapConfig.lock();
    if (!config) {
        _delegate->didUnloadBaseMap(_coreID);
        return;
    }

    _delegate->didLoadBaseMap(_coreID);
    dispatchZoomLevelConfigUpdate();
}

bool SeatCanvasCoreRenderer::syncBoundsFromPlatformView() {
    if (_platformView == nullptr) {
        return false;
    }

    auto size = _platformView->getSize();
    auto density = _platformView->getDensity();
    if (size.width <= 0 || size.height <= 0 || density < 1.0f) {
        return false;
    }

    _state->updateScreen(size.width, size.height, density);
    _zoomPanController->setBounds(tgfx::Size::Make(size));
    return true;
}

void SeatCanvasCoreRenderer::setBaseMapLayer(const tgfx::Size &baseMapSize) {
    _state->updateOriginSize(baseMapSize);
}

void SeatCanvasCoreRenderer::setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer) {
    // minimap 使用与 baseMap 相同的尺寸
    tgfx::Size baseMapSize = _state->getOriginSize();
    _overlayLayer->setBaseMapLayer(std::move(layer), baseMapSize);
}

void SeatCanvasCoreRenderer::updateZoomPanControllerState(bool notifyViewport) {
    auto previousZoom = _state->getZoomScale();
    auto previousOffset = _state->getContentOffset();
    auto currentZoom = _zoomPanController->getZoomScale();
    auto currentOffset = _zoomPanController->getContentOffset();
    auto changed = _state->updateZoomAndOffset(currentZoom, currentOffset);
    if (changed) {
        invalidateContent();
    }
    if (!changed || !notifyViewport || !_delegate) {
        return;
    }

    auto event = makeViewportEvent();
    bool offsetChanged = std::abs(previousOffset.x - currentOffset.x) > FLT_EPSILON || std::abs(previousOffset.y - currentOffset.y) > FLT_EPSILON;
    bool zoomChanged = std::abs(previousZoom - currentZoom) > FLT_EPSILON;
    if (offsetChanged) {
        _delegate->viewportDidScroll(_coreID, event);
    }
    if (zoomChanged) {
        _delegate->viewportDidZoom(_coreID, event);
    }
}

SeatCanvasViewportEvent SeatCanvasCoreRenderer::makeViewportEvent() const {
    return MakeViewportEvent(_state.get());
}

void SeatCanvasCoreRenderer::dispatchZoomLevelConfigUpdate() {
    if (_delegate == nullptr) {
        return;
    }
    SeatCanvasZoomLevelConfigEvent event = {};
    event.zoomLevels = _zoomLevelConfig;
    event.minimumZoomScale = getMinimumZoomScale();
    event.maximumZoomScale = getMaximumZoomScale();
    event.zoomScale = getZoomScale();
    _delegate->didUpdateZoomLevelConfig(_coreID, event);
}

bool SeatCanvasCoreRenderer::shouldAutoDrawSeat() const {
    if (_viewportController->isAutoDrawSeatDisabled()) {
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

// ---- ViewportControllerCallback 实现 ----

void SeatCanvasCoreRenderer::onInvalidateContent() {
    invalidateContent();
}

void SeatCanvasCoreRenderer::onApplyBaseMapColorState(kk::BaseMapColorState state) {
    applyBaseMapColorState(state);
}

void SeatCanvasCoreRenderer::onHideMinimapWithoutAnimation() {
    hideMinimapWithoutAnimation();
}

void SeatCanvasCoreRenderer::onSetOverlayBackVisible(bool visible) {
    if (_overlayLayer) {
        _overlayLayer->setBackVisible(visible);
    }
}

void SeatCanvasCoreRenderer::onSetOverlayBackAlpha(float alpha) {
    if (_overlayLayer) {
        _overlayLayer->setBackAlpha(alpha);
    }
}

bool SeatCanvasCoreRenderer::onIsOverlayBackVisible() const {
    return _overlayLayer && _overlayLayer->backVisible();
}

void SeatCanvasCoreRenderer::onUpdateZoomPanControllerState(bool notify) {
    updateZoomPanControllerState(notify);
}

void SeatCanvasCoreRenderer::prepareSeatIfNeeded() {
    if (!_state) {
        return;
    }

    auto clearSeatsAndStats = [this]() {
        _renderedSeatZoneCount = 0;
        _renderedSeatCount = 0;
        _customSeatPass->clearSeats();
    };

    auto baseMapConfig = _useBaseMapConfig.lock();
    if (!baseMapConfig) {
        clearSeatsAndStats();
        return;
    }

    auto meshBuilder = baseMapConfig->meshBuilder();
    if (meshBuilder == nullptr) {
        clearSeatsAndStats();
        return;
    }

    auto zoomScale = _state->getZoomScale();

    if (zoomScale < getSeatRenderZoomThreshold()) {
        clearSeatsAndStats();
        if (_viewportController->autoChangeBaseMapColorState) {
            applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        }
        return;
    }

    if (_viewportController->autoChangeBaseMapColorState) {
        applyBaseMapColorState(kk::BaseMapColorState::Original);
    }

    if (!shouldAutoDrawSeat()) {
        clearSeatsAndStats();
        return;
    }

    auto visibleOriginalRect = getVisibleOriginalRect();
    if (visibleOriginalRect.isEmpty()) {
        clearSeatsAndStats();
        return;
    }

    /*
     * 扩大一点点，避免出现刚好在边缘的隐藏/显示，视觉上体验不好
     */
    visibleOriginalRect.outset(_seatSize * 2.0, _seatSize * 2.0);

    auto result = _seatDataManager->collectVisibleSeatInstances(visibleOriginalRect, meshBuilder.get(), _seatSize);
    _renderedSeatZoneCount = result.zoneCount;
    _renderedSeatCount = result.instances.size();
    _customSeatPass->updateSeats(std::move(result.instances));
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

    auto seatData = _seatDataManager->getSeatDataForZone(zoneInfo->zoneId);
    if (!seatData) {
        return;
    }

    for (const auto &seatInfo : *seatData) {
        auto rect = tgfx::Rect::MakeXYWH(seatInfo.x, seatInfo.y, _seatSize, _seatSize);
        if (!rect.contains(originalLocation.x, originalLocation.y)) {
            continue;
        }

        auto changed = _delegate->didTapSeat(_coreID, zoneInfo->zoneId, seatInfo.seatId);
        if (changed) {
            _customSeatPass->clearSeats();
            invalidateContent();
            showMinimapWithoutAnimation();
        }
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
        _viewportController->scrollViewWithLocation(location, getSeatRenderZoomThreshold());
        return;
    }

    // 如果已经缩放到座位级别（seatRenderZoomThreshold），则执行点击位置的渐进式缩放
    auto zoomScale = _zoomPanController->getZoomScale();
    if (zoomScale >= getSeatRenderZoomThreshold() || isSmallVenue()) {
        _viewportController->scrollViewWithLocation(location, getSeatRenderZoomThreshold());
        return;
    }

    // 使用找到的区域进行缩放
    if (!zoneInfo->zoneId.empty()) {
        if (_delegate) {
            _delegate->didTapZone(_coreID, zoneInfo->zoneId);
        }
        _viewportController->scrollViewWithZone(zoneInfo, getSeatRenderZoomThreshold());
        return;
    }

    _viewportController->scrollViewWithLocation(location, getSeatRenderZoomThreshold());
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

void SeatCanvasCoreRenderer::updateMiniMapZoneAlternateColors(const std::unordered_map<std::string, tgfx::Color> &colors) {
    if (_minimapZoneColorMap != colors) {
        _minimapZoneColorMap = colors;
    }

    auto config = _useBaseMapConfig.lock();
    if (!config) {
        return;
    }

    const auto &miniLayerMap = config->miniLayerMap();
    if (miniLayerMap.empty()) {
        return;
    }

    for (const auto &[zoneId, layer] : miniLayerMap) {
        if (static_cast<kk::layer::CustomLayerType>(layer->type()) != kk::layer::CustomLayerType::Zone) {
            continue;
        }

        auto zoneLayer = std::static_pointer_cast<kk::layer::SeatZoneLayer>(layer);
        if (!zoneLayer) {
            continue;
        }

        if (colors.empty()) {
            zoneLayer->setAlternateFillColor(std::nullopt);
            continue;
        }

        const auto iter = colors.find(zoneId);
        if (iter == colors.end() || iter->second == tgfx::Color::Transparent()) {
            zoneLayer->setAlternateFillColor(std::nullopt);
        } else {
            zoneLayer->setAlternateFillColor(iter->second);
        }
    }

    if (_overlayLayer) {
        _overlayLayer->invalidateAreaCacheImage();
    }
    invalidateContent();
}

void SeatCanvasCoreRenderer::applySavedSeatZoneAlternateColors(std::shared_ptr<BaseMapMeshBuilder> meshBuilder) {
    if (!meshBuilder) {
        return;
    }

    updateSeatZoneAlternateColors(_zoneColorMap);
}

void SeatCanvasCoreRenderer::applySavedMiniMapZoneAlternateColors(std::shared_ptr<BaseMapMeshBuilder> meshBuilder) {
    if (!meshBuilder) {
        return;
    }
    updateMiniMapZoneAlternateColors(_minimapZoneColorMap);
}

void SeatCanvasCoreRenderer::setSeatData(const std::string &zoneId, const std::vector<kk::SeatData> &seats) {
    if (_seatDataManager->setSeatData(zoneId, seats)) {
        _customSeatPass->clearSeats();
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::clearSeatData() {
    if (_seatDataManager->clearSeatData()) {
        _customSeatPass->clearSeats();
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::registerPricecodes(const std::vector<std::string> &pricecodes) {
    _seatDataManager->registerPricecodes(pricecodes);
}

uint16_t SeatCanvasCoreRenderer::pricecodeIndexForCode(const std::string &pricecode) const {
    return _seatDataManager->pricecodeIndexForCode(pricecode);
}

void SeatCanvasCoreRenderer::updateSeatStatuses(const std::vector<kk::SeatStatusUpdate> &updates) {
    if (_seatDataManager->updateSeatStatuses(updates)) {
        _customSeatPass->clearSeats();
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::updateSeatStatusesForZone(const std::string &zoneId, const uint32_t *statuses, size_t count) {
    if (_seatDataManager->updateSeatStatusesForZone(zoneId, statuses, count)) {
        _customSeatPass->clearSeats();
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::setSelectedSeatIds(const std::vector<std::string> &seatIds) {
    _seatDataManager->setSelectedSeatIds(seatIds);
    _customSeatPass->clearSeats();
    invalidateContent();
}

void SeatCanvasCoreRenderer::updateSelectedSeatIds(const std::vector<std::string> &added, const std::vector<std::string> &removed) {
    if (_seatDataManager->updateSelectedSeatIds(added, removed)) {
        _customSeatPass->clearSeats();
        invalidateContent();
    }
}
};  // namespace kk::renderer
