//
//  NativePlatform.cpp
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

#include "NativePlatform.hpp"

#include "NativeDisplayLink.hpp"

namespace kk {

const Platform *Platform::Current() {
    static const NativePlatform platform = {};
    return &platform;
}

bool NativePlatform::registerFallbackFonts() const {
    // Font registration is handled from the JavaScript side via
    // SeatCanvasRenderer.RegisterFonts() before the renderer is created.
    return false;
}

std::shared_ptr<DisplayLink> NativePlatform::createDisplayLink(std::function<void()> callback, void *userInfo) const {
    if (!callback) {
        return nullptr;
    }
    return NativeDisplayLink::Make(std::move(callback));
}
};  // namespace kk
