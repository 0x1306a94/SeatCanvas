//
//  JNIHelper.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#ifndef JNIHelper_hpp
#define JNIHelper_hpp

#include <jni.h>
#include <string>

namespace kk::jni {
class JNIHelper {
  public:
    static void InitJNI(JNIEnv *env);
    static bool RegisterFallbackFonts();
};

}  // namespace kk::jni

#endif /* JNIHelper_hpp */
