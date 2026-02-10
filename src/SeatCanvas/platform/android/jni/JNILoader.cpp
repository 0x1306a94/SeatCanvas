//
//  JNILoader.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/24.
//

#include <jni.h>

#include <tgfx/core/Color.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/layers/TextLayer.h>
#include <tgfx/platform/Print.h>
#include <tgfx/platform/android/JNIEnvironment.h>
#include <tgfx/svg/SVGDOM.h>

#include "AndroidSeatCanvasCoreRendererDelegate.hpp"
#include "JNIHelper.hpp"
#include "JRect.h"
#include "JSeatData.h"
#include "JSeatZoneData.h"
#include "JStringUtil.hpp"
#include "JZoomLevel.h"
#include "core/BaseMapConfig.hpp"
#include "core/FontManager.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/parser/BaseMapFormat.hpp"
#include "core/parser/BaseMapParserFactory.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/svg/ConvertSVGLayer.hpp"
#include "core/svg/SVGMeshParser.hpp"
#include "core/utils/SystemProperties.hpp"
#include "core/utils/TimeProfiler.hpp"
#include "platform/android/NativePlatform.hpp"
#include "platform/android/renderer/AndroidPlatformView.hpp"

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
}  // namespace kk::jni

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeLoadBaseMapFromFormat(JNIEnv *env, jobject thiz,
                                                                  jbyteArray srcData,
                                                                  jstring jformatName) {
    if (srcData == nullptr) {
        tgfx::PrintError("data is null");
        return 0;
    }

    auto bytes = env->GetByteArrayElements(srcData, nullptr);
    auto len = env->GetArrayLength(srcData);
    if (len == 0) {
        tgfx::PrintError("data length is zero");
        return 0;
    }

    auto formatName = kk::jni::SafeConvertToStdString(env, jformatName);
    auto format = kk::parser::parseFormatName(formatName);
    if (format == kk::parser::BaseMapFormat::Unknown) {
        tgfx::PrintError("format is Unknown");
        return 0;
    }

    PROFILE_TIME(std::string("LoadBaseMapFrom") + kk::parser::formatNameToString(format));

    auto data = tgfx::Data::MakeWithoutCopy(bytes, len);
    auto result = kk::parser::BaseMapParserFactory::parse(data, format);
    env->ReleaseByteArrayElements(srcData, bytes, 0);
    if (!result) {
        tgfx::PrintError("parse result is null");
        return 0;
    }

    auto outResult = new kk::jni::LoadBaseMapResult(std::move(result));
    return reinterpret_cast<jlong>(outResult);
}

JNIEXPORT jboolean JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeLoadBaseMap(JNIEnv *env, jobject thiz, jlong dataPtr) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, false);
    if (dataPtr == 0) {
        renderer->setBaseMapConfig(nullptr, kk::SeatRenderMode::ZoomBased);
        return false;
    }

    auto map = reinterpret_cast<kk::jni::LoadBaseMapResult *>(dataPtr);
    auto baseMapConfig = std::make_shared<kk::BaseMapConfig>(map->meshBuilder, map->textLayer,
                                                             map->miniLayer, map->baseMapSize);
    renderer->setBaseMapConfig(std::move(baseMapConfig), kk::SeatRenderMode::ZoomBased);
    delete map;

    return true;
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeSetCanvasColor(JNIEnv *env, jobject thiz, jint color) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    auto argb = static_cast<uint32_t>(color);
    renderer->setBackgroundColor(kk::jni::ColorFromARGBInt(argb));
}

