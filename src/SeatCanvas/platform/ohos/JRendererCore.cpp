//
//  JRendererCore.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "JRendererCore.h"

#include "JsHelper.h"
#include "NapiEnvHolder.hpp"
#include "OHOSPlatformView.h"
#include "OHOSSeatCanvasCoreRendererDelegate.hpp"
#include "core/BaseMapConfig.hpp"
#include "core/Platform.hpp"
#include "core/SeatData.hpp"
#include "core/UniqueID.h"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/parser/BaseMapFormat.hpp"
#include "core/parser/BaseMapLoadResult.hpp"
#include "core/parser/BaseMapParserFactory.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/svg/ConvertSVGLayer.hpp"
#include "core/svg/SVGMeshParser.hpp"
#include "core/utils/ColorIntConverter.hpp"
#include "core/utils/SystemProperties.hpp"
#include "core/utils/TimeProfiler.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>

namespace kk::js {

static std::unordered_map<std::string, std::shared_ptr<JRendererCore>> ViewMap = {};

struct LoadBaseMapTaskData {
    napi_async_work asyncWork{nullptr};
    napi_deferred deferred{nullptr};
    napi_ref callback{nullptr};
    NativeResourceManager *mNativeResMgr{nullptr};
    std::string filename{""};
    kk::parser::BaseMapFormat format{kk::parser::BaseMapFormat::Unknown};
    std::string parseConfigJSON{""};
    kk::parser::BaseMapLoadResult *result{nullptr};
};

static void LoadBaseMapTaskExecute(napi_env env, void *userdata) {
    auto taskData = static_cast<LoadBaseMapTaskData *>(userdata);

    auto data = LoadDataFromAsset(taskData->mNativeResMgr, taskData->filename.c_str());
    if (data == nullptr) {
        tgfx::PrintError("data is null");
        return;
    }
    if (taskData->format == kk::parser::BaseMapFormat::Unknown) {
        tgfx::PrintError("format is Unknown");
        return;
    }

    PROFILE_TIME(std::string("LoadBaseMapFrom") + kk::parser::formatNameToString(taskData->format));
    std::shared_ptr<tgfx::Data> parseConfigData = nullptr;
    if (!taskData->parseConfigJSON.empty()) {
        parseConfigData = tgfx::Data::MakeWithCopy(taskData->parseConfigJSON.data(), taskData->parseConfigJSON.length());
    }
    auto result = kk::parser::BaseMapParserFactory::parse(data, taskData->format, parseConfigData);
    if (!result) {
        tgfx::PrintError("parse result is null");
        return;
    }

    auto taskResult = new kk::parser::BaseMapLoadResult(std::move(result));
    taskData->result = taskResult;
}

static void LoadBaseMapTaskComplete(napi_env env, napi_status status, void *userdata) {
    auto taskData = static_cast<LoadBaseMapTaskData *>(userdata);
    napi_value result = nullptr;
    napi_create_int64(env, reinterpret_cast<int64_t>(taskData->result), &result);
    if (taskData->result != nullptr) {
        napi_resolve_deferred(env, taskData->deferred, result);
    } else {
        napi_reject_deferred(env, taskData->deferred, result);
    }

    napi_delete_async_work(env, taskData->asyncWork);
    delete taskData;
}

static napi_value InitSystemProperties(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    double density;
    napi_get_value_double(env, args[0], &density);
    double fontScale;
    napi_get_value_double(env, args[1], &fontScale);
    kk::renderer::OHOSPlatformView::UpdateDensity(static_cast<float>(density));

    auto &properties = kk::utils::SystemProperties::Instance();
    properties.updateDensity(static_cast<float>(density));
    properties.updateFontScale(static_cast<float>(fontScale));
    return nullptr;
}

static napi_value ParseBaseMapFromAssets(napi_env env, napi_callback_info info) {
    napi_value jsView = nullptr;
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);

    NativeResourceManager *mNativeResMgr = OH_ResourceManager_InitNativeResourceManager(env, args[0]);
    if (mNativeResMgr == nullptr) {
        return nullptr;
    }

    size_t nameLength;
    char name[1024];
    napi_get_value_string_utf8(env, args[1], name, sizeof(name), &nameLength);

