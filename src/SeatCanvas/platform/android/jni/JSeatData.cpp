//
//  JSeatData.cpp
//  SeatCanvas
//

#include "JSeatData.h"

#include "JNIHelper.hpp"
#include "JStringUtil.hpp"

#include <tgfx/platform/Print.h>

namespace kk::jni {

static kk::jni::Global<jclass> SeatDataClass;
static jfieldID SeatData_seatId;
static jfieldID SeatData_status;
static jfieldID SeatData_selected;
static jfieldID SeatData_x;
static jfieldID SeatData_y;
static jmethodID SeatData_constructor;

void JSeatData::InitJNI(JNIEnv *env) {
    SeatDataClass = env->FindClass("com/libseatcanvas/SeatData");
    if (SeatDataClass.get() == nullptr) {
        tgfx::PrintError("Could not run JSeatData::InitJNI, SeatDataClass is not found!");
        return;
    }
    SeatData_seatId = env->GetFieldID(SeatDataClass.get(), "seatId", "Ljava/lang/String;");
    SeatData_status = env->GetFieldID(SeatDataClass.get(), "status", "I");
    SeatData_selected = env->GetFieldID(SeatDataClass.get(), "selected", "Z");
    SeatData_x = env->GetFieldID(SeatDataClass.get(), "x", "F");
    SeatData_y = env->GetFieldID(SeatDataClass.get(), "y", "F");
    SeatData_constructor =
        env->GetMethodID(SeatDataClass.get(), "<init>", "(Ljava/lang/String;IZFF)V");
    if (SeatData_constructor == nullptr) {
        tgfx::PrintError("Could not get SeatData constructor!");
    }
}

std::optional<kk::SeatData> JSeatData::FromJava(JNIEnv *env, jobject object) {
    auto clazz = SeatDataClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return std::nullopt;
    }
    auto jseatId = static_cast<jstring>(env->GetObjectField(object, SeatData_seatId));
    auto seatId = SafeConvertToStdString(env, jseatId);
    auto status = static_cast<uint32_t>(env->GetIntField(object, SeatData_status));
    auto selected = env->GetBooleanField(object, SeatData_selected) == JNI_TRUE;
    auto x = env->GetFloatField(object, SeatData_x);
    auto y = env->GetFloatField(object, SeatData_y);
    return kk::SeatData(seatId, status, selected, x, y);
}

jobject JSeatData::ToJava(JNIEnv *env, const kk::SeatData &data) {
    auto clazz = SeatDataClass.get();
    if (env == nullptr || clazz == nullptr || SeatData_constructor == nullptr) {
        return nullptr;
    }
    auto jseatId = SafeConvertToJString(env, data.seatId);
    jobject result =
        env->NewObject(clazz, SeatData_constructor, jseatId,
                       static_cast<jint>(data.status), static_cast<jboolean>(data.selected),
                       static_cast<jfloat>(data.x), static_cast<jfloat>(data.y));
    if (jseatId != nullptr) {
        env->DeleteLocalRef(jseatId);
    }
    return result;
}

}  // namespace kk::jni
