//
//  PanProxy.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef PanProxy_hpp
#define PanProxy_hpp

#include "GestureState.hpp"

#include <tgfx/core/Point.h>

#include <functional>
#include <memory>

namespace kk::gesture {
class VelocityTracker;
class PanProxy {
  public:
    using CallBack = std::function<void()>;

    explicit PanProxy(GestureState state = GestureState::POSSIBLE);

    ~PanProxy() = default;

    void setCallback(CallBack cb);

    GestureState state() const;

    const tgfx::Point &translation() const;

    tgfx::Point velocity() const;

    void handle(GestureState state, const tgfx::Point &translation, double timestampMs);

    void reset();

  private:
    void begin(const tgfx::Point &translation, double timestampMs);
    void move(const tgfx::Point &translation, double timestampMs);
    void end(const tgfx::Point &translation, double timestampMs, bool cancelled);
    void updateTranslation(const tgfx::Point &translation);
    void ensureVelocityTrackers();
    void addSample(double timestampMs);
    void doCallback();

  private:
    GestureState _state;
    tgfx::Point _translation;
    double _beginTime;
    CallBack _cb;
    std::unique_ptr<VelocityTracker> _velocityTrackerX;
    std::unique_ptr<VelocityTracker> _velocityTrackerY;
};
};  // namespace kk::gesture

#endif /* PanProxy_hpp */
