#include "core/gesture/GestureConstant.hpp"
#include "core/gesture/Scroller.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace kk::gesture;

TEST(Scroller, DefaultDecelerationRate) {
    Scroller scroller;
    scroller.fling(100.f);
    auto result = scroller.value(1.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->velocity, 100.f * std::pow(DecelerationRateNormal, 1.f), 1e-6f);
}

TEST(Scroller, CustomDecelerationRate) {
    Scroller scroller(DecelerationRateFast);
    scroller.fling(100.f);
    auto result = scroller.value(1.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->velocity, 100.f * std::pow(DecelerationRateFast, 1.f), 1e-6f);
}

TEST(Scroller, SetDecelerationRateAfterConstruction) {
    Scroller scroller;
    scroller.setDecelerationRate(DecelerationRateFast);
    scroller.fling(100.f);
    auto result = scroller.value(1.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->velocity, 100.f * std::pow(DecelerationRateFast, 1.f), 1e-6f);
}

TEST(Scroller, ZeroVelocityReturnsNullopt) {
    Scroller scroller;
    scroller.fling(0.f);
    auto result = scroller.value(10.f);
    EXPECT_FALSE(result.has_value());
}

TEST(Scroller, VelocityBelowThresholdReturnsNullopt) {
    Scroller scroller;
    scroller.fling(VelocityThreshold * 0.9f);
    auto result = scroller.value(0.f);
    EXPECT_FALSE(result.has_value());
}

TEST(Scroller, PositiveFlingProducesPositiveOffset) {
    Scroller scroller;
    scroller.fling(1000.f);
    auto result = scroller.value(10.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result->velocity, 0.f);
    EXPECT_GT(result->offset, 0.f);
}

TEST(Scroller, NegativeFlingProducesNegativeOffset) {
    Scroller scroller;
    scroller.fling(-500.f);
    auto result = scroller.value(5.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_LT(result->velocity, 0.f);
    EXPECT_LT(result->offset, 0.f);
}

TEST(Scroller, ResetAfterFlingReturnsNullopt) {
    Scroller scroller;
    scroller.fling(100.f);
    scroller.reset();
    auto result = scroller.value(10.f);
    EXPECT_FALSE(result.has_value());
}

TEST(Scroller, OffsetMagnitudeIncreasesOverTime) {
    Scroller scroller(DecelerationRateFast);
    scroller.fling(100.f);
    auto result1 = scroller.value(5.f);
    auto result2 = scroller.value(10.f);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_GT(std::fabs(result2->offset), std::fabs(result1->offset));
}

TEST(Scroller, VelocityDecaysOverTime) {
    Scroller scroller(DecelerationRateFast);
    scroller.fling(1000.f);
    auto result1 = scroller.value(5.f);
    auto result2 = scroller.value(10.f);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_GT(std::fabs(result1->velocity), std::fabs(result2->velocity));
}

TEST(Scroller, ValueAtZeroTime) {
    Scroller scroller;
    scroller.fling(100.f);
    auto result = scroller.value(0.f);
    ASSERT_TRUE(result.has_value());
    // At time 0, offset should be 0, velocity at full
    EXPECT_FLOAT_EQ(result->offset, 0.f);
    EXPECT_FLOAT_EQ(result->velocity, 100.f);
}

TEST(Scroller, EventualDecayToNullopt) {
    Scroller scroller(DecelerationRateFast);
    scroller.fling(100.f);
    // After a very long time, velocity should fall below threshold
    auto result = scroller.value(10000.f);
    EXPECT_FALSE(result.has_value());
}
