//
//  NativePlatform.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/13.
//

#include "NativePlatform.hpp"

#include "NativeDisplayLink.hpp"

namespace kk {

const Platform *Platform::Current() {
    static const NativePlatform platform = {};
    return &platform;
}

bool NativePlatform::registerFallbackFonts() const {
    // Since it is not possible to call ArkTs code from C++, the registration of system fonts on the
    // HarmonyOS platform is handled at the ArkTs code level.
    return false;
}

std::shared_ptr<DisplayLink> NativePlatform::createDisplayLink(std::function<void()> callback, void *userInfo) const {
    if (!callback) {
        return nullptr;
    }
    return std::make_shared<NativeDisplayLink>(callback);
}
};  // namespace kk
