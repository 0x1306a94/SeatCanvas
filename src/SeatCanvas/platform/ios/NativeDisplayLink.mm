//
//  NativeDisplayLink.mm
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#import "NativeDisplayLink.h"

#import "InternalAnimationCallback.h"

#import <Foundation/NSRunLoop.h>

#import <tgfx/platform/Print.h>

namespace kk {
NativeDisplayLink::NativeDisplayLink(std::function<void()> callback) {
    _animationCallback = [[InternalAnimationCallback alloc] initWithCallback:callback];
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

NativeDisplayLink::~NativeDisplayLink() {
    stop();
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void NativeDisplayLink::start() {
    if (_displayLink != nullptr) {
        return;
    }
    _displayLink = [CADisplayLink displayLinkWithTarget:_animationCallback selector:@selector(update:)];
    // The default mode was previously set here. However, rendering is not possible when the UI is in
    // drag mode. Therefore, it has been changed to common modes.
    [_displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    if (@available(iOS 15.0, *)) {
        _displayLink.preferredFrameRateRange = CAFrameRateRangeMake(60, 120, 120);
    } else {
        _displayLink.preferredFrameRateRange = CAFrameRateRangeDefault;
    }
}

void NativeDisplayLink::stop() {
    if (_displayLink == nullptr) {
        return;
    }
    [_displayLink invalidate];
    _displayLink = nullptr;
}
};  // namespace kk
