//
//  NativeDisplayLink.hpp
//  SeatCanvas
//
//  Created by KK on 2026/5/26.
//

#ifndef NativeDisplayLink_hpp
#define NativeDisplayLink_hpp

#include <atomic>
#include <functional>
#include <memory>

#include <emscripten/html5.h>

#include "core/utils/DisplayLink.hpp"

namespace kk {
class NativeDisplayLink : public DisplayLink {
  public:
    static std::shared_ptr<DisplayLink> Make(std::function<void()> callback);
    ~NativeDisplayLink() override;

    void start() override;
    void stop() override;

  private:
    explicit NativeDisplayLink(std::function<void()> callback);

    static EM_BOOL OnFrame(double time, void *userData);

    void cancelAnimationFrame();

    std::function<void()> _callback = {nullptr};
    std::atomic<bool> _started = {false};
    long _animationFrameId = {-1};
};
};  // namespace kk

#endif /* NativeDisplayLink_hpp */