    size_t formatLength;
    char format[128];
    napi_get_value_string_utf8(env, args[2], format, sizeof(format), &formatLength);

    std::string parseConfigJSON = "";
    if (argc >= 4 && args[3] != nullptr) {
        parseConfigJSON = GetUtf8String(env, args[3]);
    }

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);

    auto taskData = new LoadBaseMapTaskData();
    taskData->deferred = deferred;
    taskData->mNativeResMgr = mNativeResMgr;
    taskData->filename = std::string(name);
    taskData->format = kk::parser::parseFormatName(format);
    taskData->parseConfigJSON = parseConfigJSON;

    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "LoadBaseMapTask", NAPI_AUTO_LENGTH, &resourceName);
    // 创建异步任务
    napi_create_async_work(env, nullptr, resourceName, LoadBaseMapTaskExecute, LoadBaseMapTaskComplete, taskData, &taskData->asyncWork);
    // 将异步任务加入队列
    napi_queue_async_work(env, taskData->asyncWork);

    return promise;
}

static napi_value UniqueID(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    napi_value result;
    napi_create_string_utf8(env, view->id.c_str(), view->id.length(), &result);
    return result;
}

static napi_value LoadBaseMap(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    int64_t nativePtr = 0;
    napi_get_value_int64(env, args[0], &nativePtr);

    auto renderer = view->internalRenderer();
    auto map = reinterpret_cast<kk::parser::BaseMapLoadResult *>(nativePtr);
    if (map == nullptr) {
        renderer->setBaseMapConfig(nullptr);
        return nullptr;
    }

    auto baseMapConfig = map->makeBaseMapConfig();
    renderer->setBaseMapConfig(std::move(baseMapConfig));

    delete map;

    return nullptr;
}

static napi_value ApplySeatStyleJSONConfig(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (argc == 0 || args[0] == nullptr) {
        renderer->setStyleKeyToConfigFromJSON(nullptr, 0);
        return nullptr;
    }

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, args[0], nullptr, 0, &len);
    if (status != napi_ok || len == 0) {
        renderer->setStyleKeyToConfigFromJSON(nullptr, 0);
        return nullptr;
    }

    std::vector<char> buffer(len + 1);
    size_t actualLen = 0;
    status = napi_get_value_string_utf8(env, args[0], buffer.data(), len + 1, &actualLen);
    if (status != napi_ok) {
        renderer->setStyleKeyToConfigFromJSON(nullptr, 0);
        return nullptr;
    }

    renderer->setStyleKeyToConfigFromJSON(buffer.data(), actualLen);
    return nullptr;
}

static napi_value Start(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }
    view->start();
    return nullptr;
}

static napi_value Stop(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }
    view->stop();
    return nullptr;
}

static napi_value SetCanvasColor(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 1) {
        return nullptr;
    }
    int32_t colorInt = 0;
    napi_get_value_int32(env, args[0], &colorInt);
    view->setBackgroundColor(kk::utils::ColorFromARGBInt(static_cast<uint32_t>(colorInt)));
    return nullptr;
}

static napi_value GetCanvasColor(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        napi_value def = nullptr;
        napi_create_int32(env, 0xFFFFFFFF, &def);
        return def;
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        napi_value def = nullptr;
        napi_create_int32(env, 0xFFFFFFFF, &def);
        return def;
    }
    napi_value result = nullptr;
    napi_create_int32(env, kk::utils::ColorToARGBInt(renderer->getBackgroundColor()), &result);
    return result;
}

static napi_value GetSeatSize(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 36.0, &def);
        return def;
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 36.0, &def);
        return def;
    }
    napi_value result = nullptr;
    napi_create_double(env, renderer->getSeatSize(), &result);
    return result;
}

static napi_value GetMinimumZoomScale(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 1.0, &def);
        return def;
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 1.0, &def);
        return def;
    }
    napi_value result = nullptr;
    napi_create_double(env, renderer->getMinimumZoomScale(), &result);
    return result;
}

static napi_value GetMaximumZoomScale(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 1.0, &def);
        return def;
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 1.0, &def);
        return def;
    }
    napi_value result = nullptr;
    napi_create_double(env, renderer->getMaximumZoomScale(), &result);
    return result;
}

