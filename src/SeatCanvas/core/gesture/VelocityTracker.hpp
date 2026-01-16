//
//  VelocityTracker.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef VelocityTracker_hpp
#define VelocityTracker_hpp

#include "GestureConstant.hpp"
#include "VelocityTrackerStrategy.hpp"

#include <array>
#include <optional>
#include <vector>

namespace kk::gesture {

float CalculateRubberBandOffset(float offset, float range);

float CalculateRubberBandOffsetInv(float offset, float range);

namespace detail {

std::optional<std::array<float, 3>> PolyFitLeastSquares(const float *x, const float *y, std::size_t sample_count, std::size_t degree);

std::optional<float> CalculateRecurrenceRelationVelocity(const std::vector<float> &times, const std::vector<float> &values);

}  // namespace detail

class VelocityTracker {
  public:
    explicit VelocityTracker(VelocityTrackerStrategy strategy = VelocityTrackerStrategy::Recurrence);

    void addDataPoint(float time_milliseconds, float value);

    float calculate() const;

    void reset();

    static bool ApproachingHalt(float horizontal_velocity, float vertical_velocity);

  private:
    struct DataPoint {
        float time = 0.f;
        float value = 0.f;
    };

    struct SampleSlot {
        bool has_value = false;
        DataPoint data{};
    };

    struct Cache {
        std::array<float, VelocityTrackerHistorySize> reusable_values{};
        std::array<float, VelocityTrackerHistorySize> reusable_times{};
    };

    std::size_t minSampleSize() const;

    VelocityTrackerStrategy _strategy;
    mutable Cache _cache;
    std::array<SampleSlot, VelocityTrackerHistorySize> _samples{};
    std::size_t _samplesIndex{0};
};

};  // namespace kk::gesture

#endif /* VelocityTracker_hpp */
