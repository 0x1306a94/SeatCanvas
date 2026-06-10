#include "core/Log.hpp"
//
//  InternalAnimationCallback.m
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#import "InternalAnimationCallback.h"

@implementation InternalAnimationCallback
#if DEBUG
- (void)dealloc {
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
}
#endif

- (instancetype)initWithCallback:(std::function<void()>)animationCallback {
    self = [super init];
    if (self) {
        self->callback = animationCallback;
    }
#if DEBUG
    SC_LOG_TRACE(__PRETTY_FUNCTION__);
#endif
    return self;
}

- (void)update:(CADisplayLink *)sender {
#if DEBUG
    CFTimeInterval frameInterval = sender.targetTimestamp - sender.timestamp;
    if (frameInterval > 0.02) {  // 超过20ms(低于50fps)
        SC_LOG_WARN("Frame drop detected: %.2fms", (frameInterval * 1000));
    }
#endif

    @autoreleasepool {
        callback();
    }
}
@end
