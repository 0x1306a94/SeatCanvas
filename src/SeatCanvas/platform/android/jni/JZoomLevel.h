//
//  JZoomLevel.h
//  SeatCanvas
//
//  Created by KK on 2026/2/10.
//

#ifndef JZoomLevel_h
#define JZoomLevel_h

#include <jni.h>

#include "core/ZoomLevelConfig.hpp"

namespace kk::jni {

class JZoomLevel {
  public:
    static void InitJNI(JNIEnv *env);
    static jobject ToJava(JNIEnv *env, const kk::ZoomLevelConfig &config);
};

};  // namespace kk::jni

#endif  // JZoomLevel_h
