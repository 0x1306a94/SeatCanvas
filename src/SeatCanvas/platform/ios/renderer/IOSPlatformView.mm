//
//  IOSPlatformView.mm
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "IOSPlatformView.h"

#import <tgfx/gpu/opengl/eagl/EAGLWindow.h>
#include <tgfx/platform/Print.h>

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

void IOSPlatformView::invalidSize() {
    if (_window) {
        _window->invalidSize();
    }
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
