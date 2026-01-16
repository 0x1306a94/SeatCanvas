//
//  JStringUtil.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#include "JStringUtil.hpp"

#include "JNIHelper.hpp"

#include <cstring>

#include <tgfx/platform/android/Global.h>

namespace kk::jni {

jstring SafeConvertToJString(JNIEnv *env, const std::string &text) {
    static Global<jclass> StringClass = env->FindClass("java/lang/String");
    static jmethodID StringConstructID = env->GetMethodID(StringClass.get(), "<init>", "([BLjava/lang/String;)V");
    auto array = env->NewByteArray(text.size());
    env->SetByteArrayRegion(array, 0, text.size(), reinterpret_cast<const jbyte *>(text.data()));
    auto stringUTF = env->NewStringUTF("UTF-8");
    auto result = (jstring)env->NewObject(StringClass.get(), StringConstructID, array, stringUTF);
    env->DeleteLocalRef(array);
    env->DeleteLocalRef(stringUTF);
    return result;
}

std::string SafeConvertToStdString(JNIEnv *env, jstring jText) {
    if (jText == nullptr) {
        return "";
    }
    std::string result;
    static Global<jclass> StringClass = env->FindClass("java/lang/String");
    static jmethodID GetBytesID = env->GetMethodID(StringClass.get(), "getBytes", "(Ljava/lang/String;)[B");
    auto encoding = env->NewStringUTF("utf-8");
    auto jBytes = (jbyteArray)env->CallObjectMethod(jText, GetBytesID, encoding);
    env->DeleteLocalRef(encoding);
    auto textLength = env->GetArrayLength(jBytes);
    if (textLength > 0) {
        char *bytes = new char[textLength];
        env->GetByteArrayRegion(jBytes, 0, textLength, (jbyte *)bytes);
        result = std::string(bytes, (unsigned)textLength);
        delete[] bytes;
    }
    env->DeleteLocalRef(jBytes);
    return result;
}

}  // namespace kk::jni