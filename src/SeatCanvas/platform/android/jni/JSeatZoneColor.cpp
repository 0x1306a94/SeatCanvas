//
//  JSeatZoneColor.cpp
//  SeatCanvas
//

#include "JSeatZoneColor.h"

#include "JNIHelper.hpp"
#include "JStringUtil.hpp"
#include "core/utils/ColorIntConverter.hpp"

#include <tgfx/core/Color.h>
#include <tgfx/platform/Print.h>

namespace kk::jni {

static kk::jni::Global<jclass> SeatZoneColorClass;
static kk::jni::Global<jclass> IntegerClass;
static jfieldID SeatZoneColor_zoneId;
static jfieldID SeatZoneColor_alternateColor;
static jmethodID SeatZoneColor_constructor;
static jmethodID Integer_intValue;
static jmethodID Integer_valueOf;

void JSeatZoneColor::InitJNI(JNIEnv *env) {
    SeatZoneColorClass = env->FindClass("com/libseatcanvas/SeatZoneColor");
    if (SeatZoneColorClass.get() == nullptr) {
        tgfx::PrintError("Could not run JSeatZoneColor::InitJNI, SeatZoneColorClass is not found!");
        return;
    }
    IntegerClass = env->FindClass("java/lang/Integer");
    if (IntegerClass.get() == nullptr) {
        tgfx::PrintError("Could not find java/lang/Integer in JSeatZoneColor::InitJNI!");
        return;
    }
    SeatZoneColor_zoneId = env->GetFieldID(SeatZoneColorClass.get(), "zoneId", "Ljava/lang/String;");
    SeatZoneColor_alternateColor =
        env->GetFieldID(SeatZoneColorClass.get(), "alternateColor", "Ljava/lang/Integer;");
    SeatZoneColor_constructor = env->GetMethodID(
        SeatZoneColorClass.get(), "<init>", "(Ljava/lang/String;Ljava/lang/Integer;)V");
    if (SeatZoneColor_constructor == nullptr) {
        tgfx::PrintError("Could not get SeatZoneColor constructor!");
    }
    Integer_intValue = env->GetMethodID(IntegerClass.get(), "intValue", "()I");
    Integer_valueOf = env->GetStaticMethodID(IntegerClass.get(), "valueOf", "(I)Ljava/lang/Integer;");
    if (Integer_intValue == nullptr || Integer_valueOf == nullptr) {
        tgfx::PrintError("Could not get Integer intValue or valueOf in JSeatZoneColor::InitJNI!");
    }
}

std::optional<std::string> JSeatZoneColor::ReadZoneIdFromJava(JNIEnv *env, jobject object) {
    auto clazz = SeatZoneColorClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return std::nullopt;
    }
    auto jzoneId = static_cast<jstring>(env->GetObjectField(object, SeatZoneColor_zoneId));
    auto zoneId = SafeConvertToStdString(env, jzoneId);
    if (zoneId.empty()) {
        return std::nullopt;
    }

    return zoneId;
}

std::optional<tgfx::Color> JSeatZoneColor::ReadAlternateColorFromJava(JNIEnv *env, jobject object) {
    auto clazz = SeatZoneColorClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return std::nullopt;
    }

    std::optional<tgfx::Color> alternateColor = std::nullopt;
    auto alternateColorObj = env->GetObjectField(object, SeatZoneColor_alternateColor);
    if (alternateColorObj != nullptr && Integer_intValue != nullptr) {
        auto argb = static_cast<uint32_t>(
            env->CallIntMethod(alternateColorObj, Integer_intValue));
        alternateColor = kk::utils::ColorFromARGBInt(argb);
    }

    return alternateColor;
}

}  // namespace kk::jni
