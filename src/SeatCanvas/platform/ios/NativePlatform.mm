//
//  NativePlatform.cpp
//
//
//  Created by king on 2025/11/13.
//

#import "NativePlatform.h"

#import "NativeDisplayLink.h"

namespace kk {
const Platform *Platform::Current() {
    static const NativePlatform platform = {};
    return &platform;
}

std::shared_ptr<DisplayLink> NativePlatform::createDisplayLink(std::function<void()> callback) const {
    return std::make_shared<NativeDisplayLink>(std::move(callback));
}
};  // namespace kk