static napi_value GetZoomScale(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 1.0, &def);
        return def;
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 1.0, &def);
        return def;
    }
    napi_value result = nullptr;
    napi_create_double(env, renderer->getZoomScale(), &result);
    return result;
}

static napi_value GetVisibleOriginalRect(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return CreateRect(env, tgfx::Rect::MakeEmpty());
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return CreateRect(env, tgfx::Rect::MakeEmpty());
    }
    return CreateRect(env, renderer->getVisibleOriginalRect());
}

static napi_value GetZoneIdsInOriginalRect(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);

    napi_value emptyArray = nullptr;
    napi_create_array(env, &emptyArray);

    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 1) {
        return emptyArray;
    }

    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return emptyArray;
    }

    auto queryRect = GetRect(env, args[0]);
    auto zoneIds = renderer->getZoneIdsInOriginalRect(queryRect);

    napi_value result = nullptr;
    napi_create_array_with_length(env, zoneIds.size(), &result);
    if (result == nullptr) {
        return emptyArray;
    }

    for (size_t index = 0; index < zoneIds.size(); ++index) {
        napi_value zoneId = nullptr;
        napi_create_string_utf8(env, zoneIds[index].c_str(), NAPI_AUTO_LENGTH, &zoneId);
        napi_set_element(env, result, index, zoneId);
    }
    return result;
}

static napi_value SetSeatSize(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 1) {
        return nullptr;
    }
    double seatSize = 36.0;
    napi_get_value_double(env, args[0], &seatSize);
    auto *renderer = view->internalRenderer();
    if (renderer != nullptr) {
        renderer->setSeatSize(static_cast<float>(seatSize));
    }
    return nullptr;
}

static napi_value GetZoomLevel(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return CreateZoomLevel(env, 1.0, 1.0, 1.0, 1.0);
    }

    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return CreateZoomLevel(env, 1.0, 1.0, 1.0, 1.0);
    }

    const auto &result = renderer->zoomLevelConfig();
    return CreateZoomLevel(env, result.seat, result.row, result.zone, result.venue);
}

static napi_value GetSeatRenderZoomThreshold(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 0.0, &def);
        return def;
    }
    auto *renderer = view->internalRenderer();
    if (renderer == nullptr) {
        napi_value def = nullptr;
        napi_create_double(env, 0.0, &def);
        return def;
    }
    napi_value result = nullptr;
    napi_create_double(env, renderer->getSeatRenderZoomThreshold(), &result);
    return result;
}

static napi_value SetSeatRenderZoomThreshold(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 1) {
        return nullptr;
    }
    double threshold = 0.0;
    napi_get_value_double(env, args[0], &threshold);
    auto *renderer = view->internalRenderer();
    if (renderer != nullptr) {
        renderer->setSeatRenderZoomThreshold(static_cast<float>(threshold));
    }
    return nullptr;
}

static napi_value IsDebugHUDEnabled(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    napi_value result = nullptr;
    bool enabled = false;
    if (view != nullptr) {
        auto *renderer = view->internalRenderer();
        if (renderer != nullptr) {
            enabled = renderer->isDebugHUDEnabled();
        }
    }
    napi_get_boolean(env, enabled, &result);
    return result;
}

static napi_value SetDebugHUDEnabled(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 1) {
        return nullptr;
    }
    bool enabled = false;
    napi_get_value_bool(env, args[0], &enabled);
    auto *renderer = view->internalRenderer();
    if (renderer != nullptr) {
        renderer->setDebugHUDEnabled(enabled);
    }
    return nullptr;
}

static napi_value Release(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    //    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    // 会立即触发 napi_wrap 的 finalize_cb
    napi_remove_wrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }
    view->stop();
    return nullptr;
}

static napi_value HandleTap(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    if (argc < 2) {
        return nullptr;
    }

    double x;
    double y;
    napi_get_value_double(env, args[0], &x);
    napi_get_value_double(env, args[1], &y);

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->handleTap(tgfx::Point{static_cast<float>(x), static_cast<float>(y)});

    return nullptr;
}

