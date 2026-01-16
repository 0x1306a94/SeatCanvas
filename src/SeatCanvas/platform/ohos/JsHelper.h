//
//  JsHelper.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef SEATCANVASSAMPLE_JSHELPER_H
#define SEATCANVASSAMPLE_JSHELPER_H

#include <memory>
#include <napi/native_api.h>
#include <rawfile/raw_file_manager.h>
#include <string>

#include <tgfx/core/Rect.h>

#define JS_DEFAULT_METHOD_ENTRY(name, func) \
    { #name, nullptr, func, nullptr, nullptr, nullptr, napi_default, nullptr }
#define JS_STATIC_METHOD_ENTRY(name, func) \
    { #name, nullptr, func, nullptr, nullptr, nullptr, napi_static, nullptr }

namespace tgfx {
class Data;
};

namespace kk::js {
napi_status DefineClass(napi_env env, napi_value exports, const std::string &utf8name, size_t propertyCount, const napi_property_descriptor *properties, napi_callback constructor,
                        const std::string &parentName);

napi_value GetConstructor(napi_env env, const std::string &name);

napi_value NewInstance(napi_env env, const std::string &name, void *handler);

napi_value CreateRect(napi_env env, const tgfx::Rect &rect);

tgfx::Rect GetRect(napi_env env, napi_value value);

std::shared_ptr<tgfx::Data> LoadDataFromAsset(NativeResourceManager *mNativeResMgr, const char *name);
}  // namespace kk::js

#endif  // SEATCANVASSAMPLE_JSHELPER_H
