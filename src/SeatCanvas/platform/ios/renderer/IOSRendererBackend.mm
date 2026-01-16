//
//  IOSRendererBackend.mm
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#import "IOSRendererBackend.h"

#import <tgfx/gpu/opengl/eagl/EAGLWindow.h>
#include <tgfx/platform/Print.h>

#import <cmath>

namespace kk::renderer {

IOSRendererBackend::IOSRendererBackend(CAEAGLLayer *eagLayer)
    : _eagLayer(eagLayer) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

IOSRendererBackend::~IOSRendererBackend() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> IOSRendererBackend::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::EAGLWindow::MakeFrom(_eagLayer);
    }
    return _window;
}

void IOSRendererBackend::invalidSize() {
    if (_window) {
        _window->invalidSize();
    }
}

tgfx::ISize IOSRendererBackend::getSize() {
    auto width = static_cast<int>(roundf(static_cast<float>(_eagLayer.bounds.size.width * _eagLayer.contentsScale)));
    auto height = static_cast<int>(roundf(static_cast<float>(_eagLayer.bounds.size.height * _eagLayer.contentsScale)));
    return tgfx::ISize{width, height};
}

float IOSRendererBackend::getDensity() {
    float contentsScale = static_cast<float>(_eagLayer.contentsScale);
    return contentsScale;
}

};  // namespace kk::renderer
