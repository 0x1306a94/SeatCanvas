//
//  SwiftBridge.mm
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#import "swift/SwiftBridge.h"

#import "core/BaseMapConfig.hpp"
#import "core/Platform.hpp"
#import "core/RegionInfo.hpp"
#import "core/gesture/ElasticZoomPanController.hpp"
#import "core/layers/BaseMapRootLayer.hpp"
#import "core/renderer/SeatCanvasCoreRenderer.hpp"
#import "core/renderer/SeatCanvasCoreRendererState.hpp"
#import "core/utils/SystemProperties.hpp"
#import "platform/apple/LayerPrerenderImage.h"
#import "platform/ios/renderer/IOSPlatformView.h"

#import "core/parser/BaseMapFormat.hpp"
#import "core/parser/BaseMapParserFactory.hpp"
#import "core/svg/ConvertSVGLayer.hpp"
#import "core/svg/SVGMeshParser.hpp"
#import "core/utils/TimeProfiler.hpp"

#import "SwiftSeatCanvasCoreRendererDelegate.hpp"

#import <tgfx/core/Data.h>
#import <tgfx/core/Stream.h>
#import <tgfx/platform/Print.h>
#import <tgfx/svg/SVGDOM.h>

#define __GetCPPObjectOrReturn(rawObj, objType, typedObjName, action) \
    if (rawObj == nullptr) {                                          \
        action;                                                       \
    }                                                                 \
    if (rawObj->realValue == nullptr) {                               \
        action;                                                       \
    }                                                                 \
    auto typedObjName = static_cast<objType>(rawObj->realValue);

/* clang-format off */

#define GetCPPObjectOrReturn(rawObj, objType, typedObjName) \
    __GetCPPObjectOrReturn(rawObj, objType, typedObjName, return)


#define GetCPPObjectOrReturnValue(rawObj, objType, typedObjName, value) \
    __GetCPPObjectOrReturn(rawObj, objType, typedObjName, return value)

/* clang-format on */

namespace kk {

template <typename T>
static void releaseCPPObject(void *_Nonnull obj) {
    T *typedObj = (T *)obj;
    if (typedObj != nullptr) {
        delete typedObj;
    }
}

// #ifdef __cplusplus
// extern "C" {
// #endif

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

void SeatCanvasReleaseCPPObject(CPPObject *_Nonnull obj) {
    if (obj == nullptr) {
        return;
    }

    if (obj->deleter != nullptr) {
        obj->deleter(obj->realValue);
    }

    free(obj);
}

void SeatCanvasRegisterFallbackFonts() {
    kk::Platform::Current()->registerFallbackFonts();
}

void SeatCanvasInitSystemProperties(CGFloat density, CGFloat fontScale) {
    auto &properties = kk::utils::SystemProperties::Instance();
    properties.updateDensity(static_cast<float>(density));
    properties.updateFontScale(static_cast<float>(fontScale));
}

void *_Nullable SeatCanvasLoadBaseMap(const void *_Nullable __sized_by_or_null(len) bytes, size_t len, kk::parser::BaseMapFormat format, UIImage *_Nullable *_Nullable miniMapImage) {
    if (bytes == nullptr || len == 0) {
        tgfx::PrintError("bytes is null or len is zero");
        return nullptr;
    }

    if (format == kk::parser::BaseMapFormat::Unknown) {
        tgfx::PrintError("format is Unknown");
        return nullptr;
    }

    PROFILE_TIME(std::string("LoadBaseMapFrom") + kk::parser::formatNameToString(format));
    auto data = tgfx::Data::MakeWithoutCopy(bytes, len);
    auto result = kk::parser::BaseMapParserFactory::parse(data, format);
    if (!result) {
        tgfx::PrintError("parse result is null");
        return nullptr;
    }

    LoadBaseMapResult *outResult = new LoadBaseMapResult(std::move(result));
    //    if (miniMapImage != nullptr && minLayer) {
    //        PROFILE_STAGE_START(group, genMiniMapImage, "Gen MiniMap Image")
    //        auto result = kk::renderer::LayerPrerenderImage::Render(baseMapLayer, baseMapSize, 6000);
    //        PROFILE_STAGE_END(group, genMiniMapImage)
    //        if (result) {
    //            UIImage *image = [UIImage imageWithCGImage:result];
    //            CGImageRelease(result);
    //            *miniMapImage = image;
    //        }
    //    }

    return static_cast<void *>(outResult);
}

void *_Nullable SeatCanvasLoadBaseMapFromSVG(const void *_Nullable __sized_by_or_null(len) bytes, size_t len, UIImage *_Nullable *_Nullable miniMapImage) {
    return SeatCanvasLoadBaseMap(bytes, len, kk::parser::BaseMapFormat::SVG, miniMapImage);
}

CPPObject *_Nonnull CreateSeatCanvasCoreRenderer(CAEAGLLayer *_Nonnull eagLayer) {
    auto platformView = std::make_unique<kk::renderer::IOSPlatformView>(eagLayer);
    auto zoomPanController = std::make_unique<kk::gesture::ElasticZoomPanController>();
    auto renderer = new kk::renderer::SeatCanvasCoreRenderer(std::move(platformView), std::move(zoomPanController));

    auto delegate = std::make_shared<kk::renderer::SwiftSeatCanvasCoreRendererDelegate>();
    renderer->setDelegate(std::move(delegate));

    CPPObject *cppObj = (CPPObject *)malloc(sizeof(CPPObject));
    cppObj->realValue = renderer;
    cppObj->deleter = releaseCPPObject<kk::renderer::SeatCanvasCoreRenderer>;
    return cppObj;
}

uint32_t SeatCanvasCoreRendererGetCoreID(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 0);
    return renderer->coreID();
}

