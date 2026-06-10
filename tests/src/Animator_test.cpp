#include "core/animation/Animator.hpp"

#include <gtest/gtest.h>

using namespace kk::animation;

TEST(AnimatorEvaluateCurve, Linear) {
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::Linear, 0.f), 0.f);
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::Linear, 0.5f), 0.5f);
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::Linear, 1.f), 1.f);
}

TEST(AnimatorEvaluateCurve, EaseOut) {
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::EaseOut, 0.f), 0.f);
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::EaseOut, 1.f), 1.f);
    // EaseOut curve: 1 - (1-t)^2, should start fast and slow down
    float mid = Animator::evaluateCurve(AnimationCurve::EaseOut, 0.5f);
    EXPECT_FLOAT_EQ(mid, 0.75f);
    EXPECT_GT(mid, 0.5f);
}

TEST(AnimatorEvaluateCurve, EaseInOut) {
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::EaseInOut, 0.f), 0.f);
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::EaseInOut, 1.f), 1.f);
    // EaseInOut: at t=0.5, value should be 0.5
    EXPECT_FLOAT_EQ(Animator::evaluateCurve(AnimationCurve::EaseInOut, 0.5f), 0.5f);
}

TEST(AnimatorEvaluateCurve, Monotonic) {
    float prev = 0.f;
    for (int i = 1; i <= 100; i++) {
        float t = static_cast<float>(i) / 100.f;
        float v = Animator::evaluateCurve(AnimationCurve::EaseInOut, t);
        EXPECT_GE(v, prev);
        prev = v;
    }
}

TEST(Animator, ZeroDurationCompletesImmediately) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 0.0;

    int updateCalled = 0;
    int completionCalled = 0;
    animator.play(
        options, 0.0,
        [&](float progress) {
            updateCalled++;
            EXPECT_FLOAT_EQ(progress, 1.f);
        },
        [&](bool finished) {
            completionCalled++;
            EXPECT_TRUE(finished);
        });

    EXPECT_EQ(updateCalled, 1);
    EXPECT_EQ(completionCalled, 1);
    EXPECT_FALSE(animator.hasRunningAnimations());
}

TEST(Animator, ZeroDurationWithDelayStillFires) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 0.0;
    options.delayMs = 100.0;

    int updateCalled = 0;
    int completionCalled = 0;
    animator.play(
        options, 0.0,
        [&](float) {
            updateCalled++;
        },
        [&](bool finished) {
            completionCalled++;
            EXPECT_TRUE(finished);
        });

    EXPECT_EQ(updateCalled, 1);
    EXPECT_EQ(completionCalled, 1);
}

TEST(Animator, NormalDurationProgressesOverTime) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 200.0;

    float lastProgress = -1.f;
    animator.play(
        options, 0.0,
        [&](float progress) {
            lastProgress = progress;
        },
        nullptr);

    // Tick at start of animation
    animator.tick(0.0);
    EXPECT_GE(lastProgress, 0.f);

    // Tick at halfway
    animator.tick(100.0);
    EXPECT_NEAR(lastProgress, 0.5f, 0.05f);

    // Tick at end
    animator.tick(200.0);
    EXPECT_FLOAT_EQ(lastProgress, 1.f);
}

TEST(Animator, DelayPausesStart) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;
    options.delayMs = 50.0;

    float lastProgress = 0.f;
    animator.play(
        options, 0.0,
        [&](float progress) {
            lastProgress = progress;
        },
        nullptr);

    // Still in delay period
    animator.tick(25.0);
    EXPECT_FLOAT_EQ(lastProgress, 0.f);

    // Just past delay
    animator.tick(50.0);
    EXPECT_FLOAT_EQ(lastProgress, 0.f);

    // Well into animation
    animator.tick(100.0);
    EXPECT_NEAR(lastProgress, 0.5f, 0.05f);
}

TEST(Animator, CompletionCallbackFiresAtEnd) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;

    bool completed = false;
    animator.play(options, 0.0, nullptr,
                  [&](bool finished) {
                      completed = true;
                      EXPECT_TRUE(finished);
                  });

    animator.tick(100.0);
    EXPECT_TRUE(completed);
}

TEST(Animator, CancelFiresCompletionWithFalse) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;

    bool finished = true;
    auto id = animator.play(options, 0.0, nullptr,
                            [&](bool f) {
                                finished = f;
                            });

    animator.cancel(id);
    EXPECT_FALSE(finished);
}

TEST(Animator, CancelNonexistentIdIsSafe) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;
    animator.play(options, 0.0, nullptr, nullptr);

    animator.cancel(9999);
    EXPECT_TRUE(animator.hasRunningAnimations());
}

TEST(Animator, CancelZeroIsNoop) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;
    animator.play(options, 0.0, nullptr, nullptr);

    animator.cancel(0);
    EXPECT_TRUE(animator.hasRunningAnimations());
}

TEST(Animator, CancelAllFiresAllCompletionsWithFalse) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;

    int completionCount = 0;
    int falseCount = 0;
    for (int i = 0; i < 3; i++) {
        animator.play(options, 0.0, nullptr,
                      [&](bool finished) {
                          completionCount++;
                          if (!finished)
                              falseCount++;
                      });
    }

    animator.cancelAll();
    EXPECT_EQ(completionCount, 3);
    EXPECT_EQ(falseCount, 3);
    EXPECT_FALSE(animator.hasRunningAnimations());
}

TEST(Animator, HasRunningAnimationsFalseInitially) {
    Animator animator;
    EXPECT_FALSE(animator.hasRunningAnimations());
}

TEST(Animator, HasRunningAnimationsAfterPlay) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;
    animator.play(options, 0.0, nullptr, nullptr);
    EXPECT_TRUE(animator.hasRunningAnimations());
}

TEST(Animator, NullCallbacksAreSafe) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;
    // Should not crash
    auto id = animator.play(options, 0.0, nullptr, nullptr);
    animator.tick(50.0);
    animator.tick(100.0);
    EXPECT_FALSE(animator.hasRunningAnimations());
}

TEST(Animator, SequentialNormalAnimationsGetDifferentIds) {
    Animator animator;
    AnimationOptions options{};
    options.durationMs = 100.0;

    auto id1 = animator.play(options, 0.0, nullptr, nullptr);
    auto id2 = animator.play(options, 0.0, nullptr, nullptr);
    // Both ids should be non-zero and different
    EXPECT_NE(id1, 0u);
    EXPECT_NE(id2, 0u);
    EXPECT_NE(id1, id2);
}
