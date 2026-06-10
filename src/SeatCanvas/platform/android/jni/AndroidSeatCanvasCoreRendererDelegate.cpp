//
//  AndroidSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#include "AndroidSeatCanvasCoreRendererDelegate.hpp"

#include "core/Log.hpp"
#include <jni.h>
#include <tgfx/platform/android/JNIEnvironment.h>

#include "JStringUtil.hpp"

namespace kk::renderer {

AndroidSeatCanvasCoreRendererDelegate::AndroidSeatCanvasCoreRendererDelegate(jobject seatCanvasView)
    : _seatCanvasView(seatCanvasView) {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

AndroidSeatCanvasCoreRendererDelegate::~AndroidSeatCanvasCoreRendererDelegate() {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

void AndroidSeatCanvasCoreRendererDelegate::didLoadBaseMap(uint32_t) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidLoadBaseMap", "()V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(clazz);
}

void AndroidSeatCanvasCoreRendererDelegate::didUnloadBaseMap(uint32_t) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidUnloadBaseMap", "()V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID);

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(clazz);
}

void AndroidSeatCanvasCoreRendererDelegate::didUpdateZoomLevelConfig(uint32_t, const SeatCanvasZoomLevelConfigEvent &event) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidUpdateZoomLevelConfig", "(FFFFFFF)V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID,
                        static_cast<jfloat>(event.zoomLevels.seat),
                        static_cast<jfloat>(event.zoomLevels.row),
                        static_cast<jfloat>(event.zoomLevels.zone),
                        static_cast<jfloat>(event.zoomLevels.venue),
                        static_cast<jfloat>(event.minimumZoomScale),
                        static_cast<jfloat>(event.maximumZoomScale),
                        static_cast<jfloat>(event.zoomScale));

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(clazz);
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

bool AndroidSeatCanvasCoreRendererDelegate::didTapSeat(uint32_t coreID, const std::string &zoneId,
                                                       const std::string &seatId) {
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

    jmethodID methodID = env->GetMethodID(clazz, "nativeOnDidTapSeat", "(Ljava/lang/String;Ljava/lang/String;)Z");
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

void AndroidSeatCanvasCoreRendererDelegate::viewportWillBeginDragging(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportWillBeginDragging", event);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportDidScroll(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportDidScroll", event);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportDidEndDragging(uint32_t, const SeatCanvasViewportEvent &event, bool willDecelerate) {
    callViewportMethod("nativeOnViewportDidEndDragging", event, willDecelerate);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportDidEndDecelerating(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportDidEndDecelerating", event);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportWillBeginZooming(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportWillBeginZooming", event);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportDidZoom(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportDidZoom", event);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportDidEndZooming(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportDidEndZooming", event);
}

void AndroidSeatCanvasCoreRendererDelegate::viewportDidEndScrollingAnimation(uint32_t, const SeatCanvasViewportEvent &event) {
    callViewportMethod("nativeOnViewportDidEndScrollingAnimation", event);
}

void AndroidSeatCanvasCoreRendererDelegate::callViewportMethod(const char *methodName, const SeatCanvasViewportEvent &event) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, methodName, "(FFFFFFF)V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID,
                        static_cast<jfloat>(event.zoomScale),
                        static_cast<jfloat>(event.contentOffset.x),
                        static_cast<jfloat>(event.contentOffset.y),
                        static_cast<jfloat>(event.visibleOriginalRect.x()),
                        static_cast<jfloat>(event.visibleOriginalRect.y()),
                        static_cast<jfloat>(event.visibleOriginalRect.width()),
                        static_cast<jfloat>(event.visibleOriginalRect.height()));

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(clazz);
}

void AndroidSeatCanvasCoreRendererDelegate::callViewportMethod(const char *methodName, const SeatCanvasViewportEvent &event, bool willDecelerate) {
    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr || _seatCanvasView.isEmpty()) {
        return;
    }

    jclass clazz = env->GetObjectClass(_seatCanvasView.get());
    if (clazz == nullptr) {
        env->ExceptionClear();
        return;
    }

    jmethodID methodID = env->GetMethodID(clazz, methodName, "(FFFFFFFZ)V");
    if (methodID == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(clazz);
        return;
    }

    env->CallVoidMethod(_seatCanvasView.get(), methodID,
                        static_cast<jfloat>(event.zoomScale),
                        static_cast<jfloat>(event.contentOffset.x),
                        static_cast<jfloat>(event.contentOffset.y),
                        static_cast<jfloat>(event.visibleOriginalRect.x()),
                        static_cast<jfloat>(event.visibleOriginalRect.y()),
                        static_cast<jfloat>(event.visibleOriginalRect.width()),
                        static_cast<jfloat>(event.visibleOriginalRect.height()),
                        static_cast<jboolean>(willDecelerate));

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(clazz);
}

};  // namespace kk::renderer