static napi_value HandlePan(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 3;
    napi_value args[3] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    if (argc < 3) {
        return nullptr;
    }

    int32_t state;
    double tx;
    double ty;
    napi_get_value_int32(env, args[0], &state);
    napi_get_value_double(env, args[1], &tx);
    napi_get_value_double(env, args[2], &ty);

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    auto timestampMs = Platform::Current()->currentMediaTime();
    renderer->handlePan(static_cast<kk::gesture::GestureState>(state), tgfx::Point{static_cast<float>(tx), static_cast<float>(ty)}, timestampMs);

    return nullptr;
}

static napi_value HandlePinch(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 4;
    napi_value args[4] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    if (argc < 4) {
        return nullptr;
    }

    int32_t state;
    double scale;
    double cx;
    double cy;
    napi_get_value_int32(env, args[0], &state);
    napi_get_value_double(env, args[1], &scale);
    napi_get_value_double(env, args[2], &cx);
    napi_get_value_double(env, args[3], &cy);

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->handlePinch(static_cast<kk::gesture::GestureState>(state), static_cast<float>(scale), tgfx::Point{static_cast<float>(cx), static_cast<float>(cy)});

    return nullptr;
}

static napi_value SetDelegateCallback(napi_env env, napi_callback_info info,
                                      void (OHOSSeatCanvasCoreRendererDelegate::*setter)(napi_env, napi_value)) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    auto delegate = view->getDelegate();
    if (delegate == nullptr) {
        return nullptr;
    }

    napi_value callback = nullptr;
    if (argc > 0 && args[0] != nullptr) {
        callback = args[0];
    }

    (delegate.get()->*setter)(env, callback);
    return nullptr;
}

static napi_value SetDidLoadBaseMapCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setDidLoadBaseMapCallback);
}

static napi_value SetDidUnloadBaseMapCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setDidUnloadBaseMapCallback);
}

static napi_value SetDidUpdateZoomLevelConfigCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setDidUpdateZoomLevelConfigCallback);
}

static napi_value SetDidTapSeatCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setDidTapSeatCallback);
}

static napi_value SetDidTapZoneCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setDidTapZoneCallback);
}

static napi_value SetViewportWillBeginDraggingCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportWillBeginDraggingCallback);
}

static napi_value SetViewportDidScrollCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportDidScrollCallback);
}

static napi_value SetViewportDidEndDraggingCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndDraggingCallback);
}

static napi_value SetViewportDidEndDeceleratingCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndDeceleratingCallback);
}

static napi_value SetViewportWillBeginZoomingCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportWillBeginZoomingCallback);
}

static napi_value SetViewportDidZoomCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportDidZoomCallback);
}

static napi_value SetViewportDidEndZoomingCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndZoomingCallback);
}

static napi_value SetViewportDidEndScrollingAnimationCallback(napi_env env, napi_callback_info info) {
    return SetDelegateCallback(env, info, &OHOSSeatCanvasCoreRendererDelegate::setViewportDidEndScrollingAnimationCallback);
}

static napi_value ZoomToRect(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 4;
    napi_value args[4] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    if (argc < 4) {
        return nullptr;
    }

    auto rect = GetRect(env, args[0]);
    if (rect.isEmpty()) {
        return nullptr;
    }

    bool animated = true;
    double padding = 0.0;
    double durationMs = 300.0;

    napi_get_value_bool(env, args[1], &animated);
    napi_get_value_double(env, args[2], &padding);
    napi_get_value_double(env, args[3], &durationMs);

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->zoomToRect(rect, animated, static_cast<float>(padding), durationMs);

    return nullptr;
}

static napi_value UpdateSeatZoneAlternateColors(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc == 0) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    bool isArray = false;
    napi_is_array(env, args[0], &isArray);
    if (!isArray) {
        return nullptr;
    }

    uint32_t length = 0;
    napi_get_array_length(env, args[0], &length);

    std::unordered_map<std::string, tgfx::Color> cppColors{};
    cppColors.reserve(static_cast<size_t>(length));

    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, args[0], i, &element);

        auto zoneId = ReadString(env, element, "zoneId");
        if (zoneId.empty()) {
            continue;
        }
        auto color = ReadOptionalColorFromARGBHex(env, element, "alternateColor");
        if (!color) {
            continue;
        }
        cppColors.emplace(zoneId, color.value());
    }

    renderer->updateSeatZoneAlternateColors(cppColors);
    return nullptr;
}

