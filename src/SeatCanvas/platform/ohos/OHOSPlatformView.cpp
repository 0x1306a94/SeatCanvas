//
//  OHOSPlatformView.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "OHOSPlatformView.h"

#include <tgfx/gpu/opengl/egl/EGLWindow.h>

namespace kk::renderer {

static float screenDensity = 1.0f;
OHOSPlatformView::OHOSPlatformView(OH_NativeXComponent *component, void *nativeWindow)
    : component(component)
    , nativeWindow(nativeWindow)
    , window(nullptr) {
}
OHOSPlatformView::~OHOSPlatformView() {
}

std::shared_ptr<tgfx::Window> OHOSPlatformView::getWindow() {
    if (nativeWindow == nullptr) {
        return nullptr;
    }
    if (window == nullptr) {
        window = tgfx::EGLWindow::MakeFrom(reinterpret_cast<EGLNativeWindowType>(nativeWindow));
    }
    return window;
}

void OHOSPlatformView::invalidSize() {
    if (window) {
        window->invalidSize();
    }
}

tgfx::ISize OHOSPlatformView::getSize() {
    if (component == nullptr || nativeWindow == nullptr) {
        return tgfx::ISize::MakeEmpty();
    }
    uint64_t width;
    uint64_t height;
    int32_t ret = OH_NativeXComponent_GetXComponentSize(component, nativeWindow, &width, &height);
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
