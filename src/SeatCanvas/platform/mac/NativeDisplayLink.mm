//
//  NativeDisplayLink.mm
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#import "NativeDisplayLink.h"

#import "platform/apple/InternalAnimationCallback.h"

#import <Foundation/NSRunLoop.h>
#import <MetalKit/MTKView.h>

#import <tgfx/platform/Print.h>

namespace kk {
NativeDisplayLink::NativeDisplayLink(std::function<void()> callback, MTKView *view)
    : view(view) {
    animationCallback = [[InternalAnimationCallback alloc] initWithCallback:callback];
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

NativeDisplayLink::~NativeDisplayLink() {
    stop();
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void NativeDisplayLink::start() {
    if (view == nullptr) {
        return;
    }

    if (displayLink != nullptr) {
        return;
    }
    displayLink = [view displayLinkWithTarget:animationCallback selector:@selector(update:)];
    // The default mode was previously set here. However, rendering is not possible when the UI is in
    // drag mode. Therefore, it has been changed to common modes.
    [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
}

void NativeDisplayLink::stop() {
    if (displayLink == nullptr) {
        return;
    }
    [displayLink invalidate];
    displayLink = nullptr;
}
};  // namespace kk
