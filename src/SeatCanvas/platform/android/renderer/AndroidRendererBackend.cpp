//
//  AndroidRendererBackend.cpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#include "AndroidRendererBackend.hpp"

#include <tgfx/gpu/opengl/egl/EGLWindow.h>
#include <tgfx/platform/Print.h>

namespace kk::renderer {
AndroidRendererBackend::AndroidRendererBackend(ANativeWindow *nativeWindow, float density)
    : _nativeWindow(nativeWindow)
    , _density(density) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

AndroidRendererBackend::~AndroidRendererBackend() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> AndroidRendererBackend::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::EGLWindow::MakeFrom(_nativeWindow);
    }
    return _window;
}

void AndroidRendererBackend::invalidSize() {
    if (_window) {
        _window->invalidSize();
    }
}

tgfx::ISize AndroidRendererBackend::getSize() {
    if (_nativeWindow == nullptr) {
        return {0, 0};
    }

    auto width = ANativeWindow_getWidth(_nativeWindow);
    auto height = ANativeWindow_getHeight(_nativeWindow);
    return {static_cast<int>(width), static_cast<int>(height)};
}

float AndroidRendererBackend::getDensity() {
    return _density;
}

}  // namespace kk::renderer