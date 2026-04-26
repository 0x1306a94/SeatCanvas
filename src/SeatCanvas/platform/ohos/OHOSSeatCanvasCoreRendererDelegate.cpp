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
        if (_styleIdForSeat != nullptr) {
            napi_delete_reference(env, _styleIdForSeat);
            _styleIdForSeat = nullptr;
        }
        if (_didTapSeat != nullptr) {
            napi_delete_reference(env, _didTapSeat);
            _didTapSeat = nullptr;
        }
        if (_didTapZone != nullptr) {
            napi_delete_reference(env, _didTapZone);
            _didTapZone = nullptr;
        }
    }
}

void OHOSSeatCanvasCoreRendererDelegate::setDidTapZoneCallback(napi_env env, napi_value callback) {
    if (env == nullptr) {
        return;
    }

    if (_didTapZone != nullptr) {
        napi_delete_reference(env, _didTapZone);
        _didTapZone = nullptr;
    }

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &_didTapZone);
    }
}

void OHOSSeatCanvasCoreRendererDelegate::setStyleIdForSeatCallback(napi_env env, napi_value callback) {
    if (env == nullptr) {
        return;
    }

    if (_styleIdForSeat != nullptr) {
        napi_delete_reference(env, _styleIdForSeat);
        _styleIdForSeat = nullptr;
    }

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &_styleIdForSeat);
    }
}

void OHOSSeatCanvasCoreRendererDelegate::setDidTapSeatCallback(napi_env env, napi_value callback) {
    if (env == nullptr) {
        return;
    }

    if (_didTapSeat != nullptr) {
        napi_delete_reference(env, _didTapSeat);
        _didTapSeat = nullptr;
    }

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &_didTapSeat);
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

bool OHOSSeatCanvasCoreRendererDelegate::styleIdForSeat(uint32_t coreID, const std::string &zoneId,
                                                        const std::string &seatId, std::string &outStyleId) {
    if (_styleIdForSeat == nullptr) {
        return false;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return false;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _styleIdForSeat, &callback);
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

    napi_valuetype resultType = napi_undefined;
    napi_typeof(env, result, &resultType);
    if (resultType == napi_null || resultType == napi_undefined) {
        return false;
    }

    napi_value strValue = nullptr;
    status = napi_coerce_to_string(env, result, &strValue);
    if (status != napi_ok || strValue == nullptr) {
        return false;
    }

    outStyleId = GetUtf8String(env, strValue);
    if (outStyleId.empty()) {
        return false;
    }
    return true;
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

};  // namespace kk::js