void SeatCanvasCoreRendererSetSeatStyleJSONConfig(CPPObject *_Nonnull cppObject, const void *_Nullable __sized_by_or_null(len) bytes, size_t len) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->setStyleKeyToConfigFromJSON(bytes, len);
}

bool SeatCanvasCoreRendererReplacePlatformView(CPPObject *_Nonnull cppObject, CAEAGLLayer *_Nonnull eagLayer) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, false);
    if (eagLayer == nullptr) {
        renderer->replacePlatformView(nullptr);
        return true;
    }
    auto platformView = std::make_unique<kk::renderer::IOSPlatformView>(eagLayer);
    renderer->replacePlatformView(std::move(platformView));
    return true;
}

void SeatCanvasCoreRendererInvalidateContent(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->invalidateContent();
}

bool SeatCanvasCoreRendererUpdateSize(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, false);
    auto sizeChanged = renderer->updateSize();
    return sizeChanged;
}

void SeatCanvasCoreRendererUpdateZoomScale(CPPObject *_Nonnull cppObject, CGFloat zoomScale) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->setZoomScale(zoomScale);
}

void SeatCanvasCoreRendererUpdateContentOffset(CPPObject *_Nonnull cppObject, CGPoint contentOffset) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->setContentOffset(tgfx::Point{static_cast<float>(contentOffset.x), static_cast<float>(contentOffset.y)});
}

CGFloat SeatCanvasCoreRendererZoomScale(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 1.0);
    auto zoomScale = renderer->state()->getZoomScale();
    return static_cast<CGFloat>(zoomScale);
}

CGFloat SeatCanvasCoreRendererMinimumZoomScale(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 1.0);
    auto minimumZoomScale = renderer->getMinimumZoomScale();
    return static_cast<CGFloat>(minimumZoomScale);
}

CGFloat SeatCanvasCoreRendererMaximumZoomScale(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 1.0);
    auto maximumZoomScale = renderer->getMaximumZoomScale();
    return static_cast<CGFloat>(maximumZoomScale);
}

CGPoint SeatCanvasCoreRendereContentOffset(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, CGPointZero);
    auto contentOffset = renderer->state()->getContentOffset();
    return CGPointMake(static_cast<CGFloat>(contentOffset.x), static_cast<CGFloat>(contentOffset.y));
}

bool SeatCanvasCoreRendererLoadBaseMap(CPPObject *_Nonnull cppObject, void **_Nullable loadResult) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, false);
    if (loadResult == nullptr || *loadResult == nullptr) {
        renderer->setBaseMapConfig(nullptr, kk::SeatRenderMode::ZoomBased);
        return true;
    }

    // 支持 LoadBaseMapResult 和 LoadBaseMapResult（向后兼容）
    LoadBaseMapResult *map = nullptr;
    LoadBaseMapResult *svgMap = nullptr;

    // 尝试转换为 LoadBaseMapResult
    map = static_cast<LoadBaseMapResult *>(*loadResult);
    if (map == nullptr) {
        // 尝试转换为旧的 LoadBaseMapResult（向后兼容）
        svgMap = static_cast<LoadBaseMapResult *>(*loadResult);
        if (svgMap != nullptr) {
            // 转换为新的结构
            map = new LoadBaseMapResult(*svgMap);
            delete svgMap;
        }
    }

    if (map == nullptr) {
        *loadResult = nullptr;
        return false;
    }

    auto baseMapConfig = std::make_shared<kk::BaseMapConfig>(map->meshBuilder, map->textLayer, map->miniLayer, map->baseMapSize);
    renderer->setBaseMapConfig(std::move(baseMapConfig), kk::SeatRenderMode::ZoomBased);

    delete map;
    *loadResult = nullptr;

    return true;
}

CGFloat SeatCanvasCoreRendererBaseMapScale(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 1.0);
    auto contentScale = renderer->getContentScale();
    return static_cast<CGFloat>(contentScale);
}

