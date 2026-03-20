//
//  OHOSPlatformView.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "OHOSPlatformView.h"

#include <tgfx/core/Surface.h>
#include <tgfx/gpu/opengl/egl/EGLWindow.h>
#include <tgfx/platform/Print.h>

namespace kk::renderer {

static float screenDensity = 1.0f;
OHOSPlatformView::OHOSPlatformView(OH_NativeXComponent *component, void *nativeWindow)
    : _component(component)
    , _nativeWindow(nativeWindow)
    , _window(nullptr)
    , _surface(nullptr) {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

OHOSPlatformView::~OHOSPlatformView() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

std::shared_ptr<tgfx::Window> OHOSPlatformView::getWindow() {
    if (_nativeWindow == nullptr) {
        return nullptr;
    }
    if (_window == nullptr) {
        _window = tgfx::EGLWindow::MakeFrom(reinterpret_cast<EGLNativeWindowType>(_nativeWindow));
    }
    return _window;
}

std::shared_ptr<tgfx::Surface> OHOSPlatformView::getSurface(tgfx::Context *context) {
    if (context == nullptr) {
        return nullptr;
    }

    if (!_surface && _window) {
        _surface = tgfx::Surface::MakeFrom(context, _window);
    }

    return _surface;
}

void OHOSPlatformView::invalidSize() {
    _surface = nullptr;
}

tgfx::ISize OHOSPlatformView::getSize() {
    if (_component == nullptr || _nativeWindow == nullptr) {
        return tgfx::ISize::MakeEmpty();
    }
    uint64_t width;
    uint64_t height;
    int32_t ret = OH_NativeXComponent_GetXComponentSize(_component, _nativeWindow, &width, &height);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        return tgfx::ISize::MakeEmpty();
    }
    return tgfx::ISize::Make(static_cast<int>(width), static_cast<int>(height));
}

float OHOSPlatformView::getDensity() {
    return screenDensity;
}

void OHOSPlatformView::UpdateDensity(float density) {
    screenDensity = density;
}
};  // namespace kk::renderer
