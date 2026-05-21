//
//  JsHelper.h
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#ifndef SEATCANVASSAMPLE_JSHELPER_H
#define SEATCANVASSAMPLE_JSHELPER_H

#include <cstdint>
#include <functional>
#include <memory>
#include <napi/native_api.h>
#include <optional>
#include <rawfile/raw_file_manager.h>
#include <string>

#include <tgfx/core/Color.h>
#include <tgfx/core/Rect.h>

#include "core/SeatData.hpp"

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

std::string GetUtf8String(napi_env env, napi_value value);
std::string ReadString(napi_env env, napi_value obj, const char *key);
std::optional<std::string> ReadOptionalString(napi_env env, napi_value obj, const char *key);
std::optional<tgfx::Color> ReadOptionalColorFromARGBHex(napi_env env, napi_value obj, const char *key);
int32_t ReadInt32(napi_env env, napi_value obj, const char *key);
uint32_t ReadUInt32(napi_env env, napi_value obj, const char *key);
double ReadDouble(napi_env env, napi_value obj, const char *key);
double ReadDouble(napi_env env, napi_value obj, const char *key);
bool ReadBoolean(napi_env env, napi_value obj, const char *key);

napi_value CreateRect(napi_env env, const tgfx::Rect &rect);

tgfx::Rect GetRect(napi_env env, napi_value value);

napi_value CreateZoomLevel(napi_env env, float seat, float row, float zone, float venue);

using PricecodeIndexResolver = std::function<uint16_t(const std::string &)>;

std::optional<kk::SeatData> GetSeatData(napi_env env, napi_value value, const PricecodeIndexResolver &resolver = nullptr);

std::shared_ptr<tgfx::Data> LoadDataFromAsset(NativeResourceManager *mNativeResMgr, const char *name);
}  // namespace kk::js

#endif  // SEATCANVASSAMPLE_JSHELPER_H
