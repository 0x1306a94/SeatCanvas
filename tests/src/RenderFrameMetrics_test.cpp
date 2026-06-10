#include "core/renderer/RenderFrameMetrics.hpp"

#include <gtest/gtest.h>

using namespace kk::renderer;

TEST(RenderFrameMetrics, InitialFPSIsZero) {
    RenderFrameMetrics metrics;
    EXPECT_FLOAT_EQ(metrics.currentFPS(), 0.0f);
}

TEST(RenderFrameMetrics, InitialLastDrawTimeIsZero) {
    RenderFrameMetrics metrics;
    EXPECT_EQ(metrics.lastDrawTime(), 0);
}

TEST(RenderFrameMetrics, InitialAverageDrawTimeIsZero) {
    RenderFrameMetrics metrics;
    EXPECT_EQ(metrics.averageDrawTime(), 0);
}

TEST(RenderFrameMetrics, IsFirstFrameInitiallyTrue) {
    RenderFrameMetrics metrics;
    EXPECT_TRUE(metrics.isFirstFrame());
}

TEST(RenderFrameMetrics, IsNotFirstFrameAfterRecord) {
    RenderFrameMetrics metrics;
    metrics.recordFrame(16);
    EXPECT_FALSE(metrics.isFirstFrame());
}

TEST(RenderFrameMetrics, LastDrawTimeReturnsLastRecordedValue) {
    RenderFrameMetrics metrics;
    metrics.recordFrame(10);
    EXPECT_EQ(metrics.lastDrawTime(), 10);
    metrics.recordFrame(20);
    EXPECT_EQ(metrics.lastDrawTime(), 20);
    metrics.recordFrame(30);
    EXPECT_EQ(metrics.lastDrawTime(), 30);
}

TEST(RenderFrameMetrics, AverageDrawTimeCalculation) {
    RenderFrameMetrics metrics;
    // Record frames with simulated timestamps will work because Platform is available
    metrics.recordFrame(10);
    int64_t avg1 = metrics.averageDrawTime();
    EXPECT_EQ(avg1, 10);

    metrics.recordFrame(20);
    int64_t avg2 = metrics.averageDrawTime();
    EXPECT_EQ(avg2, 15);
}

TEST(RenderFrameMetrics, ResetFramesClearsData) {
    RenderFrameMetrics metrics;
    metrics.recordFrame(16);
    EXPECT_FALSE(metrics.isFirstFrame());

    metrics.resetFrames();
    EXPECT_TRUE(metrics.isFirstFrame());
    EXPECT_FLOAT_EQ(metrics.currentFPS(), 0.0f);
    EXPECT_EQ(metrics.lastDrawTime(), 0);
    EXPECT_EQ(metrics.averageDrawTime(), 0);
}

TEST(RenderFrameMetrics, ResetFramesIdempotent) {
    RenderFrameMetrics metrics;
    metrics.resetFrames();
    metrics.resetFrames();
    EXPECT_TRUE(metrics.isFirstFrame());
    EXPECT_FLOAT_EQ(metrics.currentFPS(), 0.0f);
}

TEST(RenderFrameMetrics, RecordFrameDoesNotCrash) {
    RenderFrameMetrics metrics;
    for (int i = 0; i < 100; ++i) {
        metrics.recordFrame(16);
    }
    // Old frames should have been pruned
    EXPECT_FALSE(metrics.isFirstFrame());
    SUCCEED();
}
