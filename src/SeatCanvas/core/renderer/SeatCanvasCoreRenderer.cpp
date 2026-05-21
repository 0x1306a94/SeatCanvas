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
#include <mutex>
#include <random>
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
    // atlas 生成完成后同步 UV 偏移，并重建样式键查表
    _seatAtlasManager->setOnAtlasGenerated([this](const SeatStyleAtlasManager *atlasManager) {
        _customSeatPass->updateUVOffset(atlasManager->getUVOffsets());
        rebuildStyleKeyLookup();
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
    if (_pendingDidLoadBaseMap) {
        dispatchBaseMapLifecycleCallback();
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

void SeatCanvasCoreRenderer::setStyleIdToConfig(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig) {
    _registeredStyleIdToConfig = styleIdToConfig;
    auto changed = _seatAtlasManager->setStyleIdToConfigs(styleIdToConfig);
    if (changed) {
        // 样式变更会 invalidate atlas，查表需等 atlas 重新生成后再 rebuild
        _uvIndexByStyleKey.clear();
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::setStyleKeyToConfigFromJSON(const void *bytes, size_t len) {
    if (!bytes || len == 0) {
        setStyleIdToConfig({});
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

    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> styleIdToConfig = {};

    for (const auto &entry : json) {
        if (!entry.contains("key") || !entry.contains("config")) {
            tgfx::PrintError("Invalid JSON entry: missing 'key' or 'config'");
            continue;
        }

        if (!entry["key"].is_string()) {
            tgfx::PrintError("Invalid JSON entry: 'key' is not a string");
            continue;
        }

        auto styleId = entry["key"].get<std::string>();
        if (styleId.empty()) {
            tgfx::PrintError("Invalid JSON entry: styleId is empty");
            continue;
        }

        if (!entry["config"].is_object()) {
            tgfx::PrintError("Invalid JSON entry: 'config' is not an object");
            continue;
        }

        std::shared_ptr<SeatStyleConfig> config = nullptr;
        entry["config"].get_to(config);
        if (!config) {
            tgfx::PrintError("Failed to parse config");
            continue;
        }

        styleIdToConfig[styleId] = config;
    }

    setStyleIdToConfig(styleIdToConfig);
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
        _panAnimationActive = true;
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
                    notifyViewportDidEndDeceleratingIfNeeded();
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
    _panAnimationActive = false;
    _scrollingAnimationActive = false;
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
    drawDebugHUD(canvas);
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
    if (canvas == nullptr || !_textShaper || _frameMetrics->isFirstFrame()) {
        return;
    }

    struct HudLine {
        std::string text = {};
        tgfx::Color color = {tgfx::Color::White()};
    };

    size_t zoneCount = _seatDataMap.size();
    size_t seatCount = 0;
    for (const auto &entry : _seatDataMap) {
        seatCount += entry.second.size();
    }

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

    updateZoomPanControllerState(false);

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
    updateZoomPanControllerState(false);

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
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
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
    updateContentScale();
    syncBoundsFromPlatformView();
    updateContentSize();
    dispatchBaseMapLifecycleCallback();
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

void SeatCanvasCoreRenderer::dispatchBaseMapLifecycleCallback() {
    if (!_delegate) {
        return;
    }

    auto config = _useBaseMapConfig.lock();
    if (!config) {
        _pendingDidLoadBaseMap = false;
        _delegate->didUnloadBaseMap(_coreID);
        return;
    }

    if (!isBoundsReadyForBaseMapCallback()) {
        _pendingDidLoadBaseMap = true;
        return;
    }

    syncBoundsFromPlatformView();
    updateContentSize();
    _pendingDidLoadBaseMap = false;
    _delegate->didLoadBaseMap(_coreID, makeBaseMapLoadedEvent());
}

bool SeatCanvasCoreRenderer::isBoundsReadyForBaseMapCallback() const {
    if (_platformView == nullptr) {
        return false;
    }

    auto bounds = _state->getBoundsSize();
    return !bounds.isEmpty();
}

void SeatCanvasCoreRenderer::setBaseMapLayer(const tgfx::Size &baseMapSize) {
    _state->updateOriginSize(baseMapSize);
}

void SeatCanvasCoreRenderer::setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer) {
    // minimap 使用与 baseMap 相同的尺寸
    tgfx::Size baseMapSize = _state->getOriginSize();
    _overlayLayer->setBaseMapLayer(std::move(layer), baseMapSize);
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
    SeatCanvasViewportEvent event = {};
    if (_state) {
        event.zoomScale = _state->getZoomScale();
        event.contentOffset = _state->getContentOffset();
        event.visibleOriginalRect = _state->getVisibleOriginalRect();
    }
    return event;
}

SeatCanvasBaseMapLoadedEvent SeatCanvasCoreRenderer::makeBaseMapLoadedEvent() const {
    SeatCanvasBaseMapLoadedEvent event = {};
    if (auto config = _useBaseMapConfig.lock()) {
        event.baseMapSize = config->baseMapSize();
    }
    event.zoomLevels = _zoomLevelConfig;
    event.minimumZoomScale = getMinimumZoomScale();
    event.maximumZoomScale = getMaximumZoomScale();
    event.zoomScale = getZoomScale();
    event.visibleOriginalRect = getVisibleOriginalRect();
    return event;
}

void SeatCanvasCoreRenderer::notifyViewportDidEndDeceleratingIfNeeded() {
    if (!_panAnimationActive || _zoomPanController->hasPendingAnimation()) {
        return;
    }

    _panAnimationActive = false;
    if (_delegate) {
        _delegate->viewportDidEndDecelerating(_coreID, makeViewportEvent());
    }
}

void SeatCanvasCoreRenderer::notifyViewportDidEndScrollingAnimation() {
    if (!_scrollingAnimationActive) {
        return;
    }

    _scrollingAnimationActive = false;
    if (_delegate) {
        _delegate->viewportDidEndScrollingAnimation(_coreID, makeViewportEvent());
    }
}

void SeatCanvasCoreRenderer::beginViewportScrollingAnimation() {
    _scrollingAnimationActive = true;
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

    /*
     * 缩小到一定级别后，不显示座位
     * zoomScale 越小表示缩得越小，seatRenderZoomThreshold 是一个较小的缩放值
     * 所以当 zoomScale < seatRenderZoomThreshold 时，应该隐藏座位
     */
    if (zoomScale < getSeatRenderZoomThreshold()) {
        clearSeatsAndStats();
        if (_autoChangeBaseMapColorState) {
            applyBaseMapColorState(kk::BaseMapColorState::Rainbow);
        }
        return;
    }

    if (_autoChangeBaseMapColorState) {
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

    auto zones = meshBuilder->findZoneIntersectingRect(visibleOriginalRect);
    if (zones.empty()) {
        clearSeatsAndStats();
        return;
    }

    std::vector<SeatInstanceData> instances = {};
    size_t renderedZoneCount = 0;
    for (const auto &zone : zones) {
        if (!zone) {
            continue;
        }

        auto iter = _seatDataMap.find(zone->zoneId);
        if (iter == _seatDataMap.end()) {
            continue;
        }

        auto stateIter = _seatStateByZone.find(zone->zoneId);
        if (stateIter == _seatStateByZone.end()) {
            continue;
        }

        const auto &seats = iter->second;
        const auto &statuses = stateIter->second.statuses;
        auto partial = !visibleOriginalRect.contains(zone->fillBounds);
        const size_t instanceCountBefore = instances.size();
        for (size_t index = 0; index < seats.size(); ++index) {
            const auto &seat = seats[index];
            if (partial && !tgfx::Rect::Intersects(visibleOriginalRect, tgfx::Rect::MakeXYWH(seat.x, seat.y, _seatSize, _seatSize))) {
                continue;
            }

            uint32_t status = index < statuses.size() ? statuses[index] : 0;
            bool selected = _selectedSeatIds.find(seat.seatId) != _selectedSeatIds.end();
            SeatRenderStyleKey styleKey{seat.pricecodeIndex, status, selected};

            auto uvIter = _uvIndexByStyleKey.find(styleKey);
            if (uvIter == _uvIndexByStyleKey.end() || uvIter->second < 0) {
                continue;
            }

            float rotationRad = seat.rotation * (M_PI / 180.0f);
            instances.emplace_back(seat.x, seat.y, uvIter->second, rotationRad);
        }

        if (instances.size() > instanceCountBefore) {
            renderedZoneCount++;
        }
    }

    _renderedSeatZoneCount = renderedZoneCount;
    _renderedSeatCount = instances.size();
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
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
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

    for (const auto &seatInfo : iter->second) {
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
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
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
            notifyViewportDidEndScrollingAnimation();
        }
    };

    beginViewportScrollingAnimation();
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
    if (zoneId.empty()) {
        return;
    }

    if (auto oldIter = _seatDataMap.find(zoneId); oldIter != _seatDataMap.end()) {
        for (const auto &seat : oldIter->second) {
            _seatIndexById.erase(seat.seatId);
            _selectedSeatIds.erase(seat.seatId);
        }
    }

    std::vector<kk::SeatData> validSeats = {};
    validSeats.reserve(seats.size());
    for (const auto &seat : seats) {
        if (!seat.isValid()) {
            continue;
        }
        validSeats.push_back(seat);
    }

    _seatDataMap[zoneId] = std::move(validSeats);

    ZoneSeatRuntimeState runtimeState = {};
    runtimeState.statuses.assign(_seatDataMap[zoneId].size(), 0);
    _seatStateByZone[zoneId] = std::move(runtimeState);

    const auto &storedSeats = _seatDataMap[zoneId];
    for (size_t index = 0; index < storedSeats.size(); ++index) {
        _seatIndexById[storedSeats[index].seatId] = SeatLocation{zoneId, index};
    }

    _customSeatPass->clearSeats();
    invalidateContent();
}

void SeatCanvasCoreRenderer::clearSeatData() {
    _seatDataMap.clear();
    _seatStateByZone.clear();
    _seatIndexById.clear();
    _selectedSeatIds.clear();

    _customSeatPass->clearSeats();

    invalidateContent();
}

void SeatCanvasCoreRenderer::registerPricecodes(const std::vector<std::string> &pricecodes) {
    _pricecodes = pricecodes;
    _pricecodeToIndex.clear();
    _pricecodeToIndex.reserve(pricecodes.size());
    for (size_t index = 0; index < pricecodes.size(); ++index) {
        if (pricecodes[index].empty()) {
            continue;
        }
        _pricecodeToIndex.emplace(pricecodes[index], static_cast<uint16_t>(index));
    }
    // atlas 已就绪时价档表变更需立即重建查表
    if (_seatAtlasManager->getUVOffsetCount() > 0) {
        rebuildStyleKeyLookup();
    }
}

uint16_t SeatCanvasCoreRenderer::pricecodeIndexForCode(const std::string &pricecode) const {
    return resolvePricecodeIndex(_pricecodeToIndex, pricecode);
}

void SeatCanvasCoreRenderer::updateSeatStatuses(const std::vector<kk::SeatStatusUpdate> &updates) {
    if (updates.empty()) {
        return;
    }

    bool changed = false;
    for (const auto &update : updates) {
        if (update.seatId.empty()) {
            continue;
        }
        auto locationIter = _seatIndexById.find(update.seatId);
        if (locationIter == _seatIndexById.end()) {
            continue;
        }
        auto stateIter = _seatStateByZone.find(locationIter->second.zoneId);
        if (stateIter == _seatStateByZone.end()) {
            continue;
        }
        if (locationIter->second.index >= stateIter->second.statuses.size()) {
            continue;
        }
        stateIter->second.statuses[locationIter->second.index] = update.status;
        changed = true;
    }

    if (changed) {
        _customSeatPass->clearSeats();
        invalidateContent();
    }
}

void SeatCanvasCoreRenderer::updateSeatStatusesForZone(const std::string &zoneId, const std::vector<uint32_t> &statuses) {
    if (zoneId.empty()) {
        return;
    }

    auto dataIter = _seatDataMap.find(zoneId);
    auto stateIter = _seatStateByZone.find(zoneId);
    if (dataIter == _seatDataMap.end() || stateIter == _seatStateByZone.end()) {
        return;
    }
    if (statuses.size() != dataIter->second.size()) {
        return;
    }

    stateIter->second.statuses = statuses;
    _customSeatPass->clearSeats();
    invalidateContent();
}

void SeatCanvasCoreRenderer::setSelectedSeatIds(const std::vector<std::string> &seatIds) {
    _selectedSeatIds.clear();
    _selectedSeatIds.insert(seatIds.begin(), seatIds.end());
    _customSeatPass->clearSeats();
    invalidateContent();
}

void SeatCanvasCoreRenderer::updateSelectedSeatIds(const std::vector<std::string> &added, const std::vector<std::string> &removed) {
    if (added.empty() && removed.empty()) {
        return;
    }

    for (const auto &seatId : removed) {
        _selectedSeatIds.erase(seatId);
    }
    for (const auto &seatId : added) {
        if (!seatId.empty()) {
            _selectedSeatIds.insert(seatId);
        }
    }

    _customSeatPass->clearSeats();
    invalidateContent();
}

void SeatCanvasCoreRenderer::rebuildStyleKeyLookup() {
    _uvIndexByStyleKey.clear();
    if (_registeredStyleIdToConfig.empty()) {
        return;
    }

    for (const auto &entry : _registeredStyleIdToConfig) {
        auto parsed = parseSeatStyleId(entry.first);
        if (!parsed.has_value()) {
            continue;
        }

        uint16_t pricecodeIndex = kNoPricecodeIndex;
        if (!parsed->pricecode.empty()) {
            auto pricecodeIter = _pricecodeToIndex.find(parsed->pricecode);
            if (pricecodeIter == _pricecodeToIndex.end()) {
                continue;
            }
            pricecodeIndex = pricecodeIter->second;
        }

        auto uvIndex = _seatAtlasManager->getUVOffsetIndex(entry.first);
        if (uvIndex < 0) {
            continue;
        }

        SeatRenderStyleKey styleKey{pricecodeIndex, parsed->status, parsed->selected};
        _uvIndexByStyleKey[styleKey] = uvIndex;
    }
}
};  // namespace kk::renderer
