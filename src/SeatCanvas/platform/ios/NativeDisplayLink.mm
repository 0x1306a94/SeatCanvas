#include "core/Log.hpp"
//
//  NativeDisplayLink.mm
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#import "NativeDisplayLink.h"

#import "platform/apple/InternalAnimationCallback.h"

#import <Foundation/NSRunLoop.h>

#import <tgfx/platform/Print.h>

namespace kk {
NativeDisplayLink::NativeDisplayLink(std::function<void()> callback) {
    animationCallback = [[InternalAnimationCallback alloc] initWithCallback:callback];
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

NativeDisplayLink::~NativeDisplayLink() {
    stop();
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}

void NativeDisplayLink::start() {
    if (displayLink != nullptr) {
        return;
    }
    displayLink = [CADisplayLink displayLinkWithTarget:animationCallback selector:@selector(update:)];
    // The default mode was previously set here. However, rendering is not possible when the UI is in
    // drag mode. Therefore, it has been changed to common modes.
    [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    if (@available(iOS 15.0, *)) {
        displayLink.preferredFrameRateRange = CAFrameRateRangeMake(60, 120, 120);
    } else {
        displayLink.preferredFrameRateRange = CAFrameRateRangeDefault;
    }
}

void NativeDisplayLink::stop() {
    if (displayLink == nullptr) {
        return;
    }
    [displayLink invalidate];
    displayLink = nullptr;
}
};  // namespace kk
