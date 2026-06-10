//
//  LayerPrerenderImage.mm
//  SeatCanvas
//
//  Created by king on 2025/11/19.
//

#include "LayerPrerenderImage.h"

#import <CoreGraphics/CoreGraphics.h>

#include <tgfx/core/Bitmap.h>
#include <tgfx/core/Buffer.h>
#include <tgfx/core/ImageCodec.h>
#include <tgfx/core/Surface.h>
#include <tgfx/gpu/metal/MetalDevice.h>
#include <tgfx/layers/DisplayList.h>
#include <tgfx/layers/Layer.h>

namespace kk::renderer {
CGImageRef LayerPrerenderImage::Render(const std::shared_ptr<tgfx::Layer> &layer, const tgfx::Size &sourceSize, int targetWidth) {
    if (!layer || targetWidth == 0) {
        return nullptr;
    }
    auto device = tgfx::MetalDevice::Make();
    if (!device) {
        return nullptr;
    }

    auto context = device->lockContext();
    if (!context) {
        return nullptr;
    }

    auto ratio = static_cast<float>(targetWidth / sourceSize.width);
    auto targetHeight = static_cast<int>(std::ceil(sourceSize.height * ratio));

    auto scaleX = static_cast<float>(targetWidth) / sourceSize.width;
    auto scaleY = static_cast<float>(targetHeight) / sourceSize.height;
    auto fitScale = std::min(scaleX, scaleY);

    auto surface = tgfx::Surface::Make(context, static_cast<int>(std::ceil(targetWidth)), static_cast<int>(std::ceil(targetHeight)));
    if (!surface) {
        device->unlock();
        return nullptr;
    }

    auto root = tgfx::Layer::Make();
    root->addChild(layer);
    root->setMatrix(tgfx::Matrix::MakeScale(fitScale));
    tgfx::DisplayList displayList{};
    displayList.root()->addChild(root);
    displayList.setRenderMode(tgfx::RenderMode::Direct);
    displayList.render(surface.get(), true);

    layer->removeFromParent();

    auto RGBAInfo = tgfx::ImageInfo::Make(surface->width(), surface->height(), tgfx::ColorType::RGBA_8888, tgfx::AlphaType::Premultiplied);
    tgfx::Buffer buffer(RGBAInfo.byteSize());
    auto pixels = buffer.data();

    if (!surface->readPixels(RGBAInfo, pixels)) {
        device->unlock();
        return nullptr;
    }
    device->unlock();

    // 创建 CGDataProvider
    CFDataRef data = CFDataCreate(kCFAllocatorDefault, static_cast<const UInt8 *>(pixels), RGBAInfo.byteSize());
    if (!data) {
        return nullptr;
    }

    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CFRelease(data);
    if (!provider) {
        return nullptr;
    }

    // 创建 CGImage
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;

    CGImageRef cgImage = CGImageCreate(
        static_cast<size_t>(RGBAInfo.width()),   // width
        static_cast<size_t>(RGBAInfo.height()),  // height
        8,                                       // bitsPerComponent
        32,                                      // bitsPerPixel
        RGBAInfo.rowBytes(),                     // bytesPerRow
        colorSpace,                              // colorSpace
        bitmapInfo,                              // bitmapInfo
        provider,                                // provider
        nullptr,                                 // decode
        false,                                   // shouldInterpolate
        kCGRenderingIntentDefault                // intent
    );

    CGColorSpaceRelease(colorSpace);
    CGDataProviderRelease(provider);

    return cgImage;
};

std::shared_ptr<tgfx::Image> LayerPrerenderImage::RenderImage(const std::shared_ptr<tgfx::Layer> &layer, const tgfx::Size &sourceSize, int targetWidth) {
    if (!layer || targetWidth == 0) {
        return nullptr;
    }
    auto device = tgfx::MetalDevice::Make();
    if (!device) {
        return nullptr;
    }

    auto context = device->lockContext();
    if (!context) {
        return nullptr;
    }

    auto ratio = static_cast<float>(targetWidth / sourceSize.width);
    auto targetHeight = static_cast<int>(std::ceil(sourceSize.height * ratio));

    auto scaleX = static_cast<float>(targetWidth) / sourceSize.width;
    auto scaleY = static_cast<float>(targetHeight) / sourceSize.height;
    auto fitScale = std::min(scaleX, scaleY);

    auto surface = tgfx::Surface::Make(context, targetWidth, targetHeight);
    if (!surface) {
        device->unlock();
        return nullptr;
    }

    auto root = tgfx::Layer::Make();
    root->addChild(layer);
    root->setMatrix(tgfx::Matrix::MakeScale(fitScale));
    tgfx::DisplayList displayList{};
    displayList.root()->addChild(root);
    displayList.setRenderMode(tgfx::RenderMode::Direct);
    displayList.render(surface.get(), true);

    layer->removeFromParent();

    auto RGBAInfo = tgfx::ImageInfo::Make(surface->width(), surface->height(), tgfx::ColorType::RGBA_8888, tgfx::AlphaType::Premultiplied);
    tgfx::Buffer buffer(RGBAInfo.byteSize());
    auto pixels = buffer.data();

    if (!surface->readPixels(RGBAInfo, pixels)) {
        device->unlock();
        return nullptr;
    }
    device->unlock();

    auto image = tgfx::Image::MakeFrom(RGBAInfo, buffer.release());
    return image;
};
};  // namespace kk::renderer
