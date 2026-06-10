#include "core/Log.hpp"
//
//  ApplePlatformView.mm
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "ApplePlatformView.h"

#import <tgfx/core/Surface.h>
#import <tgfx/gpu/metal/MetalWindow.h>
#import <tgfx/platform/Print.h>

#import <MetalKit/MTKView.h>

#import <cmath>

namespace kk::renderer {

ApplePlatformView::ApplePlatformView(MTKView *metalView)
    : _metalView(metalView) {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

ApplePlatformView::~ApplePlatformView() {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> ApplePlatformView::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::MetalWindow::MakeFrom(_metalView);
    }
    return _window;
}

std::shared_ptr<tgfx::Surface> ApplePlatformView::getSurface(tgfx::Context *context) {
    if (context == nullptr) {
        return nullptr;
    }

    if (!_surface && _window) {
        _surface = tgfx::Surface::MakeFrom(context, _window);
    }

    return _surface;
}

void ApplePlatformView::invalidSize() {
    _surface = nullptr;
}

tgfx::ISize ApplePlatformView::getSize() {
    CAMetalLayer *metalLayer = (CAMetalLayer *)_metalView.layer;
    auto width = static_cast<int>(roundf(static_cast<float>(metalLayer.bounds.size.width * metalLayer.contentsScale)));
    auto height = static_cast<int>(roundf(static_cast<float>(metalLayer.bounds.size.height * metalLayer.contentsScale)));
    return tgfx::ISize{width, height};
}

float ApplePlatformView::getDensity() {
    CAMetalLayer *metalLayer = (CAMetalLayer *)_metalView.layer;
    float contentsScale = static_cast<float>(metalLayer.contentsScale);
    return contentsScale;
}

};  // namespace kk::renderer
