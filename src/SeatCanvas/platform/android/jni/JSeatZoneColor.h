//
//  JSeatZoneColor.h
//  SeatCanvas
//

#ifndef JSeatZoneColor_h
#define JSeatZoneColor_h

#include <jni.h>
#include <optional>
#include <string>

#include <tgfx/core/Color.h>

namespace kk::jni {

class JSeatZoneColor {
  public:
    static void InitJNI(JNIEnv *env);
    static std::optional<std::string> ReadZoneIdFromJava(JNIEnv *env, jobject object);
    static std::optional<tgfx::Color> ReadAlternateColorFromJava(JNIEnv *env, jobject object);
};

}  // namespace kk::jni

#endif  // JSeatZoneColor_h
