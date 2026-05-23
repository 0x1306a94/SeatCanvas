//
//  OHOSSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#include "OHOSSeatCanvasCoreRendererDelegate.hpp"

#include "JsHelper.h"
#include "NapiEnvHolder.hpp"

#include <napi/native_api.h>
#include <tgfx/platform/Print.h>

namespace kk::js {

OHOSSeatCanvasCoreRendererDelegate::OHOSSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

OHOSSeatCanvasCoreRendererDelegate::~OHOSSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env != nullptr) {
        clearCallback(env, _didTapSeat);
        clearCallback(env, _didTapZone);
        clearCallback(env, _viewportWillBeginDragging);
        clearCallback(env, _viewportDidScroll);
        clearCallback(env, _viewportDidEndDragging);
        clearCallback(env, _viewportDidEndDecelerating);
        clearCallback(env, _viewportWillBeginZooming);
        clearCallback(env, _viewportDidZoom);
        clearCallback(env, _viewportDidEndZooming);
        clearCallback(env, _viewportDidEndScrollingAnimation);
        clearCallback(env, _didLoadBaseMap);
        clearCallback(env, _didUnloadBaseMap);
        clearCallback(env, _didUpdateZoomLevelConfig);
    }
}

void OHOSSeatCanvasCoreRendererDelegate::setDidLoadBaseMapCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _didLoadBaseMap);
}

void OHOSSeatCanvasCoreRendererDelegate::setDidUnloadBaseMapCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _didUnloadBaseMap);
}

void OHOSSeatCanvasCoreRendererDelegate::setDidTapZoneCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _didTapZone);
}

void OHOSSeatCanvasCoreRendererDelegate::setDidTapSeatCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _didTapSeat);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportWillBeginDraggingCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportWillBeginDragging);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportDidScrollCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportDidScroll);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndDraggingCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportDidEndDragging);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndDeceleratingCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportDidEndDecelerating);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportWillBeginZoomingCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportWillBeginZooming);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportDidZoomCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportDidZoom);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndZoomingCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportDidEndZooming);
}

void OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndScrollingAnimationCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _viewportDidEndScrollingAnimation);
}

void OHOSSeatCanvasCoreRendererDelegate::setDidUpdateZoomLevelConfigCallback(napi_env env, napi_value callback) {
    setCallback(env, callback, _didUpdateZoomLevelConfig);
}

void OHOSSeatCanvasCoreRendererDelegate::setCallback(napi_env env, napi_value callback, napi_ref &ref) {
    if (env == nullptr) {
        return;
    }

    clearCallback(env, ref);

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &ref);
    }
}

void OHOSSeatCanvasCoreRendererDelegate::clearCallback(napi_env env, napi_ref &ref) {
    if (env != nullptr && ref != nullptr) {
        napi_delete_reference(env, ref);
        ref = nullptr;
    }
}

