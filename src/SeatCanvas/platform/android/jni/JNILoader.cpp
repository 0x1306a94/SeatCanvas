//
//  JNILoader.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#include <jni.h>

#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/layers/TextLayer.h>
#include <tgfx/platform/Print.h>
#include <tgfx/platform/android/JNIEnvironment.h>
#include <tgfx/svg/SVGDOM.h>

#include "JHitTestSeatRegionResult.h"
#include "JNIHelper.hpp"
#include "JRect.h"
#include "JStringUtil.hpp"
#include "core/BaseMapConfig.hpp"
#include "core/FontManager.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/svg/ConvertSVGLayer.hpp"
#include "core/utils/TimeProfiler.hpp"
#include "platform/android/NativePlatform.hpp"
#include "platform/android/renderer/AndroidRendererBackend.hpp"

#define __GetCPPObjectOrReturn(env, thiz, typedObjName, action) \
    if (env == nullptr || thiz == nullptr) {                    \
        action;                                                 \
    }                                                           \
    auto typedObjName = kk::jni::GetSeatCanvasCoreRenderer(env, thiz);

/* clang-format off */

#define GetCPPObjectOrReturn(env, thiz, typedObjName) \
    __GetCPPObjectOrReturn(env, thiz, typedObjName, return)

#define GetCPPObjectOrReturnValue(env, thiz, typedObjName, value) \
    __GetCPPObjectOrReturn(env, thiz, typedObjName, return value)

/* clang-format on */

namespace kk::jni {
static kk::jni::Global<jclass> SeatCanvasViewClass;
static jfieldID SeatCanvasView_NativePtr;

struct LoadSVGBaseMapResult {
    std::shared_ptr<kk::layer::BaseMapRootLayer> rootLayer;
    tgfx::Size baseMapSize;
    std::shared_ptr<kk::layer::BaseMapRootLayer> miniLayer;
    std::shared_ptr<kk::BaseMapLayerManager> layerManager;
};

static kk::renderer::SeatCanvasCoreRenderer *GetSeatCanvasCoreRenderer(JNIEnv *env, jobject thiz) {
    jlong ptr = env->GetLongField(thiz, SeatCanvasView_NativePtr);
    if (ptr == 0) {
        return nullptr;
    }
    auto renderer = reinterpret_cast<kk::renderer::SeatCanvasCoreRenderer *>(ptr);
    return renderer;
}

static void DeleteSeatCanvasCoreRenderer(JNIEnv *env, jobject thiz) {
    jlong ptr = env->GetLongField(thiz, SeatCanvasView_NativePtr);
    if (ptr == 0) {
        return;
    }
    auto renderer = reinterpret_cast<kk::renderer::SeatCanvasCoreRenderer *>(ptr);
    delete renderer;
    env->SetLongField(thiz, SeatCanvasView_NativePtr, 0L);
}
}  // namespace kk::jni

