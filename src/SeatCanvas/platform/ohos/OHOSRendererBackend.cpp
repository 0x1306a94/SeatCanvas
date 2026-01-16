//
//  OHOSRendererBackend.cpp
//  SeatCanvas
//
//  Created by KK on 2025/12/1.
//

#include "OHOSRendererBackend.h"

#include <tgfx/gpu/opengl/egl/EGLWindow.h>

namespace kk::renderer {

static float screenDensity = 1.0f;
OHOSRendererBackend::OHOSRendererBackend(OH_NativeXComponent *component, void *nativeWindow)
    : component(component)
    , nativeWindow(nativeWindow)
    , window(nullptr) {
}
OHOSRendererBackend::~OHOSRendererBackend() {
}

std::shared_ptr<tgfx::Window> OHOSRendererBackend::getWindow() {
    if (nativeWindow == nullptr) {
        return nullptr;
    }
    if (window == nullptr) {
        window = tgfx::EGLWindow::MakeFrom(reinterpret_cast<EGLNativeWindowType>(nativeWindow));
    }
    return window;
}

void OHOSRendererBackend::invalidSize() {
    if (window) {
        window->invalidSize();
    }
}

tgfx::ISize OHOSRendererBackend::getSize() {
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

float OHOSRendererBackend::getDensity() {
    return screenDensity;
}

void OHOSRendererBackend::UpdateDensity(float density) {
    screenDensity = density;
}
};  // namespace kk::renderer
