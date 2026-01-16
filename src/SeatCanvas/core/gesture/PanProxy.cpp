//
//  PanProxy.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#include "PanProxy.hpp"

#include "VelocityTracker.hpp"

namespace kk::gesture {
PanProxy::PanProxy(GestureState state)
    : _state(state)
    , _translation({0.0f, 0.0f})
    , _beginTime(0.0)
    , _cb(nullptr)
    , _velocityTrackerX(nullptr)
    , _velocityTrackerY(nullptr) {
}

void PanProxy::setCallback(CallBack cb) {
    _cb = std::move(cb);
}

GestureState PanProxy::state() const {
    return _state;
}

const tgfx::Point &PanProxy::translation() const {
    return _translation;
}

tgfx::Point PanProxy::velocity() const {
    const auto vx = (_velocityTrackerX != nullptr) ? _velocityTrackerX->calculate() : 0.0f;
    const auto vy = (_velocityTrackerY != nullptr) ? _velocityTrackerY->calculate() : 0.0f;
    return tgfx::Point{vx, vy};
}

void PanProxy::handle(GestureState state, const tgfx::Point &translation, double timestampMs) {
    switch (state) {
        case GestureState::BEGAN: {
            begin(translation, timestampMs);
            break;
        }
        case GestureState::CHANGED: {
            move(translation, timestampMs);
            break;
        }
        case GestureState::ENDED: {
            end(translation, timestampMs, false);
            break;
        }
        case GestureState::CANCELLED: {
            end(translation, timestampMs, true);
            break;
        }
        default:
            break;
    }
}

void PanProxy::reset() {
    if (_velocityTrackerX) {
        _velocityTrackerX = nullptr;
    }

    if (_velocityTrackerY) {
        _velocityTrackerY = nullptr;
    }

    _state = GestureState::POSSIBLE;
    _translation = tgfx::Point::Zero();
    _beginTime = 0.0;
}

void PanProxy::begin(const tgfx::Point &translation, double timestampMs) {
    _state = GestureState::BEGAN;
    _beginTime = timestampMs;
    ensureVelocityTrackers();
    _translation = tgfx::Point::Zero();
    addSample(timestampMs);
    doCallback();
}

void PanProxy::move(const tgfx::Point &translation, double timestampMs) {
    _state = GestureState::CHANGED;
    updateTranslation(translation);
    addSample(timestampMs);
    doCallback();
}

void PanProxy::end(const tgfx::Point &translation, double timestampMs, bool cancelled) {
    _state = cancelled ? GestureState::CANCELLED : GestureState::ENDED;
    updateTranslation(translation);
    addSample(timestampMs);
    doCallback();

    _state = GestureState::POSSIBLE;
    _translation = tgfx::Point::Zero();
    _beginTime = 0.0;
}

void PanProxy::updateTranslation(const tgfx::Point &translation) {
    _translation = translation;
}

void PanProxy::ensureVelocityTrackers() {
    if (_velocityTrackerX == nullptr) {
        _velocityTrackerX = std::make_unique<VelocityTracker>(VelocityTrackerStrategy::Recurrence);
    } else {
        _velocityTrackerX->reset();
    }

    if (_velocityTrackerY == nullptr) {
        _velocityTrackerY = std::make_unique<VelocityTracker>(VelocityTrackerStrategy::Recurrence);
    } else {
        _velocityTrackerY->reset();
    }
}

void PanProxy::addSample(double timestampMs) {
    if (_beginTime == 0.0) {
        return;
    }
    if (!_velocityTrackerX) {
        ensureVelocityTrackers();
    }

    const auto elapsed = static_cast<float>((timestampMs - _beginTime));
    _velocityTrackerX->addDataPoint(elapsed, _translation.x);
    _velocityTrackerY->addDataPoint(elapsed, _translation.y);
}

void PanProxy::doCallback() {
    if (_cb) {
        _cb();
    }
}
};  // namespace kk::gesture
