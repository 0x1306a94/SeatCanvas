//
//  Scroller.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#include "Scroller.hpp"

namespace kk::gesture {

Scroller::Scroller(float deceleration_rate)
    : _deceleration_rate(deceleration_rate)
    , _initial_velocity(0.f) {
}

void Scroller::setDecelerationRate(float rate) {
    _deceleration_rate = rate;
}

void Scroller::fling(float velocity) {
    _initial_velocity = velocity;
}

std::optional<ScrollerValue> Scroller::value(float time) const {
    const float coefficient = std::pow(_deceleration_rate, time);
    const float velocity = _initial_velocity * coefficient;
    if (std::fabs(velocity) < VelocityThreshold) {
        return std::nullopt;
    }
    const float log_rate = std::log(_deceleration_rate);
    if (log_rate == 0.f) {
        return std::nullopt;
    }
    const float offset = _initial_velocity * (1.f / log_rate) * (coefficient - 1.f);
    return ScrollerValue{offset, velocity};
}

void Scroller::reset() {
    _initial_velocity = 0.f;
}

};  // namespace kk::gesture
