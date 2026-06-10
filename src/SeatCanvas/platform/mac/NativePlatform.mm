//
//  NativePlatform.cpp
//
//
//  Created by king on 2025/11/13.
//

#import "NativePlatform.h"

#import "NativeDisplayLink.h"

#import <MetalKit/MTKView.h>

namespace kk {
const Platform *Platform::Current() {
    static const NativePlatform platform = {};
    return &platform;
}

std::shared_ptr<DisplayLink> NativePlatform::createDisplayLink(std::function<void()> callback, void *userInfo) const {
    if (userInfo == nullptr) {
        return nullptr;
    }

    if (!callback) {
        return nullptr;
    }
    return std::make_shared<NativeDisplayLink>(std::move(callback), (__bridge MTKView *)userInfo);
}
};  // namespace kk
