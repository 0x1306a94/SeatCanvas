//
//  NativePlatform.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

#include "NativePlatform.hpp"

#include "NativeDisplayLink.hpp"
#include "jni/FontConfigAndroid.hpp"
#include "jni/JNIHelper.hpp"

namespace kk {

void NativePlatform::InitJNI() {
    static bool initialized = false;
    if (initialized) {
        return;
    }
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr) {
        return;
    }
    initialized = true;
    kk::jni::FontConfigAndroid::InitJNI(env);
    NativeDisplayLink::InitJNI(env);
    env->ExceptionClear();
}

const Platform *Platform::Current() {
    static const NativePlatform platform = {};
    return &platform;
}

bool NativePlatform::registerFallbackFonts() const {
    return kk::jni::FontConfigAndroid::RegisterFallbackFonts();
}

std::shared_ptr<DisplayLink> NativePlatform::createDisplayLink(std::function<void()> callback, void *userInfo) const {
    return NativeDisplayLink::Make(std::move(callback));
}
};  // namespace kk
