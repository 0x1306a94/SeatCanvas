//
//  Animator.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/25.
//

#include "Animator.hpp"

#include <algorithm>
#include <cmath>

namespace kk::animation {
constexpr double kEpsilon = 1e-6;

Animator::Animator() = default;

Animator::AnimationId Animator::play(const AnimationOptions &options, double currentTimeMs, UpdateCallback update, CompletionCallback completion) {
    auto duration = std::max(options.durationMs, 0.0);
    auto delay = std::max(options.delayMs, 0.0);

    if (duration <= kEpsilon) {
        if (update) {
            update(1.0f);
        }
        if (completion) {
            completion(true);
        }
        return 0;
    }

    Animation animation{};
    animation.id = _nextId++;
    animation.options.durationMs = duration;
    animation.options.delayMs = delay;
    animation.options.curve = options.curve;
    animation.update = std::move(update);
    animation.completion = std::move(completion);
    animation.startTimeMs = currentTimeMs;

    _animations.emplace_back(std::move(animation));
    return _animations.back().id;
}

void Animator::cancel(AnimationId id) {
    if (id == 0) {
        return;
    }

    auto iter = std::find_if(_animations.begin(), _animations.end(), [id](const Animation &anim) {
        return anim.id == id;
    });

    if (iter == _animations.end()) {
        return;
    }

    if (iter->completion) {
        iter->completion(false);
    }

    _animations.erase(iter);
}

void Animator::cancelAll() {

    for (auto &item : _animations) {
        if (item.completion) {
            item.completion(false);
        }
    }

    _animations.clear();
}

bool Animator::tick(double currentTimeMs) {
    if (_animations.empty()) {
        return false;
    }

    bool anyActive = false;
    for (auto &animation : _animations) {
        if (animation.finished) {
            continue;
        }

        const auto elapsed = currentTimeMs - animation.startTimeMs;
        if (elapsed < animation.options.delayMs) {
            anyActive = true;
            continue;
        }

        const auto runningTime = elapsed - animation.options.delayMs;
        const auto progress = std::clamp(static_cast<float>(runningTime / animation.options.durationMs), 0.0f, 1.0f);
        const auto eased = evaluateCurve(animation.options.curve, progress);

        if (animation.update) {
            animation.update(eased);
        }

        if (progress >= 1.0f - static_cast<float>(kEpsilon)) {
            animation.finished = true;
        } else {
            anyActive = true;
        }
    }

    for (auto &animation : _animations) {
        if (animation.finished && animation.completion) {
            animation.completion(true);
        }
    }

    _animations.erase(std::remove_if(_animations.begin(), _animations.end(), [](const Animation &anim) {
                          return anim.finished;
                      }),
                      _animations.end());

    return anyActive || !_animations.empty();
}

bool Animator::hasRunningAnimations() const {
    return !_animations.empty();
}

float Animator::evaluateCurve(AnimationCurve curve, float t) {
    switch (curve) {
        case AnimationCurve::EaseOut: {
            const auto inv = 1.0f - t;
            return 1.0f - inv * inv;
        }
        case AnimationCurve::EaseInOut: {
            return (t < 0.5f) ? (2.0f * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f);
        }
        case AnimationCurve::Linear:
        default:
            return t;
    }
}
};  // namespace kk::animation
