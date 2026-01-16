//
//  JStringUtil.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#ifndef JStringUtil_hpp
#define JStringUtil_hpp

#include <jni.h>
#include <string>

namespace kk::jni {

jstring SafeConvertToJString(JNIEnv *env, const std::string &text);
std::string SafeConvertToStdString(JNIEnv *env, jstring jText);

}  // namespace kk::jni

#endif /* JStringUtil_hpp */
