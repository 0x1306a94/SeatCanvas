//
//  JsLoader.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "JFont.h"
#include "JRendererCore.h"
#include "JSeatRenderStyleId.h"
#include "NativeDisplayLink.hpp"
#include "XComponentHandler.h"
#include "core/gesture/GestureState.hpp"

#include <cstdint>
#include <napi/native_api.h>

#include "core/Log.hpp"

static void InitEnums(napi_env env, napi_value exports) {
    napi_value GestureState;
    napi_create_object(env, &GestureState);

    napi_value v;
    napi_create_int32(env, static_cast<int32_t>(kk::gesture::GestureState::POSSIBLE), &v);
    napi_set_named_property(env, GestureState, "POSSIBLE", v);
    napi_create_int32(env, static_cast<int32_t>(kk::gesture::GestureState::BEGAN), &v);
    napi_set_named_property(env, GestureState, "BEGAN", v);
    napi_create_int32(env, static_cast<int32_t>(kk::gesture::GestureState::CHANGED), &v);
    napi_set_named_property(env, GestureState, "CHANGED", v);
    napi_create_int32(env, static_cast<int32_t>(kk::gesture::GestureState::ENDED), &v);
    napi_set_named_property(env, GestureState, "ENDED", v);
    napi_create_int32(env, static_cast<int32_t>(kk::gesture::GestureState::CANCELLED), &v);
    napi_set_named_property(env, GestureState, "CANCELLED", v);
    napi_set_named_property(env, exports, "GestureState", GestureState);
}

static napi_value Init(napi_env env, napi_value exports) {
    SC_LOG_INFO("Init called");

    napi_value namespaceExports;
    napi_create_object(env, &namespaceExports);
    napi_set_named_property(env, exports, "seatcanvas", namespaceExports);

    kk::js::JFont::Init(env, namespaceExports);
    kk::js::JRendererCore::Init(env, namespaceExports);
    kk::js::JSeatRenderStyleId::Init(env, namespaceExports);
    InitEnums(env, namespaceExports);

    kk::js::XComponentHandler::Init(env, exports);
    kk::NativeDisplayLink::Init(env, exports);
    return exports;
}

NAPI_MODULE_X(seatcanvas, Init, ((void *)0), 0);