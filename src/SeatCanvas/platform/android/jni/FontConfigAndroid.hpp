//
//  FontConfigAndroid.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#ifndef FontConfigAndroid_hpp
#define FontConfigAndroid_hpp

#include <jni.h>

namespace kk::jni {
class FontConfigAndroid {
  public:
    static void InitJNI(JNIEnv *env);
    static bool RegisterFallbackFonts();
};

}  // namespace kk::jni

#endif /* FontConfigAndroid_hpp */
