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
#include "core/SeatZoneData.hpp"
#include "core/UniqueID.h"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/parser/BaseMapFormat.hpp"
#include "core/parser/BaseMapParserFactory.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/svg/ConvertSVGLayer.hpp"
#include "core/svg/SVGMeshParser.hpp"
#include "core/utils/SystemProperties.hpp"
#include "core/utils/TimeProfiler.hpp"

#include <cstdint>
#include <unordered_map>

#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>

namespace kk::js {

static std::unordered_map<std::string, std::shared_ptr<JRendererCore>> ViewMap = {};

static tgfx::Color ColorFromARGBInt(uint32_t argb) {
    auto a = static_cast<uint8_t>((argb >> 24) & 0xFF);
    auto r = static_cast<uint8_t>((argb >> 16) & 0xFF);
    auto g = static_cast<uint8_t>((argb >> 8) & 0xFF);
    auto b = static_cast<uint8_t>(argb & 0xFF);
    return tgfx::Color::FromRGBA(r, g, b, a);
}

static int32_t ColorToARGBInt(const tgfx::Color &color) {
    auto a = static_cast<uint8_t>(color.alpha * 255);
    auto r = static_cast<uint8_t>(color.red * 255);
    auto g = static_cast<uint8_t>(color.green * 255);
    auto b = static_cast<uint8_t>(color.blue * 255);
    return static_cast<int32_t>((a << 24) | (r << 16) | (g << 8) | b);
}

struct LoadBaseMapResult {
    std::shared_ptr<tgfx::Layer> textLayer;
    tgfx::Size baseMapSize;
    std::shared_ptr<kk::layer::BaseMapRootLayer> miniLayer;
    std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder;

    // 从统一解析结果构造
    explicit LoadBaseMapResult(std::unique_ptr<kk::parser::BaseMapParseResult> parseResult) {
        if (parseResult) {
            textLayer = std::move(parseResult->textLayer);
            baseMapSize = parseResult->size;
            miniLayer = std::move(parseResult->miniLayer);
            meshBuilder = std::move(parseResult->meshBuilder);
        }
    }
};

struct LoadBaseMapTaskData {
    napi_async_work asyncWork{nullptr};
    napi_deferred deferred{nullptr};
    napi_ref callback{nullptr};
    NativeResourceManager *mNativeResMgr{nullptr};
    std::string filename{""};
    kk::parser::BaseMapFormat format{kk::parser::BaseMapFormat::Unknown};
    LoadBaseMapResult *result{nullptr};
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
    auto result = kk::parser::BaseMapParserFactory::parse(data, taskData->format);
    if (!result) {
        tgfx::PrintError("parse result is null");
        return;
    }

    auto taskResult = new LoadBaseMapResult(std::move(result));
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
    size_t argc = 3;
    napi_value args[3] = {nullptr};
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

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);

    auto taskData = new LoadBaseMapTaskData();
    taskData->deferred = deferred;
    taskData->mNativeResMgr = mNativeResMgr;
    taskData->filename = std::string(name);
    taskData->format = kk::parser::parseFormatName(format);

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
    auto map = reinterpret_cast<LoadBaseMapResult *>(nativePtr);
    if (map == nullptr) {
        renderer->setBaseMapConfig(nullptr, kk::SeatRenderMode::ZoomBased);
        return nullptr;
    }

    auto baseMapConfig = std::make_shared<kk::BaseMapConfig>(map->meshBuilder, map->textLayer, map->miniLayer, map->baseMapSize);
    renderer->setBaseMapConfig(std::move(baseMapConfig), kk::SeatRenderMode::ZoomBased);

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
    view->setBackgroundColor(ColorFromARGBInt(static_cast<uint32_t>(colorInt)));
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
    napi_create_int32(env, ColorToARGBInt(renderer->getBackgroundColor()), &result);
    return result;
}

static napi_value SetHighlightedZoneIds(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 2) {
        return nullptr;
    }
    bool isArray = false;
    napi_is_array(env, args[0], &isArray);
    if (!isArray) {
        return nullptr;
    }
    uint32_t length = 0;
    napi_get_array_length(env, args[0], &length);
    std::vector<std::string> zoneIds;
    zoneIds.reserve(length);
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        napi_get_element(env, args[0], i, &element);
        zoneIds.push_back(GetUtf8String(env, element));
    }
    double nonHighlightedAlpha = 0.6;
    napi_get_value_double(env, args[1], &nonHighlightedAlpha);
    auto *renderer = view->internalRenderer();
    if (renderer != nullptr) {
        renderer->setHighlightedZoneIds(zoneIds, static_cast<float>(nonHighlightedAlpha));
    }
    return nullptr;
}

static napi_value ClearHighlightedZones(napi_env env, napi_callback_info info) {
    kk::js::NapiEnvHolder::setEnv(env);
    napi_value jsView = nullptr;
    size_t argc = 0;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }
    auto *renderer = view->internalRenderer();
    if (renderer != nullptr) {
        renderer->clearHighlightedZones();
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
    //    view->release();
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

static napi_value SetShouldSelectSeatCallback(napi_env env, napi_callback_info info) {
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

    delegate->setShouldSelectSeatCallback(env, callback);

    return nullptr;
}

static napi_value SetDidSelectSeatCallback(napi_env env, napi_callback_info info) {
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

    delegate->setDidSelectSeatCallback(env, callback);

    return nullptr;
}

static napi_value SetDidDeselectSeatCallback(napi_env env, napi_callback_info info) {
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

    delegate->setDidDeselectSeatCallback(env, callback);

    return nullptr;
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

static napi_value UpdateSeatZones(napi_env env, napi_callback_info info) {
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
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, args[0], i, &element);
        auto zoneData = GetSeatZoneData(env, element);
        if (zoneData) {
            renderer->setZoneData(zoneData.value());
        }
    }
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
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, args[1], i, &element);
        auto seatData = GetSeatData(env, element);
        if (seatData) {
            seats.push_back(seatData.value());
        }
    }

    renderer->setSeatData(zoneId, seats);
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
        JS_DEFAULT_METHOD_ENTRY(setHighlightedZoneIds, SetHighlightedZoneIds),
        JS_DEFAULT_METHOD_ENTRY(clearHighlightedZones, ClearHighlightedZones),
        JS_DEFAULT_METHOD_ENTRY(release, Release),
        JS_DEFAULT_METHOD_ENTRY(handleTap, HandleTap),
        JS_DEFAULT_METHOD_ENTRY(handlePan, HandlePan),
        JS_DEFAULT_METHOD_ENTRY(handlePinch, HandlePinch),
        JS_DEFAULT_METHOD_ENTRY(zoomToRect, ZoomToRect),
        JS_DEFAULT_METHOD_ENTRY(setShouldSelectSeatCallback, SetShouldSelectSeatCallback),
        JS_DEFAULT_METHOD_ENTRY(setDidSelectSeatCallback, SetDidSelectSeatCallback),
        JS_DEFAULT_METHOD_ENTRY(setDidDeselectSeatCallback, SetDidDeselectSeatCallback),
        JS_DEFAULT_METHOD_ENTRY(updateSeatZones, UpdateSeatZones),
        JS_DEFAULT_METHOD_ENTRY(updateSeats, UpdateSeats),
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
    //    release();
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

void JRendererCore::release() {
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