static napi_value UpdateMiniMapZoneAlternateColors(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc == 0) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    bool isArray = false;
    napi_is_array(env, args[0], &isArray);
    if (!isArray) {
        return nullptr;
    }

    uint32_t length = 0;
    napi_get_array_length(env, args[0], &length);

    std::unordered_map<std::string, tgfx::Color> cppColors{};
    cppColors.reserve(static_cast<size_t>(length));

    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, args[0], i, &element);

        auto zoneId = ReadString(env, element, "zoneId");
        if (zoneId.empty()) {
            continue;
        }
        auto color = ReadOptionalColorFromARGBHex(env, element, "alternateColor");
        if (!color) {
            continue;
        }
        cppColors.emplace(zoneId, color.value());
    }

    renderer->updateMiniMapZoneAlternateColors(cppColors);
    return nullptr;
}

static napi_value UpdateSeats(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 2) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    auto zoneId = GetUtf8String(env, args[0]);
    if (zoneId.empty()) {
        return nullptr;
    }

    bool isArray = false;
    napi_is_array(env, args[1], &isArray);
    if (!isArray) {
        return nullptr;
    }

    uint32_t length = 0;
    napi_get_array_length(env, args[1], &length);
    if (length == 0) {
        return nullptr;
    }
    std::vector<kk::SeatData> seats{};
    seats.reserve(static_cast<size_t>(length));
    auto pricecodeIndexResolver = [renderer](const std::string &pricecode) {
        return renderer->pricecodeIndexForCode(pricecode);
    };
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, args[1], i, &element);
        auto seatData = GetSeatData(env, element, pricecodeIndexResolver);
        if (seatData) {
            seats.push_back(seatData.value());
        }
    }

    renderer->setSeatData(zoneId, seats);
    return nullptr;
}

static std::vector<std::string> ReadStringArray(napi_env env, napi_value arrayValue) {
    std::vector<std::string> values = {};
    if (env == nullptr || arrayValue == nullptr) {
        return values;
    }
    bool isArray = false;
    napi_is_array(env, arrayValue, &isArray);
    if (!isArray) {
        return values;
    }
    uint32_t length = 0;
    napi_get_array_length(env, arrayValue, &length);
    values.reserve(length);
    for (uint32_t index = 0; index < length; ++index) {
        napi_value element = nullptr;
        napi_get_element(env, arrayValue, index, &element);
        if (element == nullptr) {
            continue;
        }
        values.push_back(GetUtf8String(env, element));
    }
    return values;
}

static napi_value RegisterPricecodes(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->registerPricecodes(ReadStringArray(env, argc > 0 ? args[0] : nullptr));
    return nullptr;
}

static napi_value UpdateSeatStatuses(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc == 0) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    bool isArray = false;
    napi_is_array(env, args[0], &isArray);
    if (!isArray) {
        return nullptr;
    }

    uint32_t length = 0;
    napi_get_array_length(env, args[0], &length);
    std::vector<kk::SeatStatusUpdate> updates = {};
    updates.reserve(length);
    for (uint32_t index = 0; index < length; ++index) {
        napi_value element = nullptr;
        napi_get_element(env, args[0], index, &element);
        if (element == nullptr) {
            continue;
        }
        kk::SeatStatusUpdate update = {};
        update.seatId = ReadString(env, element, "seatId");
        update.status = ReadUInt32(env, element, "status");
        if (!update.seatId.empty()) {
            updates.push_back(std::move(update));
        }
    }

    renderer->updateSeatStatuses(updates);
    return nullptr;
}

static napi_value UpdateSeatStatusesForZone(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 2) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    auto zoneId = GetUtf8String(env, args[0]);
    if (zoneId.empty()) {
        return nullptr;
    }

    bool isArrayBuffer = false;
    napi_is_arraybuffer(env, args[1], &isArrayBuffer);
    if (!isArrayBuffer) {
        return nullptr;
    }

    void *data = nullptr;
    size_t byteLength = 0;
    napi_get_arraybuffer_info(env, args[1], &data, &byteLength);
    if (data == nullptr || byteLength == 0) {
        return nullptr;
    }

    renderer->updateSeatStatusesForZone(zoneId, static_cast<const uint32_t *>(data),
                                        byteLength / sizeof(uint32_t));
    return nullptr;
}

