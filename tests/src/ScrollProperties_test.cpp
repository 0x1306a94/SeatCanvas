#include "core/gesture/BounceEdge.hpp"
#include "core/gesture/ScrollProperties.hpp"
#include "core/gesture/Scroller.hpp"
#include "core/gesture/SpringBack.hpp"

#include <gtest/gtest.h>

using namespace kk::gesture;

TEST(ScrollProperties, DefaultState) {
    ScrollProperties props;
    EXPECT_FALSE(props.is_decelerating);
    EXPECT_FALSE(props.is_bouncing);
    EXPECT_EQ(props.bounce_edge, BounceEdge::NONE);
    EXPECT_EQ(props.animation_begin_time, 0.0);
    EXPECT_FLOAT_EQ(props.animation_begin_offset, 0.0f);
    EXPECT_FLOAT_EQ(props.animation_begin_velocity, 0.0f);
    EXPECT_EQ(props.scroller, nullptr);
    EXPECT_EQ(props.springBack, nullptr);
}

TEST(ScrollProperties, ClearResetsState) {
    ScrollProperties props;
    props.is_decelerating = true;
    props.is_bouncing = true;
    props.bounce_edge = BounceEdge::MIN;
    props.animation_begin_time = 42.0;
    props.animation_begin_offset = 100.0f;
    props.animation_begin_velocity = 5.0f;

    props.clear();

    EXPECT_FALSE(props.is_decelerating);
    EXPECT_FALSE(props.is_bouncing);
    EXPECT_EQ(props.bounce_edge, BounceEdge::NONE);
    EXPECT_EQ(props.animation_begin_time, 0.0);
    EXPECT_FLOAT_EQ(props.animation_begin_offset, 0.0f);
    EXPECT_FLOAT_EQ(props.animation_begin_velocity, 0.0f);
}

TEST(ScrollProperties, ResetSetsState) {
    ScrollProperties props;
    props.reset(10.0f, 50.0f);

    EXPECT_FLOAT_EQ(props.animation_begin_offset, 50.0f);
    EXPECT_FLOAT_EQ(props.animation_begin_velocity, 10.0f);
    EXPECT_GT(props.animation_begin_time, 0.0);
}

TEST(ScrollProperties, PrepareScrollerCreatesScroller) {
    ScrollProperties props;
    EXPECT_EQ(props.scroller, nullptr);

    props.prepareScroller(0.998f);
    EXPECT_NE(props.scroller, nullptr);
}

TEST(ScrollProperties, PrepareScrollerReusesExisting) {
    ScrollProperties props;
    props.prepareScroller(0.998f);
    auto *first = props.scroller.get();

    props.prepareScroller(0.95f);
    // Same scroller instance reused, rate updated
    EXPECT_EQ(props.scroller.get(), first);
}

TEST(ScrollProperties, PrepareSpringBackCreatesInstance) {
    ScrollProperties props;
    EXPECT_EQ(props.springBack, nullptr);

    props.prepareSpringBack();
    EXPECT_NE(props.springBack, nullptr);
}

TEST(ScrollProperties, PrepareSpringBackIdempotent) {
    ScrollProperties props;
    props.prepareSpringBack();
    auto *first = props.springBack.get();

    props.prepareSpringBack();
    EXPECT_EQ(props.springBack.get(), first);
}
