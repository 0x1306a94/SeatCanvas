#include "core/gesture/GestureConstant.hpp"
#include "core/gesture/SpringBack.hpp"

#include <cmath>
#include <gtest/gtest.h>

using namespace kk::gesture;

TEST(SpringBack, DefaultConstructor) {
    SpringBack spring;
    // Without absorb, value should be settled
    auto result = spring.value(0.f);
    EXPECT_FALSE(result.has_value());
}

TEST(SpringBack, AbsorbWithDefaultResponse) {
    SpringBack spring;
    spring.absorb(1.f, 100.f);
    // At time 0, offset should equal the initial distance
    auto result = spring.value(0.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(*result, 100.f, 1e-3f);
}

TEST(SpringBack, AbsorbWithCustomResponse) {
    SpringBack spring;
    spring.absorbWithResponse(0.5f, 50.f, 0.3f);
    auto result = spring.value(0.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(*result, 50.f, 1e-3f);
}

TEST(SpringBack, OffsetConvergesToZero) {
    SpringBack spring;
    spring.absorb(1.f, 100.f);
    // At a very large time, the offset should converge to zero
    auto result = spring.value(10000.f);
    EXPECT_FALSE(result.has_value());
}

TEST(SpringBack, OffsetMagnitudeDecreasesOverTime) {
    SpringBack spring;
    spring.absorb(1.f, 100.f);
    auto result1 = spring.value(50.f);
    auto result2 = spring.value(200.f);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_GT(std::fabs(*result1), std::fabs(*result2));
}

TEST(SpringBack, NegativeOffsetConvergesFromBelow) {
    SpringBack spring;
    spring.absorb(-1.f, -100.f);
    auto result = spring.value(0.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_LT(*result, 0.f);
}

TEST(SpringBack, ResetClearsSpring) {
    SpringBack spring;
    spring.absorb(1.f, 100.f);
    spring.reset();
    auto result = spring.value(0.f);
    EXPECT_FALSE(result.has_value());
}

TEST(SpringBack, AbsorbWithZeroOrNegativeResponseUsesDefault) {
    SpringBack spring;
    spring.absorbWithResponse(1.f, 100.f, 0.f);
    auto result = spring.value(0.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(*result, 100.f, 1e-3f);
}

TEST(SpringBack, AbsorbWithNegativeResponseUsesDefault) {
    SpringBack spring;
    spring.absorbWithResponse(1.f, 100.f, -0.1f);
    auto result = spring.value(0.f);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(*result, 100.f, 1e-3f);
}

TEST(SpringBack, ZeroVelocityZeroDistance) {
    SpringBack spring;
    spring.absorb(0.f, 0.f);
    auto result = spring.value(0.f);
    EXPECT_FALSE(result.has_value());
}
