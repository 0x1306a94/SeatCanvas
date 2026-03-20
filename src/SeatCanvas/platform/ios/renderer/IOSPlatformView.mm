//
//  IOSPlatformView.mm
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "IOSPlatformView.h"

#import <tgfx/core/Surface.h>
#import <tgfx/gpu/opengl/eagl/EAGLWindow.h>
#import <tgfx/platform/Print.h>

#import <cmath>

namespace kk::renderer {

IOSPlatformView::IOSPlatformView(CAEAGLLayer *eagLayer)
    : _eagLayer(eagLayer) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

IOSPlatformView::~IOSPlatformView() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> IOSPlatformView::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::EAGLWindow::MakeFrom(_eagLayer);
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
    auto width = static_cast<int>(roundf(static_cast<float>(_eagLayer.bounds.size.width * _eagLayer.contentsScale)));
    auto height = static_cast<int>(roundf(static_cast<float>(_eagLayer.bounds.size.height * _eagLayer.contentsScale)));
    return tgfx::ISize{width, height};
}

float IOSPlatformView::getDensity() {
    float contentsScale = static_cast<float>(_eagLayer.contentsScale);
    return contentsScale;
}

};  // namespace kk::renderer
