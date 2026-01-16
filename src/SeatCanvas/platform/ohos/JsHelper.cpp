//
//  JsHelper.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "JsHelper.h"

#include <tgfx/core/Data.h>
#include <tgfx/platform/Print.h>

#include <unordered_map>

namespace kk::js {

static std::unordered_map<std::string, napi_ref> ConstructorRefMap;

bool SetConstructor(napi_env env, napi_value constructor, const std::string &name) {
    if (env == nullptr || constructor == nullptr || name.empty()) {
        return false;
    }
    if (ConstructorRefMap.find(name) != ConstructorRefMap.end()) {
        napi_delete_reference(env, ConstructorRefMap.at(name));
        ConstructorRefMap.erase(name);
    }
    napi_ref ref;
    napi_create_reference(env, constructor, 1, &ref);
    ConstructorRefMap[name] = ref;
    return true;
}

napi_value GetConstructor(napi_env env, const std::string &name) {
    if (env == nullptr || name.empty()) {
        return nullptr;
    }
    napi_value result = nullptr;
    if (ConstructorRefMap.find(name) != ConstructorRefMap.end()) {
        napi_get_reference_value(env, ConstructorRefMap.at(name), &result);
    }
    return result;
}

napi_status ExtendClass(napi_env env, napi_value constructor, const std::string &parentName) {
    if (env == nullptr || constructor == nullptr || parentName.empty()) {
        return napi_status::napi_invalid_arg;
    }
    napi_value baseConstructor = GetConstructor(env, parentName);
    if (baseConstructor == nullptr) {
        tgfx::PrintError("ExtendClass get baseConstructor failed status");
        return napi_status::napi_invalid_arg;
    }
    napi_value basePrototype;
    napi_status statusCode = napi_get_named_property(env, baseConstructor, "prototype", &basePrototype);
    if (statusCode != napi_status::napi_ok) {
        tgfx::PrintError("ExtendClass get baseConstructor's prototype  failed status :%d", statusCode);
        return statusCode;
    }
    napi_value derivedPrototype;
    statusCode = napi_get_named_property(env, constructor, "prototype", &derivedPrototype);
    if (statusCode != napi_status::napi_ok) {
        tgfx::PrintError("ExtendClass get constructor's prototype  failed status :%d", statusCode);
        return statusCode;
    }
    return napi_set_named_property(env, derivedPrototype, "__proto__", basePrototype);
}

napi_status DefineClass(napi_env env, napi_value exports, const std::string &utf8name, size_t propertyCount, const napi_property_descriptor *properties, napi_callback constructor,
                        const std::string &parentName) {
    napi_value classConstructor;
    auto status = napi_define_class(env, utf8name.c_str(), utf8name.length(), constructor, nullptr, propertyCount, properties, &classConstructor);
    if (status != napi_status::napi_ok) {
        tgfx::PrintError("DefineClass napi_define_class failed:%d", status);
        return status;
    }
    status = napi_set_named_property(env, exports, utf8name.c_str(), classConstructor);
    if (status != napi_status::napi_ok) {
        tgfx::PrintError("DefineClass napi_set_named_property failed:%d", status);
        return status;
    }
    SetConstructor(env, classConstructor, utf8name);
    if (parentName.empty()) {
        return status;
    }
    status = ExtendClass(env, classConstructor, parentName);
    if (status != napi_status::napi_ok) {
        tgfx::PrintError("DefineClass ExtendClass failed:%d", status);
        return status;
    }
    return status;
}

napi_value NewInstance(napi_env env, const std::string &name, void *handler) {
    if (env == nullptr || handler == nullptr || name.empty()) {
        return nullptr;
    }
    napi_value constructor = GetConstructor(env, name);
    if (constructor == nullptr) {
        return nullptr;
    }
    napi_value result = nullptr;
    napi_value external[1];
    auto status = napi_create_external(env, handler, nullptr, nullptr, &external[0]);
    if (status != napi_ok) {
        tgfx::PrintError("NewInstance napi_create_external failed :%d", status);
        return nullptr;
    }
    status = napi_new_instance(env, constructor, 1, external, &result);
    if (status != napi_ok) {
        tgfx::PrintError("NewInstance napi_new_instance failed :%d", status);
        return nullptr;
    }
    return result;
}

napi_value CreateRect(napi_env env, const tgfx::Rect &rect) {
    napi_value jsRect;
    napi_create_object(env, &jsRect);

    napi_value v;
    napi_create_double(env, rect.x(), &v);
    napi_set_named_property(env, jsRect, "x", v);
    napi_create_double(env, rect.y(), &v);
    napi_set_named_property(env, jsRect, "y", v);
    napi_create_double(env, rect.width(), &v);
    napi_set_named_property(env, jsRect, "width", v);
    napi_create_double(env, rect.height(), &v);
    napi_set_named_property(env, jsRect, "height", v);
    return jsRect;
}

tgfx::Rect GetRect(napi_env env, napi_value value) {
    if (env == nullptr || value == nullptr) {
        return tgfx::Rect::MakeEmpty();
    }
    napi_value xValue, yValue, widthValue, heightValue;

    napi_get_named_property(env, value, "x", &xValue);
    napi_get_named_property(env, value, "y", &yValue);
    napi_get_named_property(env, value, "width", &widthValue);
    napi_get_named_property(env, value, "height", &heightValue);

    double x, y, width, height;
    napi_get_value_double(env, xValue, &x);
    napi_get_value_double(env, yValue, &y);
    napi_get_value_double(env, widthValue, &width);
    napi_get_value_double(env, heightValue, &height);

    auto rect = tgfx::Rect::MakeXYWH(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));
    return rect;
}

std::shared_ptr<tgfx::Data> LoadDataFromAsset(NativeResourceManager *mNativeResMgr, const char *name) {
    RawFile *rawFile = OH_ResourceManager_OpenRawFile(mNativeResMgr, name);
    if (rawFile == NULL) {
        return nullptr;
    }
    auto len = OH_ResourceManager_GetRawFileSize(rawFile);
    auto buffer = malloc(static_cast<size_t>(len));
    if (buffer == nullptr) {
        OH_ResourceManager_CloseRawFile(rawFile);
        return nullptr;
    }
    OH_ResourceManager_ReadRawFile(rawFile, buffer, static_cast<size_t>(len));
    OH_ResourceManager_CloseRawFile(rawFile);
    auto data = tgfx::Data::MakeAdopted(buffer, static_cast<size_t>(len), tgfx::Data::FreeProc);
    return data;
}
}  // namespace kk::js