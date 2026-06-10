#include "HeadlessTestPlatformView.hpp"

#include <tgfx/core/Surface.h>
#include <tgfx/gpu/metal/MetalDevice.h>

namespace kk::test {

HeadlessTestPlatformView::HeadlessTestPlatformView(int width, int height, float density)
    : _device(tgfx::MetalDevice::Make())
    , _size{width, height}
    , _density(density) {
}

HeadlessTestPlatformView::~HeadlessTestPlatformView() = default;

std::shared_ptr<tgfx::Device> HeadlessTestPlatformView::getDevice() {
    return _device;
}

std::shared_ptr<tgfx::Window> HeadlessTestPlatformView::getWindow() {
    return nullptr;
}

std::shared_ptr<tgfx::Surface> HeadlessTestPlatformView::getSurface(tgfx::Context *context) {
    if (context == nullptr) {
        return nullptr;
    }
    if (_surface == nullptr) {
        _surface = tgfx::Surface::Make(context, _size.width, _size.height);
    }
    return _surface;
}

void HeadlessTestPlatformView::invalidSize() {
    _surface = nullptr;
}

tgfx::ISize HeadlessTestPlatformView::getSize() {
    return _size;
}

float HeadlessTestPlatformView::getDensity() {
    return _density;
}

};  // namespace kk::test
