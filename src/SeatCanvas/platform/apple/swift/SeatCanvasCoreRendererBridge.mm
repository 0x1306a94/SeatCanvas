//
//  SeatCanvasCoreRendererBridge.mm
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "SeatCanvasCoreRendererBridge.h"

#import "ColorCast.h"
#import "core/BaseMapConfig.hpp"
#import "core/Platform.hpp"
#import "core/SeatData.hpp"
#import "core/gesture/ElasticZoomPanController.hpp"
#import "core/layers/BaseMapRootLayer.hpp"
#import "core/parser/BaseMapFormat.hpp"
#import "core/parser/BaseMapLoadResult.hpp"
#import "core/parser/BaseMapParserFactory.hpp"
#import "core/renderer/SeatCanvasCoreRenderer.hpp"
#import "core/renderer/SeatCanvasCoreRendererState.hpp"
#import "core/svg/ConvertSVGLayer.hpp"
#import "core/svg/SVGMeshParser.hpp"
#import "core/utils/SystemProperties.hpp"
#import "core/utils/TimeProfiler.hpp"
#import "platform/apple/LayerPrerenderImage.h"
#import "platform/ios/renderer/IOSPlatformView.h"

#import "SwiftSeatCanvasCoreRendererDelegate.hpp"

#import <tgfx/core/Data.h>
#import <tgfx/core/Stream.h>
#import <tgfx/platform/Print.h>
#import <tgfx/svg/SVGDOM.h>

#include <mutex>

#include <tgfx/core/Canvas.h>
#include <tgfx/core/Paint.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/metal/MetalDevice.h>

#include "core/DeviceLockGuard.hpp"

namespace kk::bridge {
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

void SeatCanvasCoreRendererPrewarmShaderCompiler(void) {
    static std::once_flag onceFlag = {};
    std::call_once(onceFlag, [] {
        // TODO: Remove this temporary warmup path after shader compiler init is moved to renderer startup.
        static std::shared_ptr<tgfx::MetalDevice> warmupDevice = nullptr;
        static std::shared_ptr<tgfx::Surface> warmupSurface = nullptr;

        warmupDevice = tgfx::MetalDevice::Make();
        if (warmupDevice == nullptr) {
            return;
        }

        kk::DeviceLockGuard deviceLockGuard(warmupDevice.get());
        auto *context = deviceLockGuard.context();
        if (context == nullptr) {
            return;
        }

        warmupSurface = tgfx::Surface::Make(context, 8, 8, tgfx::ColorType::RGBA_8888);
        if (warmupSurface == nullptr) {
            return;
        }

        auto *canvas = warmupSurface->getCanvas();
        if (canvas == nullptr) {
            return;
        }

        tgfx::Paint paint = {};
        paint.setColor(tgfx::Color::White());
        canvas->drawRect(tgfx::Rect::MakeWH(4.0f, 4.0f), paint);
        context->flushAndSubmit(true);
    });
}

CPPObject *_Nonnull CreateSeatCanvasCoreRenderer(MTKView *_Nullable view) {
    auto platformView = std::make_unique<kk::renderer::IOSPlatformView>(view);
    auto zoomPanController = std::make_unique<kk::gesture::ElasticZoomPanController>();
    auto renderer = new kk::renderer::SeatCanvasCoreRenderer(std::move(platformView), std::move(zoomPanController));

    auto delegate = std::make_shared<SwiftSeatCanvasCoreRendererDelegate>();
    renderer->setDelegate(std::move(delegate));

    CPPObject *cppObj = AllocCPPObject(ObjectTag::CoreRenderer, renderer, CPPObjectDeleter<kk::renderer::SeatCanvasCoreRenderer>);
    return cppObj;
}

uint32_t SeatCanvasCoreRendererGetCoreID(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 0);
    return renderer->coreID();
}

bool SeatCanvasCoreRendererReplacePlatformView(CPPObject *_Nonnull cppObject, MTKView *_Nullable view) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, false);
    if (view == nullptr) {
        renderer->replacePlatformView(nullptr);
        return true;
    }
    auto platformView = std::make_unique<kk::renderer::IOSPlatformView>(view);
    renderer->replacePlatformView(std::move(platformView));
    return true;
}

void *_Nullable SeatCanvasCoreRendererParseBaseMap(const void *_Nullable __sized_by_or_null(len) bytes, size_t len, kk::parser::BaseMapFormat format,
                                                   const void *_Nullable __sized_by_or_null(parseConfigLen) parseConfigBytes, size_t parseConfigLen,
                                                   UIImage *_Nullable *_Nullable miniMapImage) {
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
    std::shared_ptr<tgfx::Data> parseConfigData = nullptr;
    if (parseConfigBytes != nullptr && parseConfigLen > 0) {
        parseConfigData = tgfx::Data::MakeWithCopy(parseConfigBytes, parseConfigLen);
    }
    auto result = kk::parser::BaseMapParserFactory::parse(data, format, parseConfigData);
    if (!result) {
        tgfx::PrintError("parse result is null");
        return nullptr;
    }

    auto outResult = new kk::parser::BaseMapLoadResult(std::move(result));
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

void *_Nullable SeatCanvasCoreRendererParseBaseMapFromSVG(const void *_Nullable __sized_by_or_null(len) bytes, size_t len, UIImage *_Nullable *_Nullable miniMapImage) {
    return SeatCanvasCoreRendererParseBaseMap(bytes, len, kk::parser::BaseMapFormat::SVG, nullptr, 0, miniMapImage);
}

bool SeatCanvasCoreRendererLoadBaseMap(CPPObject *_Nonnull cppObject, void *_Nullable *_Nullable loadResult) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, false);
    if (loadResult == nullptr || *loadResult == nullptr) {
        renderer->setBaseMapConfig(nullptr);
        return true;
    }

    auto map = static_cast<kk::parser::BaseMapLoadResult *>(*loadResult);
    if (map == nullptr) {
        *loadResult = nullptr;
        return false;
    }

    auto baseMapConfig = map->makeBaseMapConfig();
    renderer->setBaseMapConfig(std::move(baseMapConfig));

    delete map;
    *loadResult = nullptr;

    return true;
}

