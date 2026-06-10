//
//  WebRendererCore.cpp
//  SeatCanvas
//
//  Created by KK on 2026/5/26.
//

#include "WebRendererCore.hpp"

#include <emscripten/bind.h>

#include <string>
#include <unordered_map>

#include <tgfx/core/Color.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/layers/TextLayer.h>

#include "WebPlatformView.hpp"
#include "WebSeatCanvasCoreRendererDelegate.hpp"

#include "core/BaseMapConfig.hpp"
#include "core/Font.h"
#include "core/FontManager.hpp"
#include "core/SeatData.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/parser/BaseMapLoadResult.hpp"
#include "core/parser/BaseMapParserFactory.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/style/ColorHexParser.hpp"
#include "core/style/SeatStyleConfig.hpp"
#include "core/utils/SystemProperties.hpp"

#include "core/Log.hpp"
#include <emscripten/val.h>

namespace kk::web {

namespace {

std::shared_ptr<tgfx::Data> getDataFromEmscripten(const emscripten::val &emscriptenData) {
    if (emscriptenData.isUndefined() || emscriptenData.isNull()) {
        return nullptr;
    }
    if (!emscriptenData.instanceof (emscripten::val::global("Uint8Array"))) {
        SC_LOG_ERROR("RegisterFonts: font data must be a Uint8Array");
        return nullptr;
    }
    auto length = emscriptenData["length"].as<unsigned int>();
    if (length == 0) {
        return nullptr;
    }
    auto *buffer = new (std::nothrow) uint8_t[length];
    if (buffer == nullptr) {
        SC_LOG_ERROR("RegisterFonts: failed to allocate font buffer (%u bytes)", length);
        return nullptr;
    }
    auto memory = emscripten::val::module_property("HEAPU8")["buffer"];
    auto memoryView =
        emscriptenData["constructor"].new_(memory, reinterpret_cast<uintptr_t>(buffer), length);
    memoryView.call<void>("set", emscriptenData);
    return tgfx::Data::MakeAdopted(buffer, length, tgfx::Data::DeleteProc);
}

void updateTextLayerFallbackTypefaces() {
    auto fallbackTypefaces = kk::FontManager::GetFallbackTypefacesDirect();
    tgfx::TextLayer::SetFallbackTypefaces(std::move(fallbackTypefaces));
}

}  // namespace

// MARK: - Lifecycle

std::shared_ptr<WebRendererCore> WebRendererCore::MakeFrom(const std::string &canvasID) {
    return std::shared_ptr<WebRendererCore>(new WebRendererCore(canvasID));
}

bool WebRendererCore::RegisterFonts(const emscripten::val &fontData, const emscripten::val &emojiFontData) {
    std::vector<std::shared_ptr<tgfx::Typeface>> typefaces = {};

    auto textData = getDataFromEmscripten(fontData);
    if (textData == nullptr) {
        SC_LOG_ERROR("RegisterFonts: text font data is missing or invalid");
        return false;
    }
    auto textTypeface = tgfx::Typeface::MakeFromData(textData, 0);
    if (textTypeface == nullptr) {
        SC_LOG_ERROR("RegisterFonts: failed to parse text font data");
        return false;
    }
    typefaces.push_back(std::move(textTypeface));

    if (!emojiFontData.isUndefined() && !emojiFontData.isNull()) {
        if (auto emojiData = getDataFromEmscripten(emojiFontData)) {
            if (auto emojiTypeface = tgfx::Typeface::MakeFromData(emojiData, 0)) {
                typefaces.push_back(std::move(emojiTypeface));
            } else {
                SC_LOG_ERROR("RegisterFonts: failed to parse emoji font data, continuing without emoji");
            }
        } else {
            SC_LOG_ERROR("RegisterFonts: emoji font data is invalid, continuing without emoji");
        }
    }

    kk::FontManager::SetFallbackFontPaths({}, {});
    kk::FontManager::PrependFallbackTypefaces(typefaces);
    updateTextLayerFallbackTypefaces();
    return true;
}

WebRendererCore::WebRendererCore(const std::string &canvasID) {
    syncSystemProperties();
    auto platformView = std::make_unique<kk::renderer::WebPlatformView>(canvasID);
    auto zoomPanController = std::make_unique<kk::gesture::ElasticZoomPanController>();

    _delegate = std::make_shared<WebSeatCanvasCoreRendererDelegate>();
    _renderer = std::make_unique<kk::renderer::SeatCanvasCoreRenderer>(
        std::move(platformView),
        std::move(zoomPanController));
    _renderer->setDelegate(_delegate);
}

WebRendererCore::~WebRendererCore() {
    if (_renderer) {
        _renderer->stop();
    }
}

void WebRendererCore::start() {
    _renderer->start();
}

void WebRendererCore::stop() {
    _renderer->stop();
}

void WebRendererCore::draw(bool force) {
    _renderer->draw(force);
}

// MARK: - Basemap

bool WebRendererCore::loadBaseMapFromSVG(const std::string &svgData, const std::string &parseConfigJSON) {
    if (svgData.empty()) {
        SC_LOG_ERROR("loadBaseMapFromSVG: empty SVG data");
        return false;
    }
    auto data = tgfx::Data::MakeWithCopy(svgData.data(), svgData.size());
    std::shared_ptr<tgfx::Data> configData = nullptr;
    if (!parseConfigJSON.empty()) {
        configData = tgfx::Data::MakeWithCopy(parseConfigJSON.data(), parseConfigJSON.size());
    }
    auto result = kk::parser::BaseMapParserFactory::parse(data, kk::parser::BaseMapFormat::SVG, configData);
    if (result == nullptr) {
        SC_LOG_ERROR("loadBaseMapFromSVG: failed to parse SVG");
        return false;
    }
    kk::parser::BaseMapLoadResult loadResult(std::move(result));
    auto baseMapConfig = loadResult.makeBaseMapConfig();
    _renderer->setBaseMapConfig(std::move(baseMapConfig));
    return true;
}

// MARK: - Gestures

void WebRendererCore::handleTap(float x, float y) {
    _renderer->handleTap(tgfx::Point::Make(x, y));
}

void WebRendererCore::handlePan(int state, float translationX, float translationY, double timestampMs) {
    _renderer->handlePan(static_cast<kk::gesture::GestureState>(state),
                         tgfx::Point::Make(translationX, translationY), timestampMs);
}

void WebRendererCore::handlePinch(int state, float scale, float centerX, float centerY) {
    _renderer->handlePinch(static_cast<kk::gesture::GestureState>(state),
                           scale, tgfx::Point::Make(centerX, centerY));
}

// MARK: - Style & color

void WebRendererCore::setBackgroundColor(float r, float g, float b, float a) {
    _renderer->setBackgroundColor(tgfx::Color{r, g, b, a});
}

emscripten::val WebRendererCore::getBackgroundColor() const {
    auto c = _renderer->getBackgroundColor();
    auto obj = emscripten::val::object();
    obj.set("r", c.red);
    obj.set("g", c.green);
    obj.set("b", c.blue);
    obj.set("a", c.alpha);
    return obj;
}

void WebRendererCore::setSeatSize(float size) {
    _renderer->setSeatSize(size);
}

float WebRendererCore::getSeatSize() const {
    return _renderer->getSeatSize();
}

void WebRendererCore::setDebugHUDEnabled(bool enabled) {
    _renderer->setDebugHUDEnabled(enabled);
}

bool WebRendererCore::isDebugHUDEnabled() const {
    return _renderer->isDebugHUDEnabled();
}

// MARK: - Zoom & viewport queries

float WebRendererCore::getZoomScale() const {
    return _renderer->getZoomScale();
}

float WebRendererCore::getMinimumZoomScale() const {
    return _renderer->getMinimumZoomScale();
}

float WebRendererCore::getMaximumZoomScale() const {
    return _renderer->getMaximumZoomScale();
}

emscripten::val WebRendererCore::getContentOffset() const {
    auto offset = _renderer->getContentOffset();
    auto obj = emscripten::val::object();
    obj.set("x", offset.x);
    obj.set("y", offset.y);
    return obj;
}

emscripten::val WebRendererCore::getVisibleOriginalRect() const {
    auto rect = _renderer->getVisibleOriginalRect();
    auto obj = emscripten::val::object();
    obj.set("x", rect.x());
    obj.set("y", rect.y());
    obj.set("width", rect.width());
    obj.set("height", rect.height());
    return obj;
}

float WebRendererCore::getFPS() const {
    return _renderer->getFPS();
}

void WebRendererCore::setSeatRenderZoomThreshold(float threshold) {
    _renderer->setSeatRenderZoomThreshold(threshold);
}

float WebRendererCore::getSeatRenderZoomThreshold() const {
    return _renderer->getSeatRenderZoomThreshold();
}

void WebRendererCore::zoomToRect(float left, float top, float right, float bottom,
                                 bool animated, float padding, double durationMs) {
    _renderer->zoomToRect(tgfx::Rect::MakeLTRB(left, top, right, bottom), animated, padding, durationMs);
}

// MARK: - Seat data

void WebRendererCore::registerPricecodes(const emscripten::val &pricecodes) {
    auto length = pricecodes["length"].as<size_t>();
    std::vector<std::string> codes;
    codes.reserve(length);
    for (size_t i = 0; i < length; i++) {
        codes.push_back(pricecodes[i].as<std::string>());
    }
    _renderer->registerPricecodes(codes);
}

void WebRendererCore::setSeatData(const std::string &zoneId, const emscripten::val &seats) {
    auto length = seats["length"].as<size_t>();
    std::vector<kk::SeatData> seatList;
    seatList.reserve(length);
    for (size_t i = 0; i < length; i++) {
        auto item = seats[i];
        kk::SeatData seat;
        seat.seatId = item["seatId"].as<std::string>();
        seat.x = item["x"].as<float>();
        seat.y = item["y"].as<float>();
        if (item.hasOwnProperty("rotation")) {
            seat.rotation = item["rotation"].as<float>();
        }
        if (item.hasOwnProperty("pricecode")) {
            auto pricecode = item["pricecode"].as<std::string>();
            seat.pricecodeIndex = _renderer->pricecodeIndexForCode(pricecode);
        }
        seatList.push_back(std::move(seat));
    }
    _renderer->setSeatData(zoneId, seatList);
}

void WebRendererCore::clearSeatData() {
    _renderer->clearSeatData();
}

void WebRendererCore::updateSeatStatuses(const emscripten::val &updates) {
    auto length = updates["length"].as<size_t>();
    std::vector<kk::SeatStatusUpdate> updateList;
    updateList.reserve(length);
    for (size_t i = 0; i < length; i++) {
        auto item = updates[i];
        kk::SeatStatusUpdate update;
        update.seatId = item["seatId"].as<std::string>();
        update.status = item["status"].as<uint32_t>();
        updateList.push_back(update);
    }
    _renderer->updateSeatStatuses(updateList);
}

void WebRendererCore::updateSeatStatusesForZone(const std::string &zoneId, const emscripten::val &statuses) {
    auto length = statuses["length"].as<size_t>();
    std::vector<uint32_t> statusList;
    statusList.reserve(length);
    for (size_t i = 0; i < length; i++) {
        statusList.push_back(statuses[i].as<uint32_t>());
    }
    _renderer->updateSeatStatusesForZone(zoneId, statusList.data(), statusList.size());
}

void WebRendererCore::setSelectedSeatIds(const emscripten::val &seatIds) {
    auto length = seatIds["length"].as<size_t>();
    std::vector<std::string> ids;
    ids.reserve(length);
    for (size_t i = 0; i < length; i++) {
        ids.push_back(seatIds[i].as<std::string>());
    }
    _renderer->setSelectedSeatIds(ids);
}

void WebRendererCore::updateSelectedSeatIds(const emscripten::val &added, const emscripten::val &removed) {
    auto addedLen = added["length"].as<size_t>();
    std::vector<std::string> addedList;
    addedList.reserve(addedLen);
    for (size_t i = 0; i < addedLen; i++) {
        addedList.push_back(added[i].as<std::string>());
    }

    auto removedLen = removed["length"].as<size_t>();
    std::vector<std::string> removedList;
    removedList.reserve(removedLen);
    for (size_t i = 0; i < removedLen; i++) {
        removedList.push_back(removed[i].as<std::string>());
    }

    _renderer->updateSelectedSeatIds(addedList, removedList);
}

void WebRendererCore::applySeatStyleJSONConfig(const std::string &jsonString) {
    _renderer->setStyleKeyToConfigFromJSON(jsonString.data(), jsonString.size());
}

void WebRendererCore::updateSeatZoneAlternateColors(const emscripten::val &colors) {
    std::unordered_map<std::string, tgfx::Color> cppColors;
    auto keys = emscripten::val::global("Object").call<emscripten::val>("keys", colors);
    auto length = keys["length"].as<size_t>();
    for (size_t i = 0; i < length; i++) {
        auto key = keys[i].as<std::string>();
        auto colorStr = colors[key].as<std::string>();
        tgfx::Color color{};
        if (!kk::renderer::ParseColorFromARGBHex(colorStr, color)) {
            SC_LOG_ERROR("updateSeatZoneAlternateColors: invalid color '%s'", colorStr.c_str());
            continue;
        }
        cppColors.emplace(key, color);
    }
    _renderer->updateSeatZoneAlternateColors(cppColors);
}

void WebRendererCore::updateMiniMapZoneAlternateColors(const emscripten::val &colors) {
    std::unordered_map<std::string, tgfx::Color> cppColors;
    auto keys = emscripten::val::global("Object").call<emscripten::val>("keys", colors);
    auto length = keys["length"].as<size_t>();
    for (size_t i = 0; i < length; i++) {
        auto key = keys[i].as<std::string>();
        auto colorStr = colors[key].as<std::string>();
        tgfx::Color color{};
        if (!kk::renderer::ParseColorFromARGBHex(colorStr, color)) {
            SC_LOG_ERROR("updateMiniMapZoneAlternateColors: invalid color '%s'", colorStr.c_str());
            continue;
        }
        cppColors.emplace(key, color);
    }
    _renderer->updateMiniMapZoneAlternateColors(cppColors);
}

bool WebRendererCore::updateSize() {
    syncSystemProperties();
    return _renderer->updateSize();
}

void WebRendererCore::syncSystemProperties() {
    float density = emscripten::val::global("window")["devicePixelRatio"].as<float>();
    if (density <= 0.0f) {
        density = 1.0f;
    }
    auto &properties = kk::utils::SystemProperties::Instance();
    properties.updateDensity(density);
    properties.updateFontScale(density);
}

void WebRendererCore::invalidateContent() {
    _renderer->invalidateContent();
}

};  // namespace kk::web

