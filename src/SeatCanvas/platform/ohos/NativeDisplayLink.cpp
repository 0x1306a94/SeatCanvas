//
//  NativeDisplayLink.cpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#include "NativeDisplayLink.hpp"

#include "core/UniqueID.h"

#include <mutex>
#include <unordered_map>

#include "core/Log.hpp"

namespace kk {

static std::unordered_map<uint32_t, std::weak_ptr<NativeDisplayLink>> VSyncCallbacks = {};
static std::once_flag init_flag;
static napi_threadsafe_function js_threadsafe_function = nullptr;
void NativeDisplayLink::DisplaySoloistCallback(long long timestamp, long long targetTimestamp, void *data) {
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
    : callback(std::move(callback))
    , id(kk::UniqueID::Next()) {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);

    displaySoloist = OH_DisplaySoloist_Create(true);
    DisplaySoloist_ExpectedRateRange expectedRateRange{60, 120, 120};
    OH_DisplaySoloist_SetExpectedFrameRateRange(displaySoloist, &expectedRateRange);
}

NativeDisplayLink::~NativeDisplayLink() {
    VSyncCallbacks.erase(id);
    started = false;
    if (displaySoloist != nullptr) {
        OH_DisplaySoloist_Destroy(displaySoloist);
        displaySoloist = nullptr;
    }
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

void NativeDisplayLink::start() {
    if (started) {
        return;
    }

    if (displaySoloist == nullptr) {
        return;
    }
    VSyncCallbacks.insert_or_assign(id, shared_from_this());
    OH_DisplaySoloist_Start(displaySoloist, &DisplaySoloistCallback, &id);
    started = true;
}

void NativeDisplayLink::stop() {
    if (!started) {
        return;
    }

    started = false;
    VSyncCallbacks.erase(id);
    if (displaySoloist) {
        OH_DisplaySoloist_Stop(displaySoloist);
    }
}

void NativeDisplayLink::update() {
    if (callback && started) {
        callback();
    }
}

};  // namespace kk
