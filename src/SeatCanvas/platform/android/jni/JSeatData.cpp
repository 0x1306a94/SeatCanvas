//
//  JSeatData.cpp
//  SeatCanvas
//

#include "JSeatData.h"

#include "JNIHelper.hpp"
#include "JStringUtil.hpp"

#include <functional>
#include <tgfx/platform/Print.h>

namespace kk::jni {

static kk::jni::Global<jclass> SeatDataClass;
static jfieldID SeatData_seatId = nullptr;
static jfieldID SeatData_x = nullptr;
static jfieldID SeatData_y = nullptr;
static jfieldID SeatData_rotation = nullptr;
static jfieldID SeatData_pricecode = nullptr;
static jmethodID SeatData_constructor = nullptr;

void JSeatData::InitJNI(JNIEnv *env) {
    SeatDataClass = env->FindClass("com/libseatcanvas/SeatData");
    if (SeatDataClass.get() == nullptr) {
        tgfx::PrintError("Could not run JSeatData::InitJNI, SeatDataClass is not found!");
        return;
    }
    SeatData_seatId = env->GetFieldID(SeatDataClass.get(), "seatId", "Ljava/lang/String;");
    SeatData_x = env->GetFieldID(SeatDataClass.get(), "x", "F");
    SeatData_y = env->GetFieldID(SeatDataClass.get(), "y", "F");
    SeatData_rotation = env->GetFieldID(SeatDataClass.get(), "rotation", "F");
    SeatData_pricecode = env->GetFieldID(SeatDataClass.get(), "pricecode", "Ljava/lang/String;");
    SeatData_constructor = env->GetMethodID(SeatDataClass.get(), "<init>", "(Ljava/lang/String;FFFLjava/lang/String;)V");
    if (SeatData_constructor == nullptr) {
        tgfx::PrintError("Could not get SeatData constructor!");
    }
}

std::optional<kk::SeatData> JSeatData::FromJava(JNIEnv *env, jobject object, const PricecodeIndexResolver &resolver) {
    auto clazz = SeatDataClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return std::nullopt;
    }
    auto jseatId = static_cast<jstring>(env->GetObjectField(object, SeatData_seatId));
    auto seatId = SafeConvertToStdString(env, jseatId);
    auto x = env->GetFloatField(object, SeatData_x);
    auto y = env->GetFloatField(object, SeatData_y);
    auto rotation = env->GetFloatField(object, SeatData_rotation);

    uint16_t pricecodeIndex = kk::kNoPricecodeIndex;
    if (SeatData_pricecode != nullptr) {
        auto jpricecode = static_cast<jstring>(env->GetObjectField(object, SeatData_pricecode));
        if (jpricecode != nullptr) {
            auto pricecode = SafeConvertToStdString(env, jpricecode);
            if (resolver) {
                pricecodeIndex = resolver(pricecode);
            }
            env->DeleteLocalRef(jpricecode);
        }
    }

    return kk::SeatData(seatId, x, y, rotation, pricecodeIndex);
}

jobject JSeatData::ToJava(JNIEnv *env, const kk::SeatData &data, const PricecodeStringResolver &resolver) {
    auto clazz = SeatDataClass.get();
    if (env == nullptr || clazz == nullptr || SeatData_constructor == nullptr) {
        return nullptr;
    }
    auto jseatId = SafeConvertToJString(env, data.seatId);
    std::string pricecode = {};
    if (resolver) {
        pricecode = resolver(data.pricecodeIndex);
    }
    auto jpricecode = SafeConvertToJString(env, pricecode);
    jobject result = env->NewObject(clazz, SeatData_constructor, jseatId, static_cast<jfloat>(data.x),
                                    static_cast<jfloat>(data.y), static_cast<jfloat>(data.rotation), jpricecode);
    if (jseatId != nullptr) {
        env->DeleteLocalRef(jseatId);
    }
    if (jpricecode != nullptr) {
        env->DeleteLocalRef(jpricecode);
    }
    return result;
}

}  // namespace kk::jni