CGSize SeatCanvasCoreRendererBaseMapSize(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, CGSizeZero);
    auto size = renderer->state()->getOriginSize();
    return CGSizeMake(static_cast<CGFloat>(size.width), static_cast<CGFloat>(size.height));
}

CGSize SeatCanvasCoreRendererBoundsSize(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, CGSizeZero);
    auto size = renderer->state()->getBoundsSize();
    return CGSizeMake(static_cast<CGFloat>(size.width), static_cast<CGFloat>(size.height));
}

CGSize SeatCanvasCoreRendererContentSize(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, CGSizeZero);
    auto size = renderer->state()->getNormalizedContentSize();
    return CGSizeMake(static_cast<CGFloat>(size.width), static_cast<CGFloat>(size.height));
}

void SeatCanvasCoreRendererSetBackgroundColor(CPPObject *_Nonnull cppObject, UIColor *_Nullable color) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    if (color == nil) {
        renderer->setBackgroundColor(tgfx::Color::White());
        return;
    }
    CGFloat red, green, blue, alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    tgfx::Color tgfxColor(static_cast<float>(red), static_cast<float>(green), static_cast<float>(blue), static_cast<float>(alpha));
    renderer->setBackgroundColor(tgfxColor);
}

ZoomLevel SeatCanvasCoreRendererGetZoomLevel(CPPObject *_Nonnull cppObject) {
    ZoomLevel zoom{};
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, zoom);
    auto config = renderer->zoomLevelConfig();
    zoom.zoomScale9 = static_cast<CGFloat>(config.zoomScale9);
    zoom.zoomScale18 = static_cast<CGFloat>(config.zoomScale18);
    zoom.zoomScale30 = static_cast<CGFloat>(config.zoomScale30);
    zoom.zoomScale50 = static_cast<CGFloat>(config.zoomScale50);
    return zoom;
}

void SeatCanvasCoreRendererHandTap(CPPObject *_Nonnull cppObject, CGPoint location) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->handleTap({static_cast<float>(location.x), static_cast<float>(location.y)});
}

void SeatCanvasCoreRendererHandPan(CPPObject *_Nonnull cppObject, kk::gesture::GestureState state, CGPoint translation, double timestampMs) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->handlePan(state, {static_cast<float>(translation.x), static_cast<float>(translation.y)}, timestampMs);
}

void SeatCanvasCoreRendererHandPinch(CPPObject *_Nonnull cppObject, kk::gesture::GestureState state, CGFloat scale, CGPoint center) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->handlePinch(state, static_cast<float>(scale), {static_cast<float>(center.x), static_cast<float>(center.y)});
}

void SeatCanvasCoreRendererStart(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->start();
}

void SeatCanvasCoreRendererStop(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->stop();
}

void SeatCanvasCoreRendererDraw(CPPObject *_Nonnull cppObject, bool force) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->draw(force);
}

CGFloat SeatCanvasCoreRendererGetDensity(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 1.0);
    auto density = renderer->state()->getDensity();
    return static_cast<CGFloat>(density);
}

CGRect SeatCanvasCoreRendererGetVisibleContentRect(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, CGRectZero);
    auto rect = renderer->getVisibleOriginalRect();
    return CGRectMake(
        static_cast<CGFloat>(rect.x()),
        static_cast<CGFloat>(rect.y()),
        static_cast<CGFloat>(rect.width()),
        static_cast<CGFloat>(rect.height()));
}

bool SeatCanvasCoreRendererGetSeatRegionByPoint(CPPObject *_Nonnull cppObject, CGPoint targetPoint, HitTestSeatRegionResult &outResult) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, false);
    auto info = renderer->getSeatRegionDataByPoint(static_cast<float>(targetPoint.x), static_cast<float>(targetPoint.y));
    if (info == nullptr) {
        return false;
    }

    const auto &bounds = info->bounds;
    outResult.regionId = info->regionId;
    outResult.bounds = CGRectMake(
        static_cast<CGFloat>(bounds.x()),
        static_cast<CGFloat>(bounds.y()),
        static_cast<CGFloat>(bounds.width()),
        static_cast<CGFloat>(bounds.height()));
    return true;
}

void SeatCanvasCoreRendererZoomToRect(CPPObject *_Nonnull cppObject, CGRect rect, bool animated, CGFloat padding, double durationMs) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    tgfx::Rect targetRect = tgfx::Rect::MakeXYWH(static_cast<float>(rect.origin.x),
                                                 static_cast<float>(rect.origin.y),
                                                 static_cast<float>(rect.size.width),
                                                 static_cast<float>(rect.size.height));
    renderer->zoomToRect(targetRect, animated, static_cast<float>(padding), durationMs);
}

// #ifdef __cplusplus
// }
// #endif

}  // namespace kk
