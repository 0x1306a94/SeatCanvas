//
//  JZoomLevel.cpp
//  SeatCanvas
//
//  Created by KK on 2026/2/10.
//

#include "JZoomLevel.h"

#include "JNIHelper.hpp"

#include <tgfx/platform/Print.h>

namespace kk::jni {

static kk::jni::Global<jclass> ZoomLevelClass;
static jmethodID ZoomLevel_constructor;

void JZoomLevel::InitJNI(JNIEnv *env) {
    ZoomLevelClass = env->FindClass("com/libseatcanvas/ZoomLevel");
    if (ZoomLevelClass.get() == nullptr) {
        tgfx::PrintError("Could not run JZoomLevel::InitJNI, ZoomLevelClass is not found!");
        return;
    }
    // 获取构造函数：ZoomLevel(seat: Float, row: Float, zone: Float, venue: Float)
    ZoomLevel_constructor = env->GetMethodID(ZoomLevelClass.get(), "<init>", "(FFFF)V");
    if (ZoomLevel_constructor == nullptr) {
        tgfx::PrintError("Could not get ZoomLevel constructor!");
    }
}

jobject JZoomLevel::ToJava(JNIEnv *env, const kk::ZoomLevelConfig &config) {
    auto clazz = ZoomLevelClass.get();
    if (env == nullptr || clazz == nullptr || ZoomLevel_constructor == nullptr) {
        return nullptr;
    }

    jobject result = env->NewObject(clazz, ZoomLevel_constructor,
                                    static_cast<jfloat>(config.seat),
                                    static_cast<jfloat>(config.row),
                                    static_cast<jfloat>(config.zone),
                                    static_cast<jfloat>(config.venue));
    return result;
}

};  // namespace kk::jni