static napi_value SetSelectedSeatIds(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->setSelectedSeatIds(ReadStringArray(env, argc > 0 ? args[0] : nullptr));
    return nullptr;
}

static napi_value UpdateSelectedSeatIds(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->updateSelectedSeatIds(ReadStringArray(env, argc > 0 ? args[0] : nullptr),
                                    ReadStringArray(env, argc > 1 ? args[1] : nullptr));
    return nullptr;
}

static napi_value ClearSeatData(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);

    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }

    auto renderer = view->internalRenderer();
    if (renderer == nullptr) {
        return nullptr;
    }

    renderer->clearSeatData();
    return nullptr;
}

napi_value JRendererCore::Constructor(napi_env env, napi_callback_info info) {
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    std::string id = "JRendererCore" + std::to_string(kk::UniqueID::Next());
    auto cView = std::make_shared<JRendererCore>(id);
    XComponentHandler::AddListener(id, cView);
    napi_wrap(
        env, jsView, cView.get(),
        [](napi_env, void *finalize_data, void *) {
            JRendererCore *view = static_cast<JRendererCore *>(finalize_data);
            XComponentHandler::RemoveListener(view->id);
            ViewMap.erase(view->id);
        },
        nullptr, nullptr);
    ViewMap.emplace(id, cView);
    return jsView;
}

