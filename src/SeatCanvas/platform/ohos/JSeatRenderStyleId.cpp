//
//  JSeatRenderStyleId.cpp
//  SeatCanvas
//

#include "JSeatRenderStyleId.h"

#include "JsHelper.h"
#include "core/style/SeatRenderStyleKey.hpp"

#include "core/Log.hpp"

namespace kk::js {

napi_value JSeatRenderStyleId::Constructor(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &result, nullptr);
    return result;
}

static napi_value Compose(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3] = {nullptr, nullptr, nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    std::string pricecode = {};
    if (argc > 0 && args[0] != nullptr) {
        pricecode = GetUtf8String(env, args[0]);
    }

    uint32_t status = 0;
    if (argc > 1 && args[1] != nullptr) {
        int32_t statusValue = 0;
        napi_get_value_int32(env, args[1], &statusValue);
        status = static_cast<uint32_t>(statusValue);
    }

    bool selected = false;
    if (argc > 2 && args[2] != nullptr) {
        napi_get_value_bool(env, args[2], &selected);
    }

    auto styleId = kk::renderer::composeSeatStyleId(pricecode, status, selected);
    napi_value result = nullptr;
    napi_create_string_utf8(env, styleId.c_str(), styleId.length(), &result);
    return result;
}

bool JSeatRenderStyleId::Init(napi_env env, napi_value exports) {
    napi_property_descriptor classProp[] = {
        JS_STATIC_METHOD_ENTRY(compose, Compose),
    };

    auto status = DefineClass(env, exports, "JSeatRenderStyleId", sizeof(classProp) / sizeof(classProp[0]), classProp, JSeatRenderStyleId::Constructor, "");
    if (status != napi_ok) {
        SC_LOG_ERROR("JSeatRenderStyleId::Init DefineClass failed: %d", status);
        return false;
    }
    return true;
}

}  // namespace kk::js
