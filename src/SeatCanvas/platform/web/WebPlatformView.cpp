//
//  WebPlatformView.cpp
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

#include "WebPlatformView.hpp"

#include <emscripten/html5.h>
#include <emscripten/val.h>

#include <tgfx/core/Surface.h>
#include <tgfx/gpu/opengl/webgl/WebGLWindow.h>
#include <tgfx/platform/Print.h>

namespace kk::renderer {

WebPlatformView::WebPlatformView(const std::string &canvasID)
    : _canvasID(canvasID) {
    refreshDensity();
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

WebPlatformView::~WebPlatformView() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
    _surface = nullptr;
    _window = nullptr;
}

std::shared_ptr<tgfx::Window> WebPlatformView::getWindow() {
    if (_window == nullptr) {
        _window = tgfx::WebGLWindow::MakeFrom(_canvasID);
    }
    return _window;
}

std::shared_ptr<tgfx::Surface> WebPlatformView::getSurface(tgfx::Context *context) {
    if (context == nullptr) {
        return nullptr;
    }
    if (_surface == nullptr && _window != nullptr) {
        _surface = tgfx::Surface::MakeFrom(context, _window);
    }
    return _surface;
}

void WebPlatformView::invalidSize() {
    _surface = nullptr;
}

tgfx::ISize WebPlatformView::getSize() {
    refreshDensity();
    double width = 0.0;
    double height = 0.0;
    int result = emscripten_get_element_css_size(_canvasID.c_str(), &width, &height);
    if (result != EMSCRIPTEN_RESULT_SUCCESS) {
        return tgfx::ISize::MakeEmpty();
    }
    return tgfx::ISize::Make(static_cast<int>(width * _density), static_cast<int>(height * _density));
}

float WebPlatformView::getDensity() {
    refreshDensity();
    return _density;
}

void WebPlatformView::refreshDensity() {
    float density = emscripten::val::global("window")["devicePixelRatio"].as<float>();
    if (density <= 0.0f) {
        density = 1.0f;
    }
    _density = density;
}

};  // namespace kk::renderer
