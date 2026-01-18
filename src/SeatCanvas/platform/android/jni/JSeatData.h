//
//  JSeatData.h
//  SeatCanvas
//

#ifndef JSeatData_h
#define JSeatData_h

#include "core/SeatData.hpp"

#include <jni.h>
#include <optional>

namespace kk::jni {

class JSeatData {
  public:
    static void InitJNI(JNIEnv *env);
    static std::optional<kk::SeatData> FromJava(JNIEnv *env, jobject object);
    static jobject ToJava(JNIEnv *env, const kk::SeatData &data);
};

}  // namespace kk::jni

#endif  // JSeatData_h
