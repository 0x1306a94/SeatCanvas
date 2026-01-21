//
//  JSeatZoneData.cpp
//  SeatCanvas
//

#include "JSeatZoneData.h"

#include "JNIHelper.hpp"
#include "JStringUtil.hpp"

#include <tgfx/core/Color.h>
#include <tgfx/platform/Print.h>

namespace kk::jni {

static tgfx::Color ColorFromARGBInt(uint32_t argb) {
    auto a = static_cast<uint8_t>((argb >> 24) & 0xFF);
    auto r = static_cast<uint8_t>((argb >> 16) & 0xFF);
    auto g = static_cast<uint8_t>((argb >> 8) & 0xFF);
    auto b = static_cast<uint8_t>(argb & 0xFF);
    return tgfx::Color::FromRGBA(r, g, b, a);
}

static int32_t ColorToARGBInt(const tgfx::Color &color) {
    auto a = static_cast<uint8_t>(color.alpha * 255);
    auto r = static_cast<uint8_t>(color.red * 255);
    auto g = static_cast<uint8_t>(color.green * 255);
    auto b = static_cast<uint8_t>(color.blue * 255);
    return static_cast<int32_t>((a << 24) | (r << 16) | (g << 8) | b);
}

static kk::jni::Global<jclass> SeatZoneDataClass;
static kk::jni::Global<jclass> IntegerClass;
static jfieldID SeatZoneData_zoneId;
static jfieldID SeatZoneData_color;
static jfieldID SeatZoneData_rainbowColor;
static jmethodID SeatZoneData_constructor;
static jmethodID Integer_intValue;
static jmethodID Integer_valueOf;

void JSeatZoneData::InitJNI(JNIEnv *env) {
    SeatZoneDataClass = env->FindClass("com/libseatcanvas/SeatZoneData");
    if (SeatZoneDataClass.get() == nullptr) {
        tgfx::PrintError("Could not run JSeatZoneData::InitJNI, SeatZoneDataClass is not found!");
        return;
    }
    IntegerClass = env->FindClass("java/lang/Integer");
    if (IntegerClass.get() == nullptr) {
        tgfx::PrintError("Could not find java/lang/Integer in JSeatZoneData::InitJNI!");
        return;
    }
    SeatZoneData_zoneId = env->GetFieldID(SeatZoneDataClass.get(), "zoneId", "Ljava/lang/String;");
    SeatZoneData_color =
        env->GetFieldID(SeatZoneDataClass.get(), "color", "Ljava/lang/Integer;");
    SeatZoneData_rainbowColor =
        env->GetFieldID(SeatZoneDataClass.get(), "rainbowColor", "Ljava/lang/Integer;");
    SeatZoneData_constructor = env->GetMethodID(
        SeatZoneDataClass.get(), "<init>", "(Ljava/lang/String;Ljava/lang/Integer;Ljava/lang/Integer;)V");
    if (SeatZoneData_constructor == nullptr) {
        tgfx::PrintError("Could not get SeatZoneData constructor!");
    }
    Integer_intValue = env->GetMethodID(IntegerClass.get(), "intValue", "()I");
    Integer_valueOf = env->GetStaticMethodID(IntegerClass.get(), "valueOf", "(I)Ljava/lang/Integer;");
    if (Integer_intValue == nullptr || Integer_valueOf == nullptr) {
        tgfx::PrintError("Could not get Integer intValue or valueOf in JSeatZoneData::InitJNI!");
    }
}

std::optional<kk::SeatZoneData> JSeatZoneData::FromJava(JNIEnv *env, jobject object) {
    auto clazz = SeatZoneDataClass.get();
    if (env == nullptr || object == nullptr || clazz == nullptr) {
        return std::nullopt;
    }
    auto jzoneId = static_cast<jstring>(env->GetObjectField(object, SeatZoneData_zoneId));
    auto zoneId = SafeConvertToStdString(env, jzoneId);

    std::optional<tgfx::Color> color = std::nullopt;
    auto colorObj = env->GetObjectField(object, SeatZoneData_color);
    if (colorObj != nullptr && Integer_intValue != nullptr) {
        auto argb = static_cast<uint32_t>(env->CallIntMethod(colorObj, Integer_intValue));
        color = ColorFromARGBInt(argb);
    }

    std::optional<tgfx::Color> rainbowColor = std::nullopt;
    auto rainbowColorObj = env->GetObjectField(object, SeatZoneData_rainbowColor);
    if (rainbowColorObj != nullptr && Integer_intValue != nullptr) {
        auto argb = static_cast<uint32_t>(
            env->CallIntMethod(rainbowColorObj, Integer_intValue));
        rainbowColor = ColorFromARGBInt(argb);
    }

    return kk::SeatZoneData(zoneId, color, rainbowColor);
}

jobject JSeatZoneData::ToJava(JNIEnv *env, const kk::SeatZoneData &data) {
    auto clazz = SeatZoneDataClass.get();
    if (env == nullptr || clazz == nullptr || SeatZoneData_constructor == nullptr) {
        return nullptr;
    }
    auto jzoneId = SafeConvertToJString(env, data.zoneId);

    jobject jcolor = nullptr;
    if (data.color.has_value() && Integer_valueOf != nullptr) {
        jcolor = env->CallStaticObjectMethod(IntegerClass.get(), Integer_valueOf,
                                             ColorToARGBInt(data.color.value()));
    }

    jobject jrainbowColor = nullptr;
    if (data.rainbowColor.has_value() && Integer_valueOf != nullptr) {
        jrainbowColor = env->CallStaticObjectMethod(
            IntegerClass.get(), Integer_valueOf,
            ColorToARGBInt(data.rainbowColor.value()));
    }

    jobject result =
        env->NewObject(clazz, SeatZoneData_constructor, jzoneId, jcolor, jrainbowColor);

    if (jzoneId != nullptr) {
        env->DeleteLocalRef(jzoneId);
    }
    if (jcolor != nullptr) {
        env->DeleteLocalRef(jcolor);
    }
    if (jrainbowColor != nullptr) {
        env->DeleteLocalRef(jrainbowColor);
    }
    return result;
}

}  // namespace kk::jni
