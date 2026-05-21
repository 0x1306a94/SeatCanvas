//
//  JSeatData.h
//  SeatCanvas
//

#ifndef JSeatData_h
#define JSeatData_h

#include "core/SeatData.hpp"

#include <functional>
#include <jni.h>
#include <optional>
#include <string>

namespace kk::jni {

using PricecodeIndexResolver = std::function<uint16_t(const std::string &)>;
using PricecodeStringResolver = std::function<std::string(uint16_t)>;

class JSeatData {
  public:
    static void InitJNI(JNIEnv *env);
    static std::optional<kk::SeatData> FromJava(JNIEnv *env, jobject object, const PricecodeIndexResolver &resolver = nullptr);
    static jobject ToJava(JNIEnv *env, const kk::SeatData &data, const PricecodeStringResolver &resolver = nullptr);
};

}  // namespace kk::jni

#endif  // JSeatData_h
