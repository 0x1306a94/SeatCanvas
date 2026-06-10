#include "core/gesture/VelocityTracker.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace kk::gesture;

TEST(RubberBandOffset, ZeroOffset) {
    EXPECT_FLOAT_EQ(CalculateRubberBandOffset(0.f, 100.f), 0.f);
}

TEST(RubberBandOffset, NegativeOffset) {
    EXPECT_FLOAT_EQ(CalculateRubberBandOffset(-10.f, 100.f), 0.f);
}

TEST(RubberBandOffset, ZeroRange) {
    EXPECT_FLOAT_EQ(CalculateRubberBandOffset(50.f, 0.f), 0.f);
}

TEST(RubberBandOffset, IncreasingWithinRange) {
    float a = CalculateRubberBandOffset(10.f, 100.f);
    float b = CalculateRubberBandOffset(50.f, 100.f);
    float c = CalculateRubberBandOffset(90.f, 100.f);
    EXPECT_LT(a, b);
    EXPECT_LT(b, c);
}

TEST(RubberBandOffsetInv, Basic) {
    float original = 50.f;
    float range = 200.f;
    float rubber = CalculateRubberBandOffset(original, range);
    float recovered = CalculateRubberBandOffsetInv(rubber, range);
    EXPECT_NEAR(recovered, original, 1e-3f);
}

TEST(RubberBandOffsetInv, ZeroRange) {
    EXPECT_FLOAT_EQ(CalculateRubberBandOffsetInv(50.f, 0.f), 0.f);
}

TEST(RubberBandOffsetInv, NegativeOffset) {
    EXPECT_FLOAT_EQ(CalculateRubberBandOffsetInv(-10.f, 100.f), 0.f);
}

TEST(PolyFitLeastSquares, DegreeTooLow) {
    float x[] = {1.f, 2.f, 3.f};
    float y[] = {2.f, 4.f, 6.f};
    auto result = detail::PolyFitLeastSquares(x, y, 3, 0);
    EXPECT_FALSE(result.has_value());
}

TEST(PolyFitLeastSquares, ZeroSamples) {
    auto result = detail::PolyFitLeastSquares(nullptr, nullptr, 0, 1);
    EXPECT_FALSE(result.has_value());
}

TEST(PolyFitLeastSquares, LinearFit) {
    float x[] = {0.f, 1.f, 2.f, 3.f};
    float y[] = {0.f, 2.f, 4.f, 6.f};
    auto result = detail::PolyFitLeastSquares(x, y, 4, 1);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR((*result)[0], 0.f, 1e-3f);
    EXPECT_NEAR((*result)[1], 2.f, 1e-3f);
}

TEST(PolyFitLeastSquares, QuadraticFit) {
    float x[] = {0.f, 1.f, 2.f, 3.f};
    float y[] = {0.f, 1.f, 4.f, 9.f};
    auto result = detail::PolyFitLeastSquares(x, y, 4, 2);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR((*result)[0], 0.f, 1e-3f);
    EXPECT_NEAR((*result)[1], 0.f, 1e-3f);
    EXPECT_NEAR((*result)[2], 1.f, 1e-3f);
}

TEST(PolyFitLeastSquares, ConstantInput) {
    float x[] = {0.f, 1.f, 2.f};
    float y[] = {5.f, 5.f, 5.f};
    auto result = detail::PolyFitLeastSquares(x, y, 3, 1);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR((*result)[0], 5.f, 1e-3f);
    EXPECT_NEAR((*result)[1], 0.f, 1e-3f);
}

TEST(CalculateRecurrenceRelationVelocity, InsufficientSamples) {
    std::vector<float> times = {0.f};
    std::vector<float> values = {0.f};
    auto result = detail::CalculateRecurrenceRelationVelocity(times, values);
    EXPECT_FALSE(result.has_value());
}

TEST(CalculateRecurrenceRelationVelocity, MismatchedSizes) {
    std::vector<float> times = {0.f, 1.f};
    std::vector<float> values = {0.f};
    auto result = detail::CalculateRecurrenceRelationVelocity(times, values);
    EXPECT_FALSE(result.has_value());
}

TEST(CalculateRecurrenceRelationVelocity, ConstantVelocity) {
    std::vector<float> times = {0.f, 1.f, 2.f, 3.f};
    std::vector<float> values = {0.f, 10.f, 20.f, 30.f};
    auto result = detail::CalculateRecurrenceRelationVelocity(times, values);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(*result, 10.f, 1e-3f);
}

TEST(CalculateRecurrenceRelationVelocity, TwoPoints) {
    std::vector<float> times = {0.f, 2.f};
    std::vector<float> values = {0.f, 100.f};
    auto result = detail::CalculateRecurrenceRelationVelocity(times, values);
    ASSERT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(*result, 50.f);
}

TEST(VelocityTracker, InitialStateReturnsZero) {
    VelocityTracker tracker(VelocityTrackerStrategy::Recurrence);
    EXPECT_FLOAT_EQ(tracker.calculate(), 0.f);
}

TEST(VelocityTracker, AddSinglePointReturnsZero) {
    VelocityTracker tracker(VelocityTrackerStrategy::Recurrence);
    tracker.addDataPoint(0.f, 100.f);
    EXPECT_FLOAT_EQ(tracker.calculate(), 0.f);
}

TEST(VelocityTracker, RecurrenceBasicCalculation) {
    VelocityTracker tracker(VelocityTrackerStrategy::Recurrence);
    tracker.addDataPoint(0.f, 0.f);
    tracker.addDataPoint(16.f, 100.f);
    tracker.addDataPoint(32.f, 200.f);
    auto velocity = tracker.calculate();
    EXPECT_GT(std::fabs(velocity), 0.f);
}

TEST(VelocityTracker, ResetClearsData) {
    VelocityTracker tracker(VelocityTrackerStrategy::Recurrence);
    tracker.addDataPoint(0.f, 0.f);
    tracker.addDataPoint(16.f, 100.f);
    tracker.reset();
    EXPECT_FLOAT_EQ(tracker.calculate(), 0.f);
}

TEST(VelocityTracker, LSQStrategy) {
    VelocityTracker tracker(VelocityTrackerStrategy::Lsq2);
    tracker.addDataPoint(0.f, 0.f);
    tracker.addDataPoint(16.f, 1.f);
    tracker.addDataPoint(32.f, 2.f);
    auto velocity = tracker.calculate();
    EXPECT_GT(std::fabs(velocity), 0.f);
}

TEST(VelocityTracker, ApproachingHaltDetection) {
    EXPECT_TRUE(VelocityTracker::ApproachingHalt(0.f, 0.f));
    EXPECT_TRUE(VelocityTracker::ApproachingHalt(0.1f, 0.1f));
    EXPECT_FALSE(VelocityTracker::ApproachingHalt(1.f, 0.f));
    EXPECT_FALSE(VelocityTracker::ApproachingHalt(0.f, 1.f));
}