extern "C" {

JNIEXPORT jlong JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeLoadBaseMapFromSVG(JNIEnv *env, jobject thiz, jbyteArray srcData) {
    if (srcData == nullptr) {
        return 0;
    }
    auto bytes = env->GetByteArrayElements(srcData, nullptr);
    auto len = env->GetArrayLength(srcData);
    if (len == 0) {
        return 0;
    }

    PROFILE_GROUP_START(group, "LoadBaseMapFromSVG");

    PROFILE_STAGE_START(group, dom, "ParseSVG");
    auto data = tgfx::Data::MakeWithoutCopy(bytes, len);
    auto stream = tgfx::Stream::MakeFromData(data);
    auto dom = tgfx::SVGDOM::Make(*stream);

    env->ReleaseByteArrayElements(srcData, bytes, 0);
    if (!dom) {
        return 0;
    }

    PROFILE_STAGE_END(group, dom);

    PROFILE_STAGE_START(group, basemap, "ConvertSVGDomToLayer BaseMap")
    kk::svg::ConvertSVGLayerOptions options{};
    options.collectRegionInfo = true;
    options.supportText = true;
    auto baseMapResult = kk::svg::convertSVGDomToLayer(dom, options);
    if (!baseMapResult) {
        return 0;
    }

    PROFILE_STAGE_END(group, basemap);

    PROFILE_STAGE_START(group, mini, "ConvertSVGDomToLayer MiniMap")
    auto minimapResult = kk::svg::convertSVGDomToLayer(dom);
    if (!minimapResult) {
        return 0;
    }
    PROFILE_STAGE_END(group, mini);

    auto outResult = new kk::jni::LoadSVGBaseMapResult();
    outResult->rootLayer = std::move(baseMapResult->layer);
    outResult->baseMapSize = baseMapResult->size;
    outResult->miniLayer = std::move(minimapResult->layer);
    outResult->layerManager = std::move(baseMapResult->layerManager);

    PROFILE_GROUP_END(group)

    return reinterpret_cast<jlong>(outResult);
}

JNIEXPORT jboolean JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeLoadBaseMap(JNIEnv *env, jobject thiz, jlong dataPtr) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, false);
    if (dataPtr == 0) {
        renderer->setBaseMapConfig(nullptr, kk::SeatRenderMode::ZoomBased);
        return false;
    }

    auto map = reinterpret_cast<kk::jni::LoadSVGBaseMapResult *>(dataPtr);
    auto baseMapConfig = std::make_shared<kk::BaseMapConfig>(map->layerManager, map->rootLayer, map->miniLayer, map->baseMapSize);
    renderer->setBaseMapConfig(std::move(baseMapConfig), kk::SeatRenderMode::ZoomBased);
    delete map;

    return true;
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeHandleTap(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->handleTap(tgfx::Point{x, y});
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeHandlePan(JNIEnv *env, jobject thiz, jint state, jfloat tx, jfloat ty, jdouble timestampMs) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->handlePan(static_cast<kk::gesture::GestureState>(state), tgfx::Point{tx, ty}, timestampMs);
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeHandlePinch(JNIEnv *env, jobject thiz, jint state, jfloat scale, jfloat cx, jfloat cy) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->handlePinch(static_cast<kk::gesture::GestureState>(state), scale, tgfx::Point{cx, cy});
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeEnableTiled(JNIEnv *env, jobject thiz, jboolean enable) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->enableTiled(enable);
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeEnableZoomBlur(JNIEnv *env, jobject thiz, jboolean enable) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->enableZoomBlur(enable);
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeStartDrawLoop(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->start();
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeStopDrawLoop(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->stop();
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeInvalidateContent(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->invalidateContent();
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeUpdateSize(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->updateSize();
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeUpdateSurface(JNIEnv *env, jobject thiz, jobject surface, jfloat density) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    if (surface == nullptr) {
        renderer->replaceBackend(nullptr);
        return;
    }

    auto nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (nativeWindow == nullptr) {
        renderer->replaceBackend(nullptr);
        return;
    }

    auto backend = std::make_unique<kk::renderer::AndroidRendererBackend>(nativeWindow, density);
    renderer->replaceBackend(std::move(backend));
    renderer->updateSize();
}

JNIEXPORT jlong JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeCreate(JNIEnv *env, jobject thiz) {
    auto zoomPanController = std::make_unique<kk::gesture::ElasticZoomPanController>();
    auto renderer = new kk::renderer::SeatCanvasCoreRenderer(nullptr, std::move(zoomPanController));
    auto ptr = reinterpret_cast<jlong>(renderer);
    return ptr;
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeRelease(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->stop();
    kk::jni::DeleteSeatCanvasCoreRenderer(env, thiz);
}

JNIEXPORT jobject JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeSeatRegionByPoint(JNIEnv *env, jobject thiz, jfloat x, jfloat y) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, nullptr);
    auto regionInfo = renderer->getSeatRegionDataByPoint(x, y);
    if (regionInfo == nullptr) {
        return nullptr;
    }
    return kk::jni::JHitTestSeatRegionResult::ToJava(env, regionInfo);
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeZoomToRect(JNIEnv *env, jobject thiz, jobject bounds, jboolean animated, jfloat padding, jdouble durationMs) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    auto rect = kk::jni::JRect::FromJava(env, bounds);
    if (rect.isEmpty()) {
        return;
    }
    renderer->zoomToRect(rect, animated, padding, durationMs);
}

JNIEXPORT jfloat JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeGetZoomScale(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 1.0f);
    return renderer->getZoomScale();
}

JNIEXPORT jfloatArray JNICALL Java_com_libseatcanvas_SeatCanvasView_nativeGetContentOffset(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, nullptr);
    auto offset = renderer->getContentOffset();
    jfloatArray result = env->NewFloatArray(2);
    if (result != nullptr) {
        jfloat values[2] = {offset.x, offset.y};
        env->SetFloatArrayRegion(result, 0, 2, values);
    }
    return result;
}

JNIEXPORT void JNICALL Java_com_libseatcanvas_Font_00024Companion_nativeSetFallbackFontPaths(JNIEnv *env, jobject thiz, jobjectArray fontNameList, jintArray ttcIndices) {
    std::vector<std::string> fallbackList;
    std::vector<int> ttcList;
    auto length = env->GetArrayLength(fontNameList);
    auto ttcLength = env->GetArrayLength(ttcIndices);
    length = std::min(length, ttcLength);
    auto ttcData = env->GetIntArrayElements(ttcIndices, nullptr);
    for (int index = 0; index < length; index++) {
        auto fontNameObject = (jstring)env->GetObjectArrayElement(fontNameList, index);
        auto fontFamily = kk::jni::SafeConvertToStdString(env, fontNameObject);
        fallbackList.push_back(fontFamily);
        ttcList.push_back(ttcData[index]);
    }
    env->ReleaseIntArrayElements(ttcIndices, ttcData, 0);

    kk::FontManager::SetFallbackFontPaths(fallbackList, ttcList);

    // 使用 GetFallbackTypefacesDirect() 避免静态初始化循环依赖
    std::vector<std::shared_ptr<tgfx::Typeface>> fallbackTypefaces = kk::FontManager::GetFallbackTypefacesDirect();
    tgfx::TextLayer::SetFallbackTypefaces(std::move(fallbackTypefaces));
}

jint JNI_OnLoad(JavaVM *vm, void *) {
    tgfx::JNIEnvironment::SetJavaVM(vm);
    kk::NativePlatform::InitJNI();

    kk::jni::JNIEnvironment environment;
    auto env = environment.current();
    kk::jni::SeatCanvasViewClass = env->FindClass("com/libseatcanvas/SeatCanvasView");
    if (kk::jni::SeatCanvasViewClass.get() == nullptr) {
        tgfx::PrintError("Could not run NativeDisplayLink.InitJNI(), DisplayLinkClass is not found!");
        return JNI_ERR;
    }
    kk::jni::SeatCanvasView_NativePtr = env->GetFieldID(kk::jni::SeatCanvasViewClass.get(), "nativePtr", "J");

    // 初始化 JRect 和 JHitTestSeatRegionResult
    kk::jni::JRect::InitJNI(env);
    kk::jni::JHitTestSeatRegionResult::InitJNI(env);

    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *, void *) {
    tgfx::JNIEnvironment::SetJavaVM(nullptr);
}
}