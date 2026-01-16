//
//  GestureConstant.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef GestureConstant_hpp
#define GestureConstant_hpp

#include <cstddef>

namespace kk::gesture {
constexpr float DefaultSpringBackResponse = 0.575f;
constexpr float DecelerationRateNormal = 0.998f;
constexpr float DecelerationRateFast = 0.99f;

constexpr float PI = 3.14159265358979323846f;
constexpr float VelocityThreshold = 1e-2f;
constexpr float ValueThreshold = 0.1f;
constexpr float RubberBandCoefficient = 0.55f;
constexpr std::size_t VelocityTrackerHistorySize = 20;
constexpr float VelocityTrackerHorizonMs = 100.f;
constexpr float VelocityTrackerStoppedMs = 40.f;
constexpr std::size_t HistorySize = VelocityTrackerHistorySize;
};  // namespace kk::gesture

#endif /* GestureConstant_hpp */
