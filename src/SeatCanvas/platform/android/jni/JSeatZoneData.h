//
//  JSeatZoneData.h
//  SeatCanvas
//

#ifndef JSeatZoneData_h
#define JSeatZoneData_h

#include "core/SeatZoneData.hpp"

#include <jni.h>
#include <optional>

namespace kk::jni {

class JSeatZoneData {
  public:
    static void InitJNI(JNIEnv *env);
    static std::optional<kk::SeatZoneData> FromJava(JNIEnv *env, jobject object);
    static jobject ToJava(JNIEnv *env, const kk::SeatZoneData &data);
};

}  // namespace kk::jni

#endif  // JSeatZoneData_h