JNIEXPORT jint JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetCanvasColor(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 0xFFFFFFFF);
    return static_cast<jint>(kk::jni::ColorToARGBInt(renderer->getBackgroundColor()));
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeSetSeatStyleJSONConfig(JNIEnv *env, jobject thiz,
                                                                   jbyteArray data, jint len) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    if (data == nullptr || len == 0) {
        renderer->setStyleKeyToConfigFromJSON(nullptr, 0);
        return;
    }

    auto bytes = env->GetByteArrayElements(data, nullptr);
    renderer->setStyleKeyToConfigFromJSON(bytes, len);
    env->ReleaseByteArrayElements(data, bytes, 0);
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeUpdateSeatZones(JNIEnv *env, jobject thiz,
                                                            jobjectArray jzones) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    if (jzones == nullptr) {
        return;
    }
    jsize length = env->GetArrayLength(jzones);
    for (jsize i = 0; i < length; ++i) {
        jobject element = env->GetObjectArrayElement(jzones, i);
        if (element == nullptr) {
            continue;
        }
        auto zoneData = kk::jni::JSeatZoneData::FromJava(env, element);
        env->DeleteLocalRef(element);
        if (zoneData) {
            renderer->setZoneData(*zoneData);
        }
    }
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeUpdateSeats(JNIEnv *env, jobject thiz,
                                                        jstring jzoneId, jobjectArray jseats) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    auto zoneId = kk::jni::SafeConvertToStdString(env, jzoneId);
    if (zoneId.empty()) {
        return;
    }
    if (jseats == nullptr) {
        return;
    }
    jsize length = env->GetArrayLength(jseats);
    std::vector<kk::SeatData> seats;
    seats.reserve(static_cast<size_t>(length));
    for (jsize i = 0; i < length; ++i) {
        jobject element = env->GetObjectArrayElement(jseats, i);
        if (element == nullptr) {
            continue;
        }
        auto seatData = kk::jni::JSeatData::FromJava(env, element);
        env->DeleteLocalRef(element);
        if (seatData) {
            seats.push_back(*seatData);
        }
    }
    renderer->setSeatData(zoneId, seats);
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeSetHighlightedZoneIds(JNIEnv *env, jobject thiz,
                                                                  jobjectArray zoneIds,
                                                                  jfloat nonHighlightedAlpha) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    std::vector<std::string> vec;
    if (zoneIds != nullptr) {
        jsize len = env->GetArrayLength(zoneIds);
        vec.reserve(static_cast<size_t>(len));
        for (jsize i = 0; i < len; ++i) {
            jobject elem = env->GetObjectArrayElement(zoneIds, i);
            if (elem != nullptr) {
                vec.push_back(kk::jni::SafeConvertToStdString(env, static_cast<jstring>(elem)));
                env->DeleteLocalRef(elem);
            }
        }
    }
    renderer->setHighlightedZoneIds(vec, static_cast<float>(nonHighlightedAlpha));
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeClearHighlightedZones(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->clearHighlightedZones();
}

JNIEXPORT jfloat JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetSeatSize(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 36.0f);
    return renderer->getSeatSize();
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeSetSeatSize(JNIEnv *env, jobject thiz, jfloat seatSize) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->setSeatSize(seatSize);
}

JNIEXPORT jfloat JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetSeatRenderZoomThreshold(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 0.0f);
    return static_cast<jfloat>(renderer->getSeatRenderZoomThreshold());
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeSetSeatRenderZoomThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->setSeatRenderZoomThreshold(static_cast<float>(threshold));
}

