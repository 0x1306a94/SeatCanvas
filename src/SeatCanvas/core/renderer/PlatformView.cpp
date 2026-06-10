//
//  PlatformView.cpp
//  SeatCanvas
//

#include "core/renderer/PlatformView.hpp"

#include <tgfx/gpu/Window.h>

namespace kk::renderer {

PlatformView::~PlatformView() = default;

std::shared_ptr<tgfx::Device> PlatformView::getDevice() {
    auto window = getWindow();
    return window ? window->getDevice() : nullptr;
}

void *PlatformView::nativeHandle() {
    return nullptr;
}

};  // namespace kk::renderer
