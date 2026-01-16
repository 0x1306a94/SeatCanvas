//
//  FontConfigAndroid.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#include "FontConfigAndroid.hpp"

#include "JNIHelper.hpp"

#include <string>

#include <tgfx/platform/Print.h>

namespace kk::jni {
static Global<jclass> FontClass;
static jmethodID Font_RegisterFallbackFonts;

void FontConfigAndroid::InitJNI(JNIEnv *env) {
    FontClass = env->FindClass("com/libseatcanvas/Font");
    if (FontClass.get() == nullptr) {
        tgfx::PrintError("Could not run Font.RegisterFallbackFonts(), class is not found!");
        return;
    }
    Font_RegisterFallbackFonts = env->GetStaticMethodID(FontClass.get(), "RegisterFallbackFonts", "()V");
}

bool FontConfigAndroid::RegisterFallbackFonts() {
    JNIEnvironment environment;
    auto env = environment.current();
    if (env == nullptr) {
        return false;
    }
    if (FontClass.get() == nullptr) {
        tgfx::PrintError("FontClass is null");
        return false;
    }
    env->CallStaticVoidMethod(FontClass.get(), Font_RegisterFallbackFonts);
    return true;
}

}  // namespace kk::jni