bool JRendererCore::Init(napi_env env, napi_value exports) {
    napi_property_descriptor classProp[] = {
        JS_STATIC_METHOD_ENTRY(InitSystemProperties, InitSystemProperties),
        JS_STATIC_METHOD_ENTRY(ParseBaseMapFromAssets, ParseBaseMapFromAssets),
        JS_DEFAULT_METHOD_ENTRY(uniqueID, UniqueID),
        JS_DEFAULT_METHOD_ENTRY(loadBaseMap, LoadBaseMap),
        JS_DEFAULT_METHOD_ENTRY(applySeatStyleJSONConfig, ApplySeatStyleJSONConfig),
        JS_DEFAULT_METHOD_ENTRY(start, Start),
        JS_DEFAULT_METHOD_ENTRY(stop, Stop),
        JS_DEFAULT_METHOD_ENTRY(setCanvasColor, SetCanvasColor),
        JS_DEFAULT_METHOD_ENTRY(getCanvasColor, GetCanvasColor),
        JS_DEFAULT_METHOD_ENTRY(getSeatSize, GetSeatSize),
        JS_DEFAULT_METHOD_ENTRY(setSeatSize, SetSeatSize),
        JS_DEFAULT_METHOD_ENTRY(getMinimumZoomScale, GetMinimumZoomScale),
        JS_DEFAULT_METHOD_ENTRY(getMaximumZoomScale, GetMaximumZoomScale),
        JS_DEFAULT_METHOD_ENTRY(getZoomScale, GetZoomScale),
        JS_DEFAULT_METHOD_ENTRY(getVisibleOriginalRect, GetVisibleOriginalRect),
        JS_DEFAULT_METHOD_ENTRY(getZoneIdsInOriginalRect, GetZoneIdsInOriginalRect),
        JS_DEFAULT_METHOD_ENTRY(zoomLevel, GetZoomLevel),
        JS_DEFAULT_METHOD_ENTRY(getSeatRenderZoomThreshold, GetSeatRenderZoomThreshold),
        JS_DEFAULT_METHOD_ENTRY(setSeatRenderZoomThreshold, SetSeatRenderZoomThreshold),
        JS_DEFAULT_METHOD_ENTRY(isDebugHUDEnabled, IsDebugHUDEnabled),
        JS_DEFAULT_METHOD_ENTRY(setDebugHUDEnabled, SetDebugHUDEnabled),
        JS_DEFAULT_METHOD_ENTRY(release, Release),
        JS_DEFAULT_METHOD_ENTRY(handleTap, HandleTap),
        JS_DEFAULT_METHOD_ENTRY(handlePan, HandlePan),
        JS_DEFAULT_METHOD_ENTRY(handlePinch, HandlePinch),
        JS_DEFAULT_METHOD_ENTRY(zoomToRect, ZoomToRect),
        JS_DEFAULT_METHOD_ENTRY(setDidLoadBaseMapCallback, SetDidLoadBaseMapCallback),
        JS_DEFAULT_METHOD_ENTRY(setDidUnloadBaseMapCallback, SetDidUnloadBaseMapCallback),
        JS_DEFAULT_METHOD_ENTRY(setDidUpdateZoomLevelConfigCallback, SetDidUpdateZoomLevelConfigCallback),
        JS_DEFAULT_METHOD_ENTRY(setDidTapSeatCallback, SetDidTapSeatCallback),
        JS_DEFAULT_METHOD_ENTRY(setDidTapZoneCallback, SetDidTapZoneCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportWillBeginDraggingCallback, SetViewportWillBeginDraggingCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportDidScrollCallback, SetViewportDidScrollCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportDidEndDraggingCallback, SetViewportDidEndDraggingCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportDidEndDeceleratingCallback, SetViewportDidEndDeceleratingCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportWillBeginZoomingCallback, SetViewportWillBeginZoomingCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportDidZoomCallback, SetViewportDidZoomCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportDidEndZoomingCallback, SetViewportDidEndZoomingCallback),
        JS_DEFAULT_METHOD_ENTRY(setViewportDidEndScrollingAnimationCallback, SetViewportDidEndScrollingAnimationCallback),
        JS_DEFAULT_METHOD_ENTRY(updateSeatZoneAlternateColors, UpdateSeatZoneAlternateColors),
        JS_DEFAULT_METHOD_ENTRY(updateMiniMapZoneAlternateColors, UpdateMiniMapZoneAlternateColors),
        JS_DEFAULT_METHOD_ENTRY(updateSeats, UpdateSeats),
        JS_DEFAULT_METHOD_ENTRY(registerPricecodes, RegisterPricecodes),
        JS_DEFAULT_METHOD_ENTRY(updateSeatStatuses, UpdateSeatStatuses),
        JS_DEFAULT_METHOD_ENTRY(updateSeatStatusesForZone, UpdateSeatStatusesForZone),
        JS_DEFAULT_METHOD_ENTRY(setSelectedSeatIds, SetSelectedSeatIds),
        JS_DEFAULT_METHOD_ENTRY(updateSelectedSeatIds, UpdateSelectedSeatIds),
        JS_DEFAULT_METHOD_ENTRY(clearSeatData, ClearSeatData),
    };

    auto status = DefineClass(env, exports, ClassName(), sizeof(classProp) / sizeof(classProp[0]), classProp, Constructor, "");
    if (status != napi_ok) {
        return false;
    }
    return true;
}

JRendererCore::JRendererCore(const std::string &id)
    : id(std::move(id))
    , renderer(std::make_unique<kk::renderer::SeatCanvasCoreRenderer>(nullptr, std::make_unique<kk::gesture::ElasticZoomPanController>()))
    , delegate(std::make_shared<OHOSSeatCanvasCoreRendererDelegate>()) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);

    renderer->setDelegate(delegate);
}

JRendererCore::~JRendererCore() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    // delegate 会在析构时自动清理 threadsafe function
}

void JRendererCore::onSurfaceCreated(OH_NativeXComponent *component, NativeWindow *window) {
    if (component == nullptr || window == nullptr) {
        renderer->replacePlatformView(nullptr);
        return;
    }
    auto platformView = std::make_unique<kk::renderer::OHOSPlatformView>(component, window);
    renderer->replacePlatformView(std::move(platformView));
    renderer->updateSize();
}

void JRendererCore::onSurfaceSizeChanged() {
    renderer->updateSize();
}

void JRendererCore::onSurfaceDestroyed() {
    renderer->replacePlatformView(nullptr);
}

void JRendererCore::start() {
    renderer->start();
}

void JRendererCore::stop() {
    renderer->stop();
}

void JRendererCore::setBackgroundColor(const tgfx::Color &color) {
    renderer->setBackgroundColor(color);
}
};  // namespace kk::js
