//
//  NativeDisplayLink.cpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#include "NativeDisplayLink.hpp"

#include "core/UniqueID.h"

#include <cstring>
#include <mutex>
#include <unordered_map>

#include <tgfx/platform/Print.h>

namespace kk {

static std::unordered_map<uint32_t, std::weak_ptr<NativeDisplayLink>> VSyncCallbacks = {};
static std::once_flag init_flag;
static napi_threadsafe_function js_threadsafe_function = nullptr;
void NativeDisplayLink::VSyncCallback(long long, void *data) {
    if (js_threadsafe_function == nullptr) {
        return;
    }

    napi_call_threadsafe_function(js_threadsafe_function, data, napi_tsfn_blocking);
}

static void CallJsFunction(napi_env env, napi_value callBack, [[maybe_unused]] void *context, void *data) {
    auto id = *static_cast<uint32_t *>(data);
    auto iter = VSyncCallbacks.find(id);
    if (iter == VSyncCallbacks.end()) {
        return;
    }

    if (auto displayLink = iter->second.lock(); displayLink) {
        displayLink->update();
    }
};

bool NativeDisplayLink::Init(napi_env env, napi_value exports) {
    std::call_once(init_flag, [env] {
        napi_value resourceName = nullptr;
        napi_create_string_utf8(env, "NativeDisplayLink Safe Function", NAPI_AUTO_LENGTH,
                                &resourceName);
        napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, nullptr, nullptr,
                                        nullptr, CallJsFunction, &js_threadsafe_function);
    });
    return js_threadsafe_function != nullptr;
}

NativeDisplayLink::NativeDisplayLink(std::function<void()> callback)
    : _callback(std::move(callback))
    , id(kk::UniqueID::Next()) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);

    char name[] = "seatcanvas_vsync";
    vSync = OH_NativeVSync_Create(name, strlen(name));
}

NativeDisplayLink::~NativeDisplayLink() {
    VSyncCallbacks.erase(id);
    _started = false;
    if (vSync != nullptr) {
        OH_NativeVSync_Destroy(vSync);
        vSync = nullptr;
    }
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void NativeDisplayLink::start() {
    if (_started) {
        return;
    }

    if (vSync == nullptr) {
        return;
    }
    VSyncCallbacks.insert_or_assign(id, shared_from_this());
    OH_NativeVSync_RequestFrame(vSync, &VSyncCallback, &id);
    _started = true;
}

void NativeDisplayLink::stop() {
    _started = false;
    VSyncCallbacks.erase(id);
}

void NativeDisplayLink::update() {
    if (_started) {
        _callback();
        OH_NativeVSync_RequestFrame(vSync, &VSyncCallback, &id);
    }
}

};  // namespace kk
