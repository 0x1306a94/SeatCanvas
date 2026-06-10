//
//  Animator.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/25.
//

#ifndef Animator_hpp
#define Animator_hpp

#include <cstdint>
#include <functional>
#include <vector>

namespace kk::animation {
enum class AnimationCurve {
    Linear,
    EaseOut,
    EaseInOut,
};

struct AnimationOptions {
    double durationMs{200.0};
    double delayMs{0.0};
    AnimationCurve curve{AnimationCurve::EaseInOut};
};

class Animator {
  public:
    using UpdateCallback = std::function<void(float)>;
    using CompletionCallback = std::function<void(bool)>;
    using AnimationId = uint32_t;

    Animator();

    AnimationId play(const AnimationOptions &options, double currentTimeMs, UpdateCallback update, CompletionCallback completion);

    void cancel(AnimationId id);
    void cancelAll();

    bool tick(double currentTimeMs);
    bool hasRunningAnimations() const;

    static float evaluateCurve(AnimationCurve curve, float t);

  private:
    struct Animation {
        AnimationId id;
        AnimationOptions options;
        UpdateCallback update;
        CompletionCallback completion;
        double startTimeMs{0.0};
        bool finished{false};
    };

    AnimationId _nextId{1};
    std::vector<Animation> _animations;
};
};  // namespace kk::animation

#endif /* Animator_hpp */
