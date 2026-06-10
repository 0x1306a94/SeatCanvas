//
//  InternalAnimationCallback.h
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/CADisplayLink.h>
#import <functional>

NS_ASSUME_NONNULL_BEGIN

@interface InternalAnimationCallback : NSObject {
    std::function<void()> callback;
}

- (instancetype)initWithCallback:(std::function<void()>)callback;

- (void)update:(CADisplayLink *)sender;

@end

NS_ASSUME_NONNULL_END
