//
//  JNIHelper.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#ifndef JNIHelper_hpp
#define JNIHelper_hpp

#include <jni.h>
#include <tgfx/platform/android/Global.h>

namespace kk::jni {
template <class T>
using Global = tgfx::Global<T>;

using JNIEnvironment = tgfx::JNIEnvironment;

}  // namespace kk::jni

#endif /* JNIHelper_hpp */
