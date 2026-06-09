//
//  WebSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

#include "WebSeatCanvasCoreRendererDelegate.hpp"

#include <tgfx/platform/Print.h>

namespace kk::web {

WebSeatCanvasCoreRendererDelegate::WebSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

WebSeatCanvasCoreRendererDelegate::~WebSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

// MARK: - Delegate overrides (lifecycle)

void WebSeatCanvasCoreRendererDelegate::didLoadBaseMap(uint32_t /*coreID*/) {
    if (isFunction(_didLoadBaseMap)) {
        _didLoadBaseMap();
    }
}

void WebSeatCanvasCoreRendererDelegate::didUnloadBaseMap(uint32_t /*coreID*/) {
    if (isFunction(_didUnloadBaseMap)) {
        _didUnloadBaseMap();
    }
}

void WebSeatCanvasCoreRendererDelegate::didUpdateZoomLevelConfig(uint32_t /*coreID*/, const kk::renderer::SeatCanvasZoomLevelConfigEvent &event) {
    if (isFunction(_didUpdateZoomLevelConfig)) {
        auto jsEvent = makeZoomLevelConfigEventValue(event);
        _didUpdateZoomLevelConfig(jsEvent);
    }
}

// MARK: - Delegate overrides (tap)

void WebSeatCanvasCoreRendererDelegate::didTapZone(uint32_t /*coreID*/, const std::string &zoneId) {
    if (isFunction(_didTapZone)) {
        _didTapZone(zoneId);
    }
}

bool WebSeatCanvasCoreRendererDelegate::didTapSeat(uint32_t /*coreID*/, const std::string &zoneId, const std::string &seatId) {
    if (isFunction(_didTapSeat)) {
        auto result = _didTapSeat(zoneId, seatId);
        return result.as<bool>();
    }
    return false;
}

// MARK: - Delegate overrides (viewport)

void WebSeatCanvasCoreRendererDelegate::viewportWillBeginDragging(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportWillBeginDragging, event);
}

void WebSeatCanvasCoreRendererDelegate::viewportDidScroll(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidScroll, event);
}

void WebSeatCanvasCoreRendererDelegate::viewportDidEndDragging(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) {
    callViewportCallback(_viewportDidEndDragging, event, willDecelerate);
}

void WebSeatCanvasCoreRendererDelegate::viewportDidEndDecelerating(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidEndDecelerating, event);
}

void WebSeatCanvasCoreRendererDelegate::viewportWillBeginZooming(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportWillBeginZooming, event);
}

void WebSeatCanvasCoreRendererDelegate::viewportDidZoom(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidZoom, event);
}

void WebSeatCanvasCoreRendererDelegate::viewportDidEndZooming(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidEndZooming, event);
}

void WebSeatCanvasCoreRendererDelegate::viewportDidEndScrollingAnimation(uint32_t /*coreID*/, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidEndScrollingAnimation, event);
}

// MARK: - Callback setters

void WebSeatCanvasCoreRendererDelegate::setDidLoadBaseMapCallback(emscripten::val callback) {
    _didLoadBaseMap = callback;
}

void WebSeatCanvasCoreRendererDelegate::setDidUnloadBaseMapCallback(emscripten::val callback) {
    _didUnloadBaseMap = callback;
}

void WebSeatCanvasCoreRendererDelegate::setDidUpdateZoomLevelConfigCallback(emscripten::val callback) {
    _didUpdateZoomLevelConfig = callback;
}

void WebSeatCanvasCoreRendererDelegate::setDidTapZoneCallback(emscripten::val callback) {
    _didTapZone = callback;
}

void WebSeatCanvasCoreRendererDelegate::setDidTapSeatCallback(emscripten::val callback) {
    _didTapSeat = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportWillBeginDraggingCallback(emscripten::val callback) {
    _viewportWillBeginDragging = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportDidScrollCallback(emscripten::val callback) {
    _viewportDidScroll = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportDidEndDraggingCallback(emscripten::val callback) {
    _viewportDidEndDragging = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportDidEndDeceleratingCallback(emscripten::val callback) {
    _viewportDidEndDecelerating = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportWillBeginZoomingCallback(emscripten::val callback) {
    _viewportWillBeginZooming = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportDidZoomCallback(emscripten::val callback) {
    _viewportDidZoom = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportDidEndZoomingCallback(emscripten::val callback) {
    _viewportDidEndZooming = callback;
}

void WebSeatCanvasCoreRendererDelegate::setViewportDidEndScrollingAnimationCallback(emscripten::val callback) {
    _viewportDidEndScrollingAnimation = callback;
}

// MARK: - Private helpers

bool WebSeatCanvasCoreRendererDelegate::isFunction(const emscripten::val &v) const {
    return !v.isUndefined() && !v.isNull() && v.typeOf().as<std::string>() == "function";
}

void WebSeatCanvasCoreRendererDelegate::callViewportCallback(emscripten::val &ref, const kk::renderer::SeatCanvasViewportEvent &event) {
    if (isFunction(ref)) {
        ref(makeViewportValue(event));
    }
}

void WebSeatCanvasCoreRendererDelegate::callViewportCallback(emscripten::val &ref, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) {
    if (isFunction(ref)) {
        ref(makeViewportValue(event), willDecelerate);
    }
}

emscripten::val WebSeatCanvasCoreRendererDelegate::makeZoomLevelConfigEventValue(const kk::renderer::SeatCanvasZoomLevelConfigEvent &event) {
    auto obj = emscripten::val::object();

    auto zoomLevels = emscripten::val::object();
    zoomLevels.set("seat", event.zoomLevels.seat);
    zoomLevels.set("row", event.zoomLevels.row);
    zoomLevels.set("zone", event.zoomLevels.zone);
    zoomLevels.set("venue", event.zoomLevels.venue);
    obj.set("zoomLevels", zoomLevels);

    obj.set("minimumZoomScale", event.minimumZoomScale);
    obj.set("maximumZoomScale", event.maximumZoomScale);
    obj.set("zoomScale", event.zoomScale);

    return obj;
}

emscripten::val WebSeatCanvasCoreRendererDelegate::makeViewportValue(const kk::renderer::SeatCanvasViewportEvent &event) {
    auto obj = emscripten::val::object();
    obj.set("zoomScale", event.zoomScale);
    obj.set("contentOffsetX", event.contentOffset.x);
    obj.set("contentOffsetY", event.contentOffset.y);

    auto visibleRect = emscripten::val::object();
    visibleRect.set("x", event.visibleOriginalRect.x());
    visibleRect.set("y", event.visibleOriginalRect.y());
    visibleRect.set("width", event.visibleOriginalRect.width());
    visibleRect.set("height", event.visibleOriginalRect.height());
    obj.set("visibleOriginalRect", visibleRect);

    return obj;
}

};  // namespace kk::web
