//
//  JFont.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "JFont.h"

#include "core/FontManager.hpp"
#include "platform/ohos/JsHelper.h"

#include <tgfx/core/Data.h>
#include <tgfx/layers/TextLayer.h>
#include <tgfx/platform/Print.h>

#include <rawfile/raw_file_manager.h>

#define JFONT_FONT_FAMILY "fontFamily"
#define JFONT_FONT_STYLE "fontStyle"

namespace kk::js {

static napi_value RegisterFontFromPath(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    char fontPath[1024];
    size_t pathLength = 0;
    napi_get_value_string_utf8(env, args[0], fontPath, 1024, &pathLength);

    int index = 0;
    if (argc > 1) {
        napi_get_value_int32(env, args[1], &index);
    }

    char fontFamily[1024];
    size_t familyLength = 0;
    if (argc > 2) {
        napi_get_value_string_utf8(env, args[2], fontFamily, 1024, &familyLength);
    }

    char fontStyle[1024];
    size_t styleLength = 0;
    if (argc > 3) {
        napi_get_value_string_utf8(env, args[3], fontStyle, 1024, &styleLength);
    }

    auto font = kk::Font::RegisterFont({fontPath, pathLength}, index, {fontFamily, familyLength}, {fontStyle, styleLength});
    return JFont::ToJs(env, font);
}

static napi_value RegisterFontFromAsset(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    NativeResourceManager *mNativeResMgr = OH_ResourceManager_InitNativeResourceManager(env, args[0]);

    size_t nameLength;
    char name[1024];
    napi_get_value_string_utf8(env, args[1], name, sizeof(name), &nameLength);
    RawFile *rawFile = OH_ResourceManager_OpenRawFile(mNativeResMgr, name);
    if (rawFile == NULL) {
        return nullptr;
    }
    auto data = LoadDataFromAsset(mNativeResMgr, name);
    if (data == nullptr) {
        return nullptr;
    }

    int index = 0;
    if (argc > 2) {
        napi_get_value_int32(env, args[2], &index);
    }

    char fontFamily[1024];
    size_t familyLength = 0;
    if (argc > 3) {
        napi_get_value_string_utf8(env, args[3], fontFamily, 1024, &familyLength);
    }

    char fontStyle[1024];
    size_t styleLength = 0;
    if (argc > 4) {
        napi_get_value_string_utf8(env, args[4], fontStyle, 1024, &styleLength);
    }

    auto font = kk::Font::RegisterFont(data->data(), data->size(), index, {fontFamily, familyLength}, {fontStyle, styleLength});
    return JFont::ToJs(env, font);
}

static napi_value UnregisterFont(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    kk::Font::UnregisterFont(JFont::FromJs(env, args[0]));
    return nullptr;
}

static napi_value SetFallbackFontPaths(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    uint32_t length = 0;
    napi_get_array_length(env, args[0], &length);
    std::vector<std::string> fontPaths;
    std::vector<int> ttcIndices;
    for (uint32_t i = 0; i < length; i++) {
        napi_value pathValue;
        napi_get_element(env, args[0], i, &pathValue);
        char path[1024];
        size_t pathLength = 0;
        napi_get_value_string_utf8(env, pathValue, path, 1024, &pathLength);
        fontPaths.push_back({path, pathLength});
        ttcIndices.push_back(0);
    }
    kk::Font::SetFallbackFontPaths(fontPaths, ttcIndices);

    // 使用 GetFallbackTypefacesDirect() 避免静态初始化循环依赖
    auto fallbackTypefaces = kk::FontManager::GetFallbackTypefacesDirect();
    tgfx::TextLayer::SetFallbackTypefaces(std::move(fallbackTypefaces));
    return nullptr;
}

bool JFont::Init(napi_env env, napi_value exports) {
    napi_property_descriptor classProp[] = {
        JS_STATIC_METHOD_ENTRY(RegisterFontFromPath, RegisterFontFromPath),
        JS_STATIC_METHOD_ENTRY(RegisterFontFromAsset, RegisterFontFromAsset),
        JS_STATIC_METHOD_ENTRY(UnregisterFont, UnregisterFont),
        JS_STATIC_METHOD_ENTRY(SetFallbackFontPaths, SetFallbackFontPaths),
    };
    auto status = DefineClass(env, exports, ClassName(), sizeof(classProp) / sizeof(classProp[0]), classProp, Constructor, "");
    return status == napi_ok;
}

napi_value JFont::ToJs(napi_env env, const kk::Font &font) {
    napi_value fontFamily;
    napi_create_string_utf8(env, font.fontFamily.c_str(), font.fontFamily.length(), &fontFamily);

    napi_value fontStyle;
    napi_create_string_utf8(env, font.fontStyle.c_str(), font.fontStyle.length(), &fontStyle);

    napi_value arg[] = {fontFamily, fontStyle};

    napi_value result;
    auto status = napi_new_instance(env, GetConstructor(env, ClassName()), 2, arg, &result);
    if (status != napi_ok) {
        tgfx::PrintError("JPAGFont::ToJs napi_new_instance failed :%d", status);
        return nullptr;
    }
    return result;
}

kk::Font JFont::FromJs(napi_env env, napi_value value) {
    napi_value fontFamily;
    napi_get_named_property(env, value, JFONT_FONT_FAMILY, &fontFamily);
    char fontFamilyBuf[1024];
    size_t familyLength = 0;
    napi_get_value_string_utf8(env, fontFamily, fontFamilyBuf, 1024, &familyLength);

    napi_value fontStyle;
    napi_get_named_property(env, value, JFONT_FONT_STYLE, &fontStyle);
    char fontStyleBuf[1024];
    size_t styleLength = 0;
    napi_get_value_string_utf8(env, fontStyle, fontStyleBuf, 1024, &styleLength);

    return kk::Font{{fontFamilyBuf, familyLength}, {fontStyleBuf, styleLength}};
}

napi_value JFont::Constructor(napi_env env, napi_callback_info info) {
    napi_value result = nullptr;
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, &result, nullptr);
    napi_set_named_property(env, result, JFONT_FONT_FAMILY, args[0]);
    napi_set_named_property(env, result, JFONT_FONT_STYLE, args[1]);
    return result;
}
}  // namespace kk::js