void OHOSSeatCanvasCoreRendererDelegate::didLoadBaseMap(uint32_t) {
    if (_didLoadBaseMap == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didLoadBaseMap, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_status status = napi_call_function(env, nullptr, callback, 0, nullptr, nullptr);
    if (status != napi_ok) {
        tgfx::PrintError("OHOSSeatCanvasCoreRendererDelegate::didLoadBaseMap failed: %d", static_cast<int>(status));
    }
}

void OHOSSeatCanvasCoreRendererDelegate::didUnloadBaseMap(uint32_t) {
    if (_didUnloadBaseMap == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didUnloadBaseMap, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_status status = napi_call_function(env, nullptr, callback, 0, nullptr, nullptr);
    if (status != napi_ok) {
        tgfx::PrintError("OHOSSeatCanvasCoreRendererDelegate::didUnloadBaseMap failed: %d", static_cast<int>(status));
    }
}

void OHOSSeatCanvasCoreRendererDelegate::didUpdateZoomLevelConfig(uint32_t, const kk::renderer::SeatCanvasZoomLevelConfigEvent &event) {
    if (_didUpdateZoomLevelConfig == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didUpdateZoomLevelConfig, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_value configEvent = makeZoomLevelConfigEventValue(env, event);
    napi_value argv[1] = {configEvent};
    napi_status status = napi_call_function(env, nullptr, callback, 1, argv, nullptr);
    if (status != napi_ok) {
        tgfx::PrintError("OHOSSeatCanvasCoreRendererDelegate::didUpdateZoomLevelConfig failed: %d", static_cast<int>(status));
    }
}

void OHOSSeatCanvasCoreRendererDelegate::didTapZone(uint32_t coreID, const std::string &zoneId) {
    if (_didTapZone == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didTapZone, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_value zoneIdValue = nullptr;
    napi_create_string_utf8(env, zoneId.c_str(), zoneId.length(), &zoneIdValue);

    napi_value argv[1] = {zoneIdValue};
    napi_call_function(env, nullptr, callback, 1, argv, nullptr);
}

bool OHOSSeatCanvasCoreRendererDelegate::didTapSeat(uint32_t coreID, const std::string &zoneId,
                                                    const std::string &seatId) {
    if (_didTapSeat == nullptr) {
        return false;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return false;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didTapSeat, &callback);
    if (callback == nullptr) {
        return false;
    }

    napi_value zoneIdValue = nullptr;
    napi_value seatIdValue = nullptr;
    napi_create_string_utf8(env, zoneId.c_str(), zoneId.length(), &zoneIdValue);
    napi_create_string_utf8(env, seatId.c_str(), seatId.length(), &seatIdValue);

    napi_value result = nullptr;
    napi_value argv[2] = {zoneIdValue, seatIdValue};
    napi_status status = napi_call_function(env, nullptr, callback, 2, argv, &result);

    if (status != napi_ok || result == nullptr) {
        return false;
    }

    bool boolResult = false;
    napi_get_value_bool(env, result, &boolResult);
    return boolResult;
}

napi_value OHOSSeatCanvasCoreRendererDelegate::makeZoomLevelConfigEventValue(napi_env env, const kk::renderer::SeatCanvasZoomLevelConfigEvent &event) {
    napi_value configEvent = nullptr;
    napi_create_object(env, &configEvent);

    napi_value zoomLevels = nullptr;
    napi_create_object(env, &zoomLevels);

    napi_value seat = nullptr;
    napi_create_double(env, event.zoomLevels.seat, &seat);
    napi_set_named_property(env, zoomLevels, "seat", seat);

    napi_value row = nullptr;
    napi_create_double(env, event.zoomLevels.row, &row);
    napi_set_named_property(env, zoomLevels, "row", row);

    napi_value zone = nullptr;
    napi_create_double(env, event.zoomLevels.zone, &zone);
    napi_set_named_property(env, zoomLevels, "zone", zone);

    napi_value venue = nullptr;
    napi_create_double(env, event.zoomLevels.venue, &venue);
    napi_set_named_property(env, zoomLevels, "venue", venue);

    napi_set_named_property(env, configEvent, "zoomLevels", zoomLevels);

    napi_value minimumZoomScale = nullptr;
    napi_create_double(env, event.minimumZoomScale, &minimumZoomScale);
    napi_set_named_property(env, configEvent, "minimumZoomScale", minimumZoomScale);

    napi_value maximumZoomScale = nullptr;
    napi_create_double(env, event.maximumZoomScale, &maximumZoomScale);
    napi_set_named_property(env, configEvent, "maximumZoomScale", maximumZoomScale);

    napi_value zoomScale = nullptr;
    napi_create_double(env, event.zoomScale, &zoomScale);
    napi_set_named_property(env, configEvent, "zoomScale", zoomScale);

    return configEvent;
}

napi_value OHOSSeatCanvasCoreRendererDelegate::makeViewportValue(napi_env env, const kk::renderer::SeatCanvasViewportEvent &event) {
    napi_value viewport = nullptr;
    napi_create_object(env, &viewport);

    napi_value zoomScaleValue = nullptr;
    napi_create_double(env, event.zoomScale, &zoomScaleValue);
    napi_set_named_property(env, viewport, "zoomScale", zoomScaleValue);

    napi_value contentOffsetXValue = nullptr;
    napi_create_double(env, event.contentOffset.x, &contentOffsetXValue);
    napi_set_named_property(env, viewport, "contentOffsetX", contentOffsetXValue);

    napi_value contentOffsetYValue = nullptr;
    napi_create_double(env, event.contentOffset.y, &contentOffsetYValue);
    napi_set_named_property(env, viewport, "contentOffsetY", contentOffsetYValue);

    napi_value visibleOriginalRect = nullptr;
    napi_create_object(env, &visibleOriginalRect);

    napi_value rectX = nullptr;
    napi_create_double(env, event.visibleOriginalRect.x(), &rectX);
    napi_set_named_property(env, visibleOriginalRect, "x", rectX);

    napi_value rectY = nullptr;
    napi_create_double(env, event.visibleOriginalRect.y(), &rectY);
    napi_set_named_property(env, visibleOriginalRect, "y", rectY);

    napi_value rectWidth = nullptr;
    napi_create_double(env, event.visibleOriginalRect.width(), &rectWidth);
    napi_set_named_property(env, visibleOriginalRect, "width", rectWidth);

    napi_value rectHeight = nullptr;
    napi_create_double(env, event.visibleOriginalRect.height(), &rectHeight);
    napi_set_named_property(env, visibleOriginalRect, "height", rectHeight);

    napi_set_named_property(env, viewport, "visibleOriginalRect", visibleOriginalRect);

    return viewport;
}

void OHOSSeatCanvasCoreRendererDelegate::callViewportCallback(napi_ref ref, const kk::renderer::SeatCanvasViewportEvent &event) {
    if (ref == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, ref, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_value viewport = makeViewportValue(env, event);
    napi_value argv[1] = {viewport};
    napi_status status = napi_call_function(env, nullptr, callback, 1, argv, nullptr);
    if (status != napi_ok) {
        tgfx::PrintError("OHOSSeatCanvasCoreRendererDelegate::callViewportCallback failed: %d", static_cast<int>(status));
    }
}

void OHOSSeatCanvasCoreRendererDelegate::callViewportCallback(napi_ref ref, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) {
    if (ref == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, ref, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_value viewport = makeViewportValue(env, event);
    napi_value willDecelerateValue = nullptr;
    napi_get_boolean(env, willDecelerate, &willDecelerateValue);
    napi_value argv[2] = {viewport, willDecelerateValue};
    napi_status status = napi_call_function(env, nullptr, callback, 2, argv, nullptr);
    if (status != napi_ok) {
        tgfx::PrintError("OHOSSeatCanvasCoreRendererDelegate::callViewportCallback(didEndDragging) failed: %d",
                         static_cast<int>(status));
    }
}

void OHOSSeatCanvasCoreRendererDelegate::viewportWillBeginDragging(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportWillBeginDragging, event);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportDidScroll(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidScroll, event);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportDidEndDragging(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) {
    callViewportCallback(_viewportDidEndDragging, event, willDecelerate);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportDidEndDecelerating(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidEndDecelerating, event);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportWillBeginZooming(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportWillBeginZooming, event);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportDidZoom(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidZoom, event);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportDidEndZooming(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidEndZooming, event);
}

void OHOSSeatCanvasCoreRendererDelegate::viewportDidEndScrollingAnimation(uint32_t, const kk::renderer::SeatCanvasViewportEvent &event) {
    callViewportCallback(_viewportDidEndScrollingAnimation, event);
}

};  // namespace kk::js
