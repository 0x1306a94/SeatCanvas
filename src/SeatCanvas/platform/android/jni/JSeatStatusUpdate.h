//
//  JSeatStatusUpdate.h
//  SeatCanvas
//

#ifndef JSeatStatusUpdate_h
#define JSeatStatusUpdate_h

#include "core/SeatData.hpp"

#include <jni.h>
#include <vector>

namespace kk::jni {

class JSeatStatusUpdate {
  public:
    static void InitJNI(JNIEnv *env);
    static std::vector<kk::SeatStatusUpdate> FromJavaArray(JNIEnv *env, jobjectArray array);
};

}  // namespace kk::jni

#endif  // JSeatStatusUpdate_h
