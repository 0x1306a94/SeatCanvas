//
//  JRendererCore.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "JRendererCore.h"

#include "JsHelper.h"
#include "OHOSRendererBackend.h"
#include "core/BaseMapConfig.hpp"
#include "core/Platform.hpp"
#include "core/RegionInfo.hpp"
#include "core/UniqueID.h"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/svg/ConvertSVGLayer.hpp"
#include "core/utils/TimeProfiler.hpp"

#include <cstdint>
#include <unordered_map>

#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/platform/Print.h>
#include <tgfx/svg/SVGDOM.h>

namespace kk::js {

static std::unordered_map<std::string, std::shared_ptr<JRendererCore>> ViewMap = {};

struct LoadSVGBaseMapResult {
    std::shared_ptr<kk::layer::BaseMapRootLayer> rootLayer;
    tgfx::Size baseMapSize;
    std::shared_ptr<kk::layer::BaseMapRootLayer> miniLayer;
    std::shared_ptr<kk::BaseMapLayerManager> layerManager;
};

struct LoadSVGBaseMapTaskData {
    napi_async_work asyncWork{nullptr};
    napi_deferred deferred{nullptr};
    napi_ref callback{nullptr};
    NativeResourceManager *mNativeResMgr{nullptr};
    std::string filename{""};
    LoadSVGBaseMapResult *result{nullptr};
};

static void LoadSVGBaseMapTaskExecute(napi_env env, void *userdata) {
    auto taskData = static_cast<LoadSVGBaseMapTaskData *>(userdata);

    auto data = LoadDataFromAsset(taskData->mNativeResMgr, taskData->filename.c_str());
    if (data == nullptr) {
        return;
    }

    PROFILE_GROUP_START(group, "LoadBaseMapFromSVG");

    PROFILE_STAGE_START(group, dom, "ParseSVG");
    auto stream = tgfx::Stream::MakeFromData(data);
    auto dom = tgfx::SVGDOM::Make(*stream);

    if (!dom) {
        return;
    }

    PROFILE_STAGE_END(group, dom);

    PROFILE_STAGE_START(group, basemap, "ConvertSVGDomToLayer BaseMap");
    kk::svg::ConvertSVGLayerOptions options{};
    options.collectRegionInfo = true;
    options.supportText = true;
    auto baseMapResult = kk::svg::convertSVGDomToLayer(dom, options);
    if (!baseMapResult) {
        return;
    }

    PROFILE_STAGE_END(group, basemap);

    PROFILE_STAGE_START(group, mini, "ConvertSVGDomToLayer MiniMap")
    auto minimapResult = kk::svg::convertSVGDomToLayer(dom);
    if (!minimapResult) {
        return;
    }
    PROFILE_STAGE_END(group, mini);

    auto taskResult = new LoadSVGBaseMapResult();
    taskResult->rootLayer = std::move(baseMapResult->layer);
    taskResult->baseMapSize = baseMapResult->size;
    taskResult->miniLayer = std::move(minimapResult->layer);
    taskResult->layerManager = std::move(baseMapResult->layerManager);

    taskData->result = taskResult;
}

static void LoadSVGBaseMapTaskComplete(napi_env env, napi_status status, void *userdata) {
    auto taskData = static_cast<LoadSVGBaseMapTaskData *>(userdata);
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

static napi_value UpdateDensity(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    double value;
    napi_get_value_double(env, args[0], &value);
    kk::renderer::OHOSRendererBackend::UpdateDensity(static_cast<float>(value));
    return nullptr;
}

static napi_value ParseBaseMapFromAssets(napi_env env, napi_callback_info info) {
    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);

    NativeResourceManager *mNativeResMgr = OH_ResourceManager_InitNativeResourceManager(env, args[0]);
    if (mNativeResMgr == nullptr) {
        return nullptr;
    }

    size_t nameLength;
    char name[1024];
    napi_get_value_string_utf8(env, args[1], name, sizeof(name), &nameLength);

    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);

    auto taskData = new LoadSVGBaseMapTaskData();
    taskData->deferred = deferred;
    taskData->mNativeResMgr = mNativeResMgr;
    taskData->filename = std::string(name);

    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "LoadSVGBaseMapTask", NAPI_AUTO_LENGTH, &resourceName);
    // 创建异步任务
    napi_create_async_work(env, nullptr, resourceName, LoadSVGBaseMapTaskExecute, LoadSVGBaseMapTaskComplete, taskData, &taskData->asyncWork);
    // 将异步任务加入队列
    napi_queue_async_work(env, taskData->asyncWork);

    return promise;
}

static napi_value UniqueID(napi_env env, napi_callback_info info) {
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
    auto map = reinterpret_cast<LoadSVGBaseMapResult *>(nativePtr);
    if (map == nullptr) {
        renderer->setBaseMapConfig(nullptr, kk::SeatRenderMode::ZoomBased);
        return nullptr;
    }

    auto baseMapConfig = std::make_shared<kk::BaseMapConfig>(map->layerManager, map->rootLayer, map->miniLayer, map->baseMapSize);
    renderer->setBaseMapConfig(std::move(baseMapConfig), kk::SeatRenderMode::ZoomBased);

    delete map;

    return nullptr;
}

