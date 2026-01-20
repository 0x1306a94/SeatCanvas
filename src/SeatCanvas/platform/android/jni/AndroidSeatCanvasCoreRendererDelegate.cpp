//
//  AndroidSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#include "AndroidSeatCanvasCoreRendererDelegate.hpp"

#include <jni.h>
#include <tgfx/platform/Print.h>
#include <tgfx/platform/android/JNIEnvironment.h>

#include "JStringUtil.hpp"

namespace kk::renderer {

AndroidSeatCanvasCoreRendererDelegate::AndroidSeatCanvasCoreRendererDelegate(jobject seatCanvasView)
    : _seatCanvasView(seatCanvasView) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

AndroidSeatCanvasCoreRendererDelegate::~AndroidSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void AndroidSeatCanvasCoreRendererDelegate::didTapZone(uint32_t coreID, const std::string &zoneId) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    auto jzoneId = kk::jni::SafeConvertToJString(env, zoneId);

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(jzoneId);
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidTapZone", "(Ljava/lang/String;)V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jzoneId);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID, jzoneId);

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jzoneId);
}

bool AndroidSeatCanvasCoreRendererDelegate::shouldSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return false;
    }

    auto jzoneId = kk::jni::SafeConvertToJString(env, zoneId);
    auto jSeatId = kk::jni::SafeConvertToJString(env, seatId);

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(jzoneId);
        env->DeleteLocalRef(jSeatId);
        return false;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnShouldSelectSeat", "(Ljava/lang/String;Ljava/lang/String;)Z");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jzoneId);
        env->DeleteLocalRef(jSeatId);
        return false;
    }

    jboolean result = env->CallBooleanMethod(_seatCanvasView.get(), methodID, jzoneId, jSeatId);

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jzoneId);
    env->DeleteLocalRef(jSeatId);

    return result == JNI_TRUE;
}

void AndroidSeatCanvasCoreRendererDelegate::didSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    auto jzoneId = kk::jni::SafeConvertToJString(env, zoneId);
    auto jSeatId = kk::jni::SafeConvertToJString(env, seatId);

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(jzoneId);
        env->DeleteLocalRef(jSeatId);
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidSelectSeat", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jzoneId);
        env->DeleteLocalRef(jSeatId);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID, jzoneId, jSeatId);

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jzoneId);
    env->DeleteLocalRef(jSeatId);
}

void AndroidSeatCanvasCoreRendererDelegate::didDeselectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    auto jzoneId = kk::jni::SafeConvertToJString(env, zoneId);
    auto jSeatId = kk::jni::SafeConvertToJString(env, seatId);

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(jzoneId);
        env->DeleteLocalRef(jSeatId);
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidDeselectSeat", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        env->DeleteLocalRef(jzoneId);
        env->DeleteLocalRef(jSeatId);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID, jzoneId, jSeatId);

    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jzoneId);
    env->DeleteLocalRef(jSeatId);
}

};  // namespace kk::renderer
