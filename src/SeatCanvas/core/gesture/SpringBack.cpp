//
//  SpringBack.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#include "SpringBack.hpp"

#include "GestureConstant.hpp"

namespace kk::gesture {

SpringBack::SpringBack()
    : _lambda(0.f)
    , _c1(0.f)
    , _c2(0.f) {
}

void SpringBack::absorb(float velocity, float distance) {
    absorbWithResponse(velocity, distance, DefaultSpringBackResponse);
}

void SpringBack::absorbWithResponse(float velocity, float distance, float response_seconds) {
    if (response_seconds <= 0.f) {
        response_seconds = DefaultSpringBackResponse;
    }
    _lambda = 2.f * PI / response_seconds;
    _c1 = distance;
    _c2 = velocity * 1e3f + _lambda * distance;
}

std::optional<float> SpringBack::value(float time_milliseconds) const {
    float time = time_milliseconds / 1e3f;
    const float exponent = std::exp(-_lambda * time);
    const float offset = (_c1 + _c2 * time) * exponent;
    const float velocity = velocityAt(time);
    if (std::fabs(offset) < ValueThreshold &&
        std::fabs(velocity) / 1e3f < VelocityThreshold) {
        return std::nullopt;
    }
    return offset;
}

void SpringBack::reset() {
    _lambda = 0.f;
    _c1 = 0.f;
    _c2 = 0.f;
}

float SpringBack::velocityAt(float time) const {
    const float exponent = std::exp(-_lambda * time);
    return (_c2 - _lambda * (_c1 + _c2 * time)) * exponent;
}

};  // namespace kk::gesture