static napi_value EnableTiled(napi_env env, napi_callback_info info) {
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }
    bool enable = false;
    napi_get_value_bool(env, args[0], &enable);
    view->internalRenderer()->enableTiled(enable);
    return nullptr;
}

static napi_value EnableZoomBlur(napi_env env, napi_callback_info info) {
    napi_value jsView = nullptr;
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr) {
        return nullptr;
    }
    bool enable = false;
    napi_get_value_bool(env, args[0], &enable);
    view->internalRenderer()->enableZoomBlur(enable);
    return nullptr;
}

static napi_value Start(napi_env env, napi_callback_info info) {
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

static napi_value Release(napi_env env, napi_callback_info info) {
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

static napi_value SeatRegionByPoint(napi_env env, napi_callback_info info) {
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value jsView = nullptr;
    size_t argc = 2;
    napi_value args[2] = {0};
    napi_get_cb_info(env, info, &argc, args, &jsView, nullptr);
    JRendererCore *view = nullptr;
    napi_unwrap(env, jsView, reinterpret_cast<void **>(&view));
    if (view == nullptr || argc < 2) {
        return undefined;
    }
    double x, y;
    napi_get_value_double(env, args[0], &x);
    napi_get_value_double(env, args[1], &y);

    auto renderer = view->internalRenderer();
    auto currentZoom = renderer->getZoomScale();
    auto contentOffset = renderer->getContentOffset();

    auto finalX = (static_cast<float>(x) - contentOffset.x) / currentZoom;
    auto finalY = (static_cast<float>(y) - contentOffset.y) / currentZoom;

    auto regionInfo = renderer->getSeatRegionDataByPoint(finalX, finalY);
    if (!regionInfo || regionInfo->regionId.empty()) {
        return undefined;
    }

    napi_value jsRect = kk::js::CreateRect(env, regionInfo->bounds);

    napi_value jsRegionInfo;
    napi_create_object(env, &jsRegionInfo);
    napi_value v;
    napi_create_string_utf8(env, regionInfo->regionId.c_str(), regionInfo->regionId.length(), &v);
    napi_set_named_property(env, jsRegionInfo, "regionId", v);
    napi_set_named_property(env, jsRegionInfo, "bounds", jsRect);

    return jsRegionInfo;
}

static napi_value ZoomToRect(napi_env env, napi_callback_info info) {

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
        JS_STATIC_METHOD_ENTRY(UpdateDensity, UpdateDensity),
        JS_STATIC_METHOD_ENTRY(ParseBaseMapFromAssets, ParseBaseMapFromAssets),
        JS_DEFAULT_METHOD_ENTRY(uniqueID, UniqueID),
        JS_DEFAULT_METHOD_ENTRY(loadBaseMap, LoadBaseMap),
        JS_DEFAULT_METHOD_ENTRY(enableTiled, EnableTiled),
        JS_DEFAULT_METHOD_ENTRY(enableZoomBlur, EnableZoomBlur),
        JS_DEFAULT_METHOD_ENTRY(start, Start),
        JS_DEFAULT_METHOD_ENTRY(stop, Stop),
        JS_DEFAULT_METHOD_ENTRY(release, Release),
        JS_DEFAULT_METHOD_ENTRY(handleTap, HandleTap),
        JS_DEFAULT_METHOD_ENTRY(handlePan, HandlePan),
        JS_DEFAULT_METHOD_ENTRY(handlePinch, HandlePinch),
        JS_DEFAULT_METHOD_ENTRY(seatRegionByPoint, SeatRegionByPoint),
        JS_DEFAULT_METHOD_ENTRY(zoomToRect, ZoomToRect),
    };

    auto status = DefineClass(env, exports, ClassName(), sizeof(classProp) / sizeof(classProp[0]), classProp, Constructor, "");
    if (status != napi_ok) {
        return false;
    }
    return true;
}

JRendererCore::JRendererCore(const std::string &id)
    : id(std::move(id))
    , renderer(std::make_unique<kk::renderer::SeatCanvasCoreRenderer>(nullptr, std::make_unique<kk::gesture::ElasticZoomPanController>())) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

JRendererCore::~JRendererCore() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    //    release();
}

void JRendererCore::onSurfaceCreated(OH_NativeXComponent *component, NativeWindow *window) {
    if (component == nullptr || window == nullptr) {
        renderer->replaceBackend(nullptr);
        return;
    }
    auto backend = std::make_unique<kk::renderer::OHOSRendererBackend>(component, window);
    renderer->replaceBackend(std::move(backend));
    renderer->updateSize();
}

void JRendererCore::onSurfaceSizeChanged() {
    renderer->updateSize();
}

void JRendererCore::onSurfaceDestroyed() {
    renderer->replaceBackend(nullptr);
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