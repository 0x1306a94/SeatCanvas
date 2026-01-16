//
//  JHitTestSeatRegionResult.h
//  SeatCanvas
//
//  Created by KK on 2025/12/5.
//

#ifndef JHitTestSeatRegionResult_h
#define JHitTestSeatRegionResult_h

#include <jni.h>
#include <optional>
#include <string>

#include "core/RegionInfo.hpp"

namespace kk::jni {

class JHitTestSeatRegionResult {
  public:
    static void InitJNI(JNIEnv *env);
    static jobject ToJava(JNIEnv *env, const kk::RegionInfo *regionInfo);
    static std::optional<kk::RegionInfo> FromJava(JNIEnv *env, jobject object);
};

};  // namespace kk::jni

#endif  // JHitTestSeatRegionResult_h
