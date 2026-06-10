#include "core/renderer/SeatCanvasCoreRendererState.hpp"

#include <gtest/gtest.h>

using namespace kk::renderer;

TEST(ConvertScreenToContent, NoOffsetIdentityScale) {
    auto screen = tgfx::Point::Make(100.f, 200.f);
    auto offset = tgfx::Point::Make(0.f, 0.f);
    auto result = ConvertScreenToContent(screen, offset, 1.f);
    EXPECT_FLOAT_EQ(result.x, 100.f);
    EXPECT_FLOAT_EQ(result.y, 200.f);
}

TEST(ConvertScreenToContent, WithOffset) {
    auto screen = tgfx::Point::Make(150.f, 250.f);
    auto offset = tgfx::Point::Make(50.f, 50.f);
    auto result = ConvertScreenToContent(screen, offset, 1.f);
    EXPECT_FLOAT_EQ(result.x, 100.f);
    EXPECT_FLOAT_EQ(result.y, 200.f);
}

TEST(ConvertScreenToContent, WithScale) {
    auto screen = tgfx::Point::Make(200.f, 400.f);
    auto offset = tgfx::Point::Make(0.f, 0.f);
    auto result = ConvertScreenToContent(screen, offset, 2.f);
    EXPECT_FLOAT_EQ(result.x, 100.f);
    EXPECT_FLOAT_EQ(result.y, 200.f);
}

TEST(ConvertScreenToContent, WithOffsetAndScale) {
    auto screen = tgfx::Point::Make(250.f, 450.f);
    auto offset = tgfx::Point::Make(50.f, 50.f);
    auto result = ConvertScreenToContent(screen, offset, 2.f);
    EXPECT_FLOAT_EQ(result.x, 100.f);
    EXPECT_FLOAT_EQ(result.y, 200.f);
}

TEST(ConvertContentToScreen, NoOffsetIdentityScale) {
    auto content = tgfx::Point::Make(100.f, 200.f);
    auto offset = tgfx::Point::Make(0.f, 0.f);
    auto result = ConvertContentToScreen(content, offset, 1.f);
    EXPECT_FLOAT_EQ(result.x, 100.f);
    EXPECT_FLOAT_EQ(result.y, 200.f);
}

TEST(ConvertContentToScreen, WithOffset) {
    auto content = tgfx::Point::Make(100.f, 200.f);
    auto offset = tgfx::Point::Make(50.f, 50.f);
    auto result = ConvertContentToScreen(content, offset, 1.f);
    EXPECT_FLOAT_EQ(result.x, 150.f);
    EXPECT_FLOAT_EQ(result.y, 250.f);
}

TEST(ConvertContentToScreen, WithScale) {
    auto content = tgfx::Point::Make(100.f, 200.f);
    auto offset = tgfx::Point::Make(0.f, 0.f);
    auto result = ConvertContentToScreen(content, offset, 2.f);
    EXPECT_FLOAT_EQ(result.x, 200.f);
    EXPECT_FLOAT_EQ(result.y, 400.f);
}

TEST(ConvertContentToScreen, WithOffsetAndScale) {
    auto content = tgfx::Point::Make(100.f, 200.f);
    auto offset = tgfx::Point::Make(50.f, 50.f);
    auto result = ConvertContentToScreen(content, offset, 2.f);
    EXPECT_FLOAT_EQ(result.x, 250.f);
    EXPECT_FLOAT_EQ(result.y, 450.f);
}

TEST(ConvertScreenContent, RoundTrip) {
    auto original = tgfx::Point::Make(123.f, 456.f);
    auto offset = tgfx::Point::Make(10.f, 20.f);
    float scale = 1.5f;

    auto content = ConvertScreenToContent(original, offset, scale);
    auto restored = ConvertContentToScreen(content, offset, scale);
    EXPECT_NEAR(restored.x, original.x, 1e-6f);
    EXPECT_NEAR(restored.y, original.y, 1e-6f);
}

TEST(ConvertScreenContent, ZeroScaleHandledGracefully) {
    // Zero scale would be a degenerate case, but the math should still compute
    auto point = tgfx::Point::Make(100.f, 200.f);
    auto offset = tgfx::Point::Make(0.f, 0.f);
    auto result = ConvertScreenToContent(point, offset, 0.f);
    // Division by zero produces inf, but code should not crash
    EXPECT_TRUE(std::isinf(result.x) || std::isnan(result.x));
}

TEST(MakeViewportEvent, NullStateProducesDefault) {
    auto event = MakeViewportEvent(nullptr);
    EXPECT_FLOAT_EQ(event.zoomScale, 1.0f);
    EXPECT_FLOAT_EQ(event.contentOffset.x, 0.f);
    EXPECT_FLOAT_EQ(event.contentOffset.y, 0.f);
}
