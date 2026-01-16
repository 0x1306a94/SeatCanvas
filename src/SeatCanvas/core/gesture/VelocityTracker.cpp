//
//  VelocityTracker.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#include "VelocityTracker.hpp"

#include <algorithm>
#include <vector>

namespace kk::gesture {

float CalculateRubberBandOffset(float offset, float range) {
    if (offset < 0.f || range <= 0.f) {
        return 0.f;
    }
    return (1.f - (1.f / (offset / range * RubberBandCoefficient + 1.f))) * range;
}

float CalculateRubberBandOffsetInv(float offset, float range) {
    if (offset < 0.f || range <= 0.f) {
        return 0.f;
    }
    const float safe_offset = std::min(offset, range - 1e-5f);
    return (range * safe_offset / (range - safe_offset)) / RubberBandCoefficient;
}

namespace detail {

namespace {

float Dot(const std::vector<float> &lhs, const std::vector<float> &rhs) {
    float result = 0.f;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        result += lhs[i] * rhs[i];
    }
    return result;
}

}  // namespace

std::optional<std::array<float, 3>> PolyFitLeastSquares(const float *x, const float *y, std::size_t sample_count, std::size_t degree) {
    if (degree < 1 || sample_count == 0) {
        return std::nullopt;
    }

    const std::size_t truncated_degree =
        degree >= sample_count ? sample_count - 1 : degree;
    const std::size_t m = sample_count;
    const std::size_t n = truncated_degree + 1;

    std::vector<std::vector<float>> a(n, std::vector<float>(m, 0.f));
    for (std::size_t h = 0; h < m; ++h) {
        a[0][h] = 1.f;
        for (std::size_t i = 1; i < n; ++i) {
            a[i][h] = a[i - 1][h] * x[h];
        }
    }

    std::vector<std::vector<float>> q = a;
    std::vector<std::vector<float>> r(n, std::vector<float>(n, 0.f));

    for (std::size_t j = 0; j < n; ++j) {
        for (std::size_t i = 0; i < j; ++i) {
            const float dot = Dot(q[j], q[i]);
            for (std::size_t h = 0; h < m; ++h) {
                q[j][h] -= dot * q[i][h];
            }
        }

        const float norm = std::sqrt(Dot(q[j], q[j]));
        if (norm < 1e-6f) {
            return std::nullopt;
        }
        for (float &value : q[j]) {
            value /= norm;
        }
        for (std::size_t i = 0; i < n; ++i) {
            r[j][i] = i < j ? 0.f : Dot(q[j], a[i]);
        }
    }

    std::array<float, 3> coefficients{};
    for (int i = static_cast<int>(n) - 1; i >= 0; --i) {
        float value = 0.f;
        for (std::size_t h = 0; h < m; ++h) {
            value += q[i][h] * y[h];
        }
        for (int j = static_cast<int>(n) - 1; j > i; --j) {
            value -= r[i][j] * coefficients[j];
        }
        value /= r[i][i];
        coefficients[i] = value;
    }

    return coefficients;
}

std::optional<float> CalculateRecurrenceRelationVelocity(const std::vector<float> &times, const std::vector<float> &values) {
    if (times.size() != values.size() || times.size() < 2) {
        return std::nullopt;
    }

    std::vector<float> velocities;
    velocities.reserve(times.size() - 1);
    for (std::size_t i = 1; i < times.size(); ++i) {
        const float delta_time = times[i] - times[i - 1];
        if (delta_time == 0.f) {
            continue;
        }
        velocities.push_back((values[i] - values[i - 1]) / delta_time);
    }

    if (velocities.empty()) {
        return std::nullopt;
    }

    std::optional<float> previous_velocity;
    std::optional<float> current_velocity;

    if (velocities.size() == 1) {
        current_velocity = velocities.front();
    } else {
        for (std::size_t i = 0; i + 1 < velocities.size(); ++i) {
            const float velocity =
                velocities[i] * 0.4f + velocities[i + 1] * 0.6f;
            if (current_velocity.has_value()) {
                previous_velocity = current_velocity;
                current_velocity =
                    current_velocity.value() * 0.8f + velocity * 0.2f;
            } else {
                current_velocity = velocity;
            }
        }
    }

    const float current = current_velocity.value_or(velocities.front());
    if (previous_velocity.has_value()) {
        return previous_velocity.value() * 0.75f + current * 0.25f;
    }
    return current;
}

}  // namespace detail

VelocityTracker::VelocityTracker(VelocityTrackerStrategy strategy)
    : _strategy(strategy)
    , _samples({})
    , _samplesIndex(0) {
}

void VelocityTracker::addDataPoint(float time_milliseconds, float value) {
    _samplesIndex = (_samplesIndex + 1) % VelocityTrackerHistorySize;
    SampleSlot &slot = _samples[_samplesIndex];
    slot.has_value = true;
    slot.data = {time_milliseconds, value};
}

float VelocityTracker::calculate() const {
    const SampleSlot &newest_slot = _samples[_samplesIndex];
    if (!newest_slot.has_value) {
        return 0.f;
    }
    const DataPoint newest = newest_slot.data;
    DataPoint previous = newest;

    std::size_t index = _samplesIndex;
    std::size_t sample_count = 0;

    while (sample_count < VelocityTrackerHistorySize) {
        const SampleSlot &slot = _samples[index];
        if (!slot.has_value) {
            break;
        }
        const DataPoint sample = slot.data;
        const float age = newest.time - sample.time;

        if (_strategy != VelocityTrackerStrategy::Recurrence) {
            const float delta = std::fabs(sample.value - previous.value);
            previous = sample;
            if (age > VelocityTrackerHorizonMs ||
                delta > VelocityTrackerStoppedMs) {
                break;
            }
        }

        _cache.reusable_values[sample_count] = sample.value;
        _cache.reusable_times[sample_count] = -age;
        index = index == 0 ? VelocityTrackerHistorySize - 1 : index - 1;
        ++sample_count;
    }

    if (sample_count < minSampleSize()) {
        return 0.f;
    }

    if (_strategy == VelocityTrackerStrategy::Recurrence) {
        const std::size_t limit = std::min<std::size_t>(sample_count, 4);
        std::vector<float> times(limit);
        std::vector<float> values(limit);
        for (std::size_t i = 0; i < limit; ++i) {
            const std::size_t idx = limit - 1 - i;
            times[i] = _cache.reusable_times[idx];
            values[i] = _cache.reusable_values[idx];
        }
        auto velocity = detail::CalculateRecurrenceRelationVelocity(times, values);
        return velocity.value_or(0.f);
    }

    auto coefficients = detail::PolyFitLeastSquares(_cache.reusable_times.data(), _cache.reusable_values.data(), sample_count, 2);
    if (!coefficients.has_value()) {
        return 0.f;
    }
    return coefficients->at(1);
}

void VelocityTracker::reset() {
    for (auto &slot : _samples) {
        slot.has_value = false;
    }
    _samplesIndex = 0;
}

bool VelocityTracker::ApproachingHalt(float horizontal_velocity, float vertical_velocity) {
    return (horizontal_velocity * horizontal_velocity + vertical_velocity * vertical_velocity) < 0.0625f;
}

std::size_t VelocityTracker::minSampleSize() const {
    return _strategy == VelocityTrackerStrategy::Recurrence ? 2 : 3;
}
};  // namespace kk::gesture
