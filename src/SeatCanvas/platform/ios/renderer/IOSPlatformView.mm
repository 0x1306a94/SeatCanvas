//
//  IOSPlatformView.mm
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "IOSPlatformView.h"

#import <tgfx/core/Surface.h>
#import <tgfx/gpu/metal/MetalWindow.h>
#import <tgfx/platform/Print.h>

#import <MetalKit/MTKView.h>

#import <cmath>

namespace kk::renderer {

IOSPlatformView::IOSPlatformView(MTKView *metalView)
    : _metalView(metalView) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

IOSPlatformView::~IOSPlatformView() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> IOSPlatformView::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::MetalWindow::MakeFrom(_metalView);
    }
    return _window;
}

std::shared_ptr<tgfx::Surface> IOSPlatformView::getSurface(tgfx::Context *context) {
    if (context == nullptr) {
        return nullptr;
    }

    if (!_surface && _window) {
        _surface = tgfx::Surface::MakeFrom(context, _window);
    }

    return _surface;
}

void IOSPlatformView::invalidSize() {
    _surface = nullptr;
}

tgfx::ISize IOSPlatformView::getSize() {
    CAMetalLayer *metalLayer = (CAMetalLayer *)_metalView.layer;
    auto width = static_cast<int>(roundf(static_cast<float>(metalLayer.bounds.size.width * metalLayer.contentsScale)));
    auto height = static_cast<int>(roundf(static_cast<float>(metalLayer.bounds.size.height * metalLayer.contentsScale)));
    return tgfx::ISize{width, height};
}

float IOSPlatformView::getDensity() {
    CAMetalLayer *metalLayer = (CAMetalLayer *)_metalView.layer;
    float contentsScale = static_cast<float>(metalLayer.contentsScale);
    return contentsScale;
}

};  // namespace kk::renderer