// MARK: - Emscripten bindings

using namespace kk::web;

EMSCRIPTEN_BINDINGS(SeatCanvasWeb) {
    emscripten::class_<WebRendererCore>("SeatCanvasRenderer")
        .smart_ptr<std::shared_ptr<WebRendererCore>>("SeatCanvasRenderer")
        .class_function("MakeFrom", &WebRendererCore::MakeFrom)
        .class_function("RegisterFonts", &WebRendererCore::RegisterFonts)
        .function("getDelegate", &WebRendererCore::getDelegate)
        .function("start", &WebRendererCore::start)
        .function("stop", &WebRendererCore::stop)
        .function("draw", &WebRendererCore::draw)
        .function("loadBaseMapFromSVG", &WebRendererCore::loadBaseMapFromSVG)
        .function("handleTap", &WebRendererCore::handleTap)
        .function("handlePan", &WebRendererCore::handlePan)
        .function("handlePinch", &WebRendererCore::handlePinch)
        .function("setBackgroundColor", &WebRendererCore::setBackgroundColor)
        .function("getBackgroundColor", &WebRendererCore::getBackgroundColor)
        .function("setSeatSize", &WebRendererCore::setSeatSize)
        .function("getSeatSize", &WebRendererCore::getSeatSize)
        .function("setDebugHUDEnabled", &WebRendererCore::setDebugHUDEnabled)
        .function("isDebugHUDEnabled", &WebRendererCore::isDebugHUDEnabled)
        .function("getZoomScale", &WebRendererCore::getZoomScale)
        .function("getMinimumZoomScale", &WebRendererCore::getMinimumZoomScale)
        .function("getMaximumZoomScale", &WebRendererCore::getMaximumZoomScale)
        .function("getContentOffset", &WebRendererCore::getContentOffset)
        .function("getVisibleOriginalRect", &WebRendererCore::getVisibleOriginalRect)
        .function("getFPS", &WebRendererCore::getFPS)
        .function("setSeatRenderZoomThreshold", &WebRendererCore::setSeatRenderZoomThreshold)
        .function("getSeatRenderZoomThreshold", &WebRendererCore::getSeatRenderZoomThreshold)
        .function("zoomToRect", &WebRendererCore::zoomToRect)
        .function("registerPricecodes", &WebRendererCore::registerPricecodes)
        .function("setSeatData", &WebRendererCore::setSeatData)
        .function("clearSeatData", &WebRendererCore::clearSeatData)
        .function("updateSeatStatuses", &WebRendererCore::updateSeatStatuses)
        .function("updateSeatStatusesForZone", &WebRendererCore::updateSeatStatusesForZone)
        .function("setSelectedSeatIds", &WebRendererCore::setSelectedSeatIds)
        .function("updateSelectedSeatIds", &WebRendererCore::updateSelectedSeatIds)
        .function("applySeatStyleJSONConfig", &WebRendererCore::applySeatStyleJSONConfig)
        .function("updateSeatZoneAlternateColors", &WebRendererCore::updateSeatZoneAlternateColors)
        .function("updateMiniMapZoneAlternateColors", &WebRendererCore::updateMiniMapZoneAlternateColors)
        .function("updateSize", &WebRendererCore::updateSize)
        .function("invalidateContent", &WebRendererCore::invalidateContent);

    emscripten::class_<WebSeatCanvasCoreRendererDelegate>("SeatCanvasRendererDelegate")
        .smart_ptr<std::shared_ptr<WebSeatCanvasCoreRendererDelegate>>("SeatCanvasRendererDelegate")
        .function("setDidLoadBaseMapCallback", &WebSeatCanvasCoreRendererDelegate::setDidLoadBaseMapCallback)
        .function("setDidUnloadBaseMapCallback", &WebSeatCanvasCoreRendererDelegate::setDidUnloadBaseMapCallback)
        .function("setDidUpdateZoomLevelConfigCallback", &WebSeatCanvasCoreRendererDelegate::setDidUpdateZoomLevelConfigCallback)
        .function("setDidTapZoneCallback", &WebSeatCanvasCoreRendererDelegate::setDidTapZoneCallback)
        .function("setDidTapSeatCallback", &WebSeatCanvasCoreRendererDelegate::setDidTapSeatCallback)
        .function("setViewportWillBeginDraggingCallback", &WebSeatCanvasCoreRendererDelegate::setViewportWillBeginDraggingCallback)
        .function("setViewportDidScrollCallback", &WebSeatCanvasCoreRendererDelegate::setViewportDidScrollCallback)
        .function("setViewportDidEndDraggingCallback", &WebSeatCanvasCoreRendererDelegate::setViewportDidEndDraggingCallback)
        .function("setViewportDidEndDeceleratingCallback", &WebSeatCanvasCoreRendererDelegate::setViewportDidEndDeceleratingCallback)
        .function("setViewportWillBeginZoomingCallback", &WebSeatCanvasCoreRendererDelegate::setViewportWillBeginZoomingCallback)
        .function("setViewportDidZoomCallback", &WebSeatCanvasCoreRendererDelegate::setViewportDidZoomCallback)
        .function("setViewportDidEndZoomingCallback", &WebSeatCanvasCoreRendererDelegate::setViewportDidEndZoomingCallback)
        .function("setViewportDidEndScrollingAnimationCallback", &WebSeatCanvasCoreRendererDelegate::setViewportDidEndScrollingAnimationCallback);
};

int main(int, const char *[]) {
    return 0;
}