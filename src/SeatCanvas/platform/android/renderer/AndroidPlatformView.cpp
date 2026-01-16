//
//  AndroidPlatformView.cpp
//  SeatCanvas
//
//  Created by KK on 2025/11/24.
//

#include "AndroidPlatformView.hpp"

#include <tgfx/gpu/opengl/egl/EGLWindow.h>
#include <tgfx/platform/Print.h>

namespace kk::renderer {
AndroidPlatformView::AndroidPlatformView(ANativeWindow *nativeWindow, float density)
    : _nativeWindow(nativeWindow)
    , _density(density) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

AndroidPlatformView::~AndroidPlatformView() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> AndroidPlatformView::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::EGLWindow::MakeFrom(_nativeWindow);
    }
    return _window;
}

void AndroidPlatformView::invalidSize() {
    if (_window) {
        _window->invalidSize();
    }
}

tgfx::ISize AndroidPlatformView::getSize() {
    if (_nativeWindow == nullptr) {
        return {0, 0};
    }

    auto width = ANativeWindow_getWidth(_nativeWindow);
    auto height = ANativeWindow_getHeight(_nativeWindow);
    return {static_cast<int>(width), static_cast<int>(height)};
}

float AndroidPlatformView::getDensity() {
    return _density;
}

}  // namespace kk::renderer
