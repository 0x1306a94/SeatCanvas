//
//  NativeDisplayLink.cpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#include "NativeDisplayLink.hpp"

#include <tgfx/platform/Print.h>

namespace kk {
static kk::jni::Global<jclass> DisplayLinkClass;
static jmethodID DisplayLink_Create;
static jmethodID DisplayLink_start;
static jmethodID DisplayLink_stop;

void NativeDisplayLink::InitJNI(JNIEnv *env) {
    DisplayLinkClass = env->FindClass("com/libseatcanvas/DisplayLink");
    if (DisplayLinkClass.get() == nullptr) {
        tgfx::PrintError("Could not run NativeDisplayLink.InitJNI(), DisplayLinkClass is not found!");
        return;
    }
    DisplayLink_Create = env->GetStaticMethodID(DisplayLinkClass.get(), "Create", "(J)Lcom/libseatcanvas/DisplayLink;");
    DisplayLink_start = env->GetMethodID(DisplayLinkClass.get(), "start", "()V");
    DisplayLink_stop = env->GetMethodID(DisplayLinkClass.get(), "stop", "()V");
}

std::shared_ptr<DisplayLink> NativeDisplayLink::Make(std::function<void()> callback) {

    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr) {
        return nullptr;
    }
    auto displayLink = std::shared_ptr<NativeDisplayLink>(new NativeDisplayLink(std::move(callback)));
    displayLink->_animator = env->CallStaticObjectMethod(DisplayLinkClass.get(), DisplayLink_Create, reinterpret_cast<jlong>(displayLink.get()));
    if (displayLink->_animator.isEmpty()) {
        return nullptr;
    }
    return displayLink;
}

NativeDisplayLink::NativeDisplayLink(std::function<void()> callback)
    : _callback(std::move(callback)) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

NativeDisplayLink::~NativeDisplayLink() {
    _started = false;
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void NativeDisplayLink::start() {

    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _animator.isEmpty()) {
        return;
    }
    _started = true;
    env->CallVoidMethod(_animator.get(), DisplayLink_start);
}

void NativeDisplayLink::stop() {

    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _animator.isEmpty()) {
        return;
    }
    _started = false;
    env->CallVoidMethod(_animator.get(), DisplayLink_stop);
}

void NativeDisplayLink::update() {
    if (_callback && _started) {
        _callback();
    }
}

};  // namespace kk

extern "C" {
JNIEXPORT void JNICALL Java_com_libseatcanvas_DisplayLink_nativeUpdate(JNIEnv *, jobject, jlong context) {
    auto displayLink = reinterpret_cast<kk::NativeDisplayLink *>(context);
    if (displayLink == nullptr) {
        return;
    }
    displayLink->update();
}
}