JNIEXPORT jobject JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetZoomLevel(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, nullptr);
    const auto &config = renderer->zoomLevelConfig();
    return kk::jni::JZoomLevel::ToJava(env, config);
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeHandleTap(JNIEnv *env, jobject thiz, jfloat x,
                                                      jfloat y) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->handleTap(tgfx::Point{x, y});
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeHandlePan(JNIEnv *env, jobject thiz, jint state,
                                                      jfloat tx, jfloat ty, jdouble timestampMs) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->handlePan(static_cast<kk::gesture::GestureState>(state), tgfx::Point{tx, ty},
                        timestampMs);
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeHandlePinch(JNIEnv *env, jobject thiz, jint state,
                                                        jfloat scale, jfloat cx, jfloat cy) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->handlePinch(static_cast<kk::gesture::GestureState>(state), scale,
                          tgfx::Point{cx, cy});
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeStartDrawLoop(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->start();
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeStopDrawLoop(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->stop();
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeInvalidateContent(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->invalidateContent();
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeUpdateSize(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->updateSize();
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeUpdateSurface(JNIEnv *env, jobject thiz,
                                                          jobject surface) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    if (surface == nullptr) {
        renderer->replacePlatformView(nullptr);
        return;
    }

    auto nativeWindow = ANativeWindow_fromSurface(env, surface);
    if (nativeWindow == nullptr) {
        renderer->replacePlatformView(nullptr);
        return;
    }
    auto &properties = kk::utils::SystemProperties::Instance();
    auto platformView = std::make_unique<kk::renderer::AndroidPlatformView>(nativeWindow,
                                                                            properties.density);
    renderer->replacePlatformView(std::move(platformView));
    renderer->updateSize();
}

JNIEXPORT jlong JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeCreate(JNIEnv *env, jobject thiz) {
    auto zoomPanController = std::make_unique<kk::gesture::ElasticZoomPanController>();
    auto renderer = new kk::renderer::SeatCanvasCoreRenderer(nullptr, std::move(zoomPanController));

    // 创建并设置 Android delegate，传入 SeatCanvasView 的 jobject
    auto delegate = std::make_shared<kk::renderer::AndroidSeatCanvasCoreRendererDelegate>(thiz);
    renderer->setDelegate(std::move(delegate));

    auto ptr = reinterpret_cast<jlong>(renderer);
    return ptr;
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeRelease(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    renderer->stop();
    kk::jni::DeleteSeatCanvasCoreRenderer(env, thiz);
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeZoomToRect(JNIEnv *env, jobject thiz, jobject bounds,
                                                       jboolean animated, jfloat padding,
                                                       jdouble durationMs) {
    GetCPPObjectOrReturn(env, thiz, renderer);
    auto rect = kk::jni::JRect::FromJava(env, bounds);
    if (rect.isEmpty()) {
        return;
    }
    renderer->zoomToRect(rect, animated, padding, durationMs);
}

JNIEXPORT jfloat JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetZoomScale(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 1.0f);
    return renderer->getZoomScale();
}

JNIEXPORT jfloat JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetMinimumZoomScale(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 1.0f);
    return renderer->getMinimumZoomScale();
}

JNIEXPORT jfloat JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetMaximumZoomScale(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, 1.0f);
    return renderer->getMaximumZoomScale();
}

JNIEXPORT jfloatArray JNICALL
Java_com_libseatcanvas_SeatCanvasView_nativeGetContentOffset(JNIEnv *env, jobject thiz) {
    GetCPPObjectOrReturnValue(env, thiz, renderer, nullptr);
    auto offset = renderer->getContentOffset();
    jfloatArray result = env->NewFloatArray(2);
    if (result != nullptr) {
        jfloat values[2] = {offset.x, offset.y};
        env->SetFloatArrayRegion(result, 0, 2, values);
    }
    return result;
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_SeatCanvasView_00024Companion_nativeInitSystemProperties(JNIEnv *env,
                                                                                jobject thiz,
                                                                                jfloat density,
                                                                                jfloat fontScale) {
    auto &properties = kk::utils::SystemProperties::Instance();
    properties.updateDensity(density);
    properties.updateFontScale(fontScale);
}

JNIEXPORT void JNICALL
Java_com_libseatcanvas_Font_00024Companion_nativeSetFallbackFontPaths(JNIEnv *env, jobject thiz,
                                                                      jobjectArray fontNameList,
                                                                      jintArray ttcIndices) {
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
        tgfx::PrintError(
            "Could not run NativeDisplayLink.InitJNI(), DisplayLinkClass is not found!");
        return JNI_ERR;
    }
    kk::jni::SeatCanvasView_NativePtr = env->GetFieldID(kk::jni::SeatCanvasViewClass.get(),
                                                        "nativePtr", "J");

    // 初始化辅助 JNI 类型
    kk::jni::JRect::InitJNI(env);
    kk::jni::JZoomLevel::InitJNI(env);
    kk::jni::JSeatData::InitJNI(env);
    kk::jni::JSeatZoneData::InitJNI(env);

    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *, void *) {
    tgfx::JNIEnvironment::SetJavaVM(nullptr);
}
}