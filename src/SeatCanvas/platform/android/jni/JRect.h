//
//  JRect.h
//  SeatCanvas
//
//  Created by KK on 2025/12/5.
//

#ifndef JRect_h
#define JRect_h

#include <tgfx/core/Rect.h>

#include <jni.h>

namespace kk::jni {

class JRect {
  public:
    static void InitJNI(JNIEnv *env);
    static tgfx::Rect FromJava(JNIEnv *env, jobject object);
    static jobject ToJava(JNIEnv *env, const tgfx::Rect &rect);
};

};  // namespace kk::jni

#endif  // JRect_h
