//
//  NativeDisplayLink.h
//  SeatCanvas
//
//  Created by KK on 2025/11/16.
//

#ifndef NativeDisplayLink_h
#define NativeDisplayLink_h

#import <QuartzCore/CADisplayLink.h>
#import <functional>

#import "core/utils/DisplayLink.hpp"

@class InternalAnimationCallback;

namespace kk {
class NativeDisplayLink : public DisplayLink {
  public:
    explicit NativeDisplayLink(std::function<void()> callback);
    ~NativeDisplayLink() override;

    void start() override;
    void stop() override;

  private:
    CADisplayLink *_displayLink = nullptr;
    InternalAnimationCallback *_animationCallback = nullptr;
};
};  // namespace kk

#endif /* NativeDisplayLink_h */
