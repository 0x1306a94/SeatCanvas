//
//  JRect.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/5.
//

#include "JRect.h"

#include "JNIHelper.hpp"

#include "core/Log.hpp"

namespace kk::jni {

static kk::jni::Global<jclass> RectClass;
static jfieldID Rect_x;
static jfieldID Rect_y;
static jfieldID Rect_width;
static jfieldID Rect_height;
static jmethodID Rect_constructor;

void JRect::InitJNI(JNIEnv *env) {
    RectClass = env->FindClass("com/libseatcanvas/Rect");
    if (RectClass.get() == nullptr) {
        SC_LOG_ERROR("Could not run JRect::InitJNI, RectClass is not found!");
        return;
    }
    Rect_x = env->GetFieldID(RectClass.get(), "x", "F");
    Rect_y = env->GetFieldID(RectClass.get(), "y", "F");
    Rect_width = env->GetFieldID(RectClass.get(), "width", "F");
    Rect_height = env->GetFieldID(RectClass.get(), "height", "F");
    // 获取构造函数：Rect(x: Float, y: Float, width: Float, height: Float)
    Rect_constructor = env->GetMethodID(RectClass.get(), "<init>", "(FFFF)V");
    if (Rect_constructor == nullptr) {
        SC_LOG_ERROR("Could not get Rect constructor!");
    }
}

tgfx::Rect JRect::FromJava(JNIEnv *env, jobject object) {
    auto clazz = RectClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return tgfx::Rect::MakeEmpty();
    }
    auto x = env->GetFloatField(object, Rect_x);
    auto y = env->GetFloatField(object, Rect_y);
    auto w = env->GetFloatField(object, Rect_width);
    auto h = env->GetFloatField(object, Rect_height);
    return tgfx::Rect::MakeXYWH(x, y, w, h);
}

jobject JRect::ToJava(JNIEnv *env, const tgfx::Rect &rect) {
    auto clazz = RectClass.get();
    if (env == nullptr || clazz == nullptr || Rect_constructor == nullptr) {
        return nullptr;
    }

    // 创建 Rect 对象
    jobject result = env->NewObject(clazz, Rect_constructor,
                                    static_cast<jfloat>(rect.x()),
                                    static_cast<jfloat>(rect.y()),
                                    static_cast<jfloat>(rect.width()),
                                    static_cast<jfloat>(rect.height()));
    return result;
}

};  // namespace kk::jni