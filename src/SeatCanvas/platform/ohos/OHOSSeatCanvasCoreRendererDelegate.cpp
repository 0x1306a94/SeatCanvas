//
//  OHOSSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#include "OHOSSeatCanvasCoreRendererDelegate.hpp"

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
        if (_shouldSelectSeat != nullptr) {
            napi_delete_reference(env, _shouldSelectSeat);
            _shouldSelectSeat = nullptr;
        }
        if (_didSelectSeat != nullptr) {
            napi_delete_reference(env, _didSelectSeat);
            _didSelectSeat = nullptr;
        }
        if (_didDeselectSeat != nullptr) {
            napi_delete_reference(env, _didDeselectSeat);
            _didDeselectSeat = nullptr;
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

void OHOSSeatCanvasCoreRendererDelegate::setShouldSelectSeatCallback(napi_env env, napi_value callback) {
    if (env == nullptr) {
        return;
    }

    if (_shouldSelectSeat != nullptr) {
        napi_delete_reference(env, _shouldSelectSeat);
        _shouldSelectSeat = nullptr;
    }

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &_shouldSelectSeat);
    }
}

void OHOSSeatCanvasCoreRendererDelegate::setDidSelectSeatCallback(napi_env env, napi_value callback) {
    if (env == nullptr) {
        return;
    }

    if (_didSelectSeat != nullptr) {
        napi_delete_reference(env, _didSelectSeat);
        _didSelectSeat = nullptr;
    }

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &_didSelectSeat);
    }
}

void OHOSSeatCanvasCoreRendererDelegate::setDidDeselectSeatCallback(napi_env env, napi_value callback) {
    if (env == nullptr) {
        return;
    }

    if (_didDeselectSeat != nullptr) {
        napi_delete_reference(env, _didDeselectSeat);
        _didDeselectSeat = nullptr;
    }

    if (callback != nullptr) {
        napi_create_reference(env, callback, 1, &_didDeselectSeat);
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

bool OHOSSeatCanvasCoreRendererDelegate::shouldSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    if (_shouldSelectSeat == nullptr) {
        return false;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return false;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _shouldSelectSeat, &callback);
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

void OHOSSeatCanvasCoreRendererDelegate::didSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    if (_didSelectSeat == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didSelectSeat, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_value zoneIdValue = nullptr;
    napi_value seatIdValue = nullptr;
    napi_create_string_utf8(env, zoneId.c_str(), zoneId.length(), &zoneIdValue);
    napi_create_string_utf8(env, seatId.c_str(), seatId.length(), &seatIdValue);

    napi_value argv[2] = {zoneIdValue, seatIdValue};
    napi_call_function(env, nullptr, callback, 2, argv, nullptr);
}

void OHOSSeatCanvasCoreRendererDelegate::didDeselectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    if (_didDeselectSeat == nullptr) {
        return;
    }

    napi_env env = kk::js::NapiEnvHolder::getEnv();
    if (env == nullptr) {
        return;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env, _didDeselectSeat, &callback);
    if (callback == nullptr) {
        return;
    }

    napi_value zoneIdValue = nullptr;
    napi_value seatIdValue = nullptr;
    napi_create_string_utf8(env, zoneId.c_str(), zoneId.length(), &zoneIdValue);
    napi_create_string_utf8(env, seatId.c_str(), seatId.length(), &seatIdValue);

    napi_value argv[2] = {zoneIdValue, seatIdValue};
    napi_call_function(env, nullptr, callback, 2, argv, nullptr);
}

};  // namespace kk::js
