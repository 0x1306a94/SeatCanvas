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
@class MTKView;
namespace kk {
class NativeDisplayLink : public DisplayLink {
  public:
    explicit NativeDisplayLink(std::function<void()> callback, MTKView *view);
    ~NativeDisplayLink() override;

    void start() override;
    void stop() override;

  private:
    MTKView *view = nullptr;
    CADisplayLink *displayLink = nullptr;
    InternalAnimationCallback *animationCallback = nullptr;
};
};  // namespace kk

#endif /* NativeDisplayLink_h */