void SeatCanvasCoreRendererSetSeatZoneAlternateColors(CPPObject *_Nonnull cppObject, NSDictionary<NSString *, UIColor *> *_Nullable colors) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    if (colors == nil) {
        renderer->updateSeatZoneAlternateColors({});
        return;
    }
    std::unordered_map<std::string, tgfx::Color> cppColors{};
    cppColors.reserve(colors.count);
    for (NSString *zoneId in colors.keyEnumerator) {
        UIColor *color = colors[zoneId];
        if (!color) {
            continue;
        }
        cppColors.emplace(std::string(zoneId.UTF8String), UIColorToTGFX(color));
    }

    renderer->updateSeatZoneAlternateColors(cppColors);
}

void SeatCanvasCoreRendererSetMiniMapZoneAlternateColors(CPPObject *_Nonnull cppObject, NSDictionary<NSString *, UIColor *> *_Nullable colors) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    if (colors == nil) {
        renderer->updateMiniMapZoneAlternateColors({});
        return;
    }
    std::unordered_map<std::string, tgfx::Color> cppColors{};
    cppColors.reserve(colors.count);
    for (NSString *zoneId in colors.keyEnumerator) {
        UIColor *color = colors[zoneId];
        if (!color) {
            continue;
        }
        cppColors.emplace(std::string(zoneId.UTF8String), UIColorToTGFX(color));
    }

    renderer->updateMiniMapZoneAlternateColors(cppColors);
}

void SeatCanvasCoreRendererSetSeatDatas(CPPObject *_Nonnull cppObject, const std::string &zoneId, CPPObject *_Nullable seatBuilder) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    if (seatBuilder == nullptr) {
        return;
    }
    auto seats = static_cast<std::vector<kk::SeatData> *>(seatBuilder->realValue);
    renderer->setSeatData(zoneId, *seats);
}

void SeatCanvasCoreRendererClearSeatData(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->clearSeatData();
}

void SeatCanvasCoreRendererSetSeatStyleJSONConfig(CPPObject *_Nonnull cppObject, const void *_Nullable __sized_by_or_null(len) bytes, size_t len) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->setStyleKeyToConfigFromJSON(bytes, len);
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

UIColor *SeatCanvasCoreRendererGetBackgroundColor(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, UIColor.clearColor);
    const auto &backgroundColor = renderer->getBackgroundColor();
    return UIColorFromTGFX(backgroundColor);
}

CGFloat SeatCanvasCoreRendererGetSeatSize(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 36.0);
    return static_cast<CGFloat>(renderer->getSeatSize());
}

void SeatCanvasCoreRendererSetSeatSize(CPPObject *_Nonnull cppObject, CGFloat seatSize) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->setSeatSize(static_cast<float>(seatSize));
}

ZoomLevel SeatCanvasCoreRendererGetZoomLevel(CPPObject *_Nonnull cppObject) {
    ZoomLevel zoom{};
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, zoom);
    auto config = renderer->zoomLevelConfig();
    zoom.seat = static_cast<CGFloat>(config.seat);
    zoom.row = static_cast<CGFloat>(config.row);
    zoom.zone = static_cast<CGFloat>(config.zone);
    zoom.venue = static_cast<CGFloat>(config.venue);
    return zoom;
}

CGFloat SeatCanvasCoreRendererGetSeatRenderZoomThreshold(CPPObject *_Nonnull cppObject) {
    GetCPPObjectOrReturnValue(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer, 0.0);
    return static_cast<CGFloat>(renderer->getSeatRenderZoomThreshold());
}

void SeatCanvasCoreRendererSetSeatRenderZoomThreshold(CPPObject *_Nonnull cppObject, CGFloat zoomThreshold) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    renderer->setSeatRenderZoomThreshold(static_cast<float>(zoomThreshold));
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

void SeatCanvasCoreRendererZoomToRect(CPPObject *_Nonnull cppObject, CGRect rect, bool animated, CGFloat padding, double durationMs) {
    GetCPPObjectOrReturn(cppObject, kk::renderer::SeatCanvasCoreRenderer *, renderer);
    tgfx::Rect targetRect = tgfx::Rect::MakeXYWH(static_cast<float>(rect.origin.x),
                                                 static_cast<float>(rect.origin.y),
                                                 static_cast<float>(rect.size.width),
                                                 static_cast<float>(rect.size.height));
    renderer->zoomToRect(targetRect, animated, static_cast<float>(padding), durationMs);
}
};  // namespace kk::bridge
