//
//  JHitTestSeatRegionResult.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/5.
//

#include "JHitTestSeatRegionResult.h"

#include "JNIHelper.hpp"
#include "JRect.h"
#include "JStringUtil.hpp"

#include <tgfx/platform/Print.h>

namespace kk::jni {

static Global<jclass> HitTestSeatRegionResultClass;
static jfieldID HitTestSeatRegionResult_regionId;
static jfieldID HitTestSeatRegionResult_bounds;
static jmethodID HitTestSeatRegionResult_constructor;

void JHitTestSeatRegionResult::InitJNI(JNIEnv *env) {
    HitTestSeatRegionResultClass = env->FindClass("com/libseatcanvas/HitTestSeatRegionResult");
    if (HitTestSeatRegionResultClass.get() == nullptr) {
        tgfx::PrintError("Could not run JHitTestSeatRegionResult::InitJNI, HitTestSeatRegionResultClass is not found!");
        return;
    }

    // 获取字段 ID
    HitTestSeatRegionResult_regionId = env->GetFieldID(HitTestSeatRegionResultClass.get(), "regionId", "Ljava/lang/String;");
    HitTestSeatRegionResult_bounds = env->GetFieldID(HitTestSeatRegionResultClass.get(), "bounds", "Lcom/libseatcanvas/Rect;");

    // 获取构造函数：HitTestSeatRegionResult(regionId: String, bounds: Rect)
    HitTestSeatRegionResult_constructor = env->GetMethodID(HitTestSeatRegionResultClass.get(), "<init>", "(Ljava/lang/String;Lcom/libseatcanvas/Rect;)V");

    if (HitTestSeatRegionResult_regionId == nullptr || HitTestSeatRegionResult_bounds == nullptr || HitTestSeatRegionResult_constructor == nullptr) {
        tgfx::PrintError("Could not get field/method IDs for HitTestSeatRegionResult!");
    }
}

jobject JHitTestSeatRegionResult::ToJava(JNIEnv *env, const kk::RegionInfo *regionInfo) {
    auto clazz = HitTestSeatRegionResultClass.get();
    if (env == nullptr || clazz == nullptr || regionInfo == nullptr) {
        return nullptr;
    }

    if (HitTestSeatRegionResult_constructor == nullptr) {
        return nullptr;
    }

    // 转换 regionId (String)
    auto regionIdStr = SafeConvertToJString(env, regionInfo->regionId);
    if (regionIdStr == nullptr) {
        return nullptr;
    }

    // 转换 bounds (Rect)
    auto boundsObj = JRect::ToJava(env, regionInfo->bounds);
    if (boundsObj == nullptr) {
        env->DeleteLocalRef(regionIdStr);
        return nullptr;
    }

    // 创建 HitTestSeatRegionResult 对象
    auto result = env->NewObject(clazz, HitTestSeatRegionResult_constructor, regionIdStr, boundsObj);

    // 清理局部引用
    env->DeleteLocalRef(regionIdStr);
    env->DeleteLocalRef(boundsObj);

    return result;
}

std::optional<kk::RegionInfo> JHitTestSeatRegionResult::FromJava(JNIEnv *env, jobject object) {
    kk::RegionInfo result;

    auto clazz = HitTestSeatRegionResultClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return std::nullopt;
    }

    if (HitTestSeatRegionResult_regionId == nullptr || HitTestSeatRegionResult_bounds == nullptr) {
        return std::nullopt;
    }

    // 获取 regionId
    auto regionIdStr = (jstring)env->GetObjectField(object, HitTestSeatRegionResult_regionId);
    if (regionIdStr != nullptr) {
        result.regionId = SafeConvertToStdString(env, regionIdStr);
        env->DeleteLocalRef(regionIdStr);
    }

    // 获取 bounds
    auto boundsObj = env->GetObjectField(object, HitTestSeatRegionResult_bounds);
    if (boundsObj != nullptr) {
        result.bounds = JRect::FromJava(env, boundsObj);
        env->DeleteLocalRef(boundsObj);
    }

    return result;
}

};  // namespace kk::jni
