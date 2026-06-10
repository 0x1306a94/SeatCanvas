#include "core/utils/ColorIntConverter.hpp"

#include <gtest/gtest.h>

using namespace kk::utils;

TEST(ColorFromARGBInt, OpaqueBlack) {
    uint32_t argb = 0xFF000000;
    auto color = ColorFromARGBInt(argb);
    EXPECT_FLOAT_EQ(color.alpha, 1.f);
    EXPECT_FLOAT_EQ(color.red, 0.f);
    EXPECT_FLOAT_EQ(color.green, 0.f);
    EXPECT_FLOAT_EQ(color.blue, 0.f);
}

TEST(ColorFromARGBInt, OpaqueWhite) {
    uint32_t argb = 0xFFFFFFFF;
    auto color = ColorFromARGBInt(argb);
    EXPECT_FLOAT_EQ(color.alpha, 1.f);
    EXPECT_FLOAT_EQ(color.red, 1.f);
    EXPECT_FLOAT_EQ(color.green, 1.f);
    EXPECT_FLOAT_EQ(color.blue, 1.f);
}

TEST(ColorFromARGBInt, SemiTransparent) {
    uint32_t argb = 0x80FF0000;
    auto color = ColorFromARGBInt(argb);
    EXPECT_NEAR(color.alpha, 128.f / 255.f, 1e-6f);
    EXPECT_FLOAT_EQ(color.red, 1.f);
    EXPECT_FLOAT_EQ(color.green, 0.f);
    EXPECT_FLOAT_EQ(color.blue, 0.f);
}

TEST(ColorFromARGBInt, FullyTransparent) {
    uint32_t argb = 0x00000000;
    auto color = ColorFromARGBInt(argb);
    EXPECT_FLOAT_EQ(color.alpha, 0.f);
}

TEST(ColorToARGBInt, OpaqueBlack) {
    auto color = tgfx::Color::FromRGBA(0, 0, 0, 255);
    int32_t argb = ColorToARGBInt(color);
    EXPECT_EQ(static_cast<uint32_t>(argb), 0xFF000000u);
}

TEST(ColorToARGBInt, OpaqueWhite) {
    auto color = tgfx::Color::FromRGBA(255, 255, 255, 255);
    int32_t argb = ColorToARGBInt(color);
    EXPECT_EQ(static_cast<uint32_t>(argb), 0xFFFFFFFFu);
}

TEST(ColorToARGBInt, SemiTransparentRed) {
    auto color = tgfx::Color::FromRGBA(255, 0, 0, 128);
    int32_t argb = ColorToARGBInt(color);
    EXPECT_EQ(static_cast<uint32_t>(argb), 0x80FF0000u);
}

TEST(ColorFromARGBInt, RoundTrip) {
    uint32_t original = 0xAA12CDEF;
    auto color = ColorFromARGBInt(original);
    int32_t converted = ColorToARGBInt(color);
    EXPECT_EQ(static_cast<uint32_t>(converted), original);
}

TEST(ColorToARGBInt, RoundTrip) {
    auto original = tgfx::Color::FromRGBA(0x12, 0x34, 0x56, 0x78);
    int32_t argb = ColorToARGBInt(original);
    auto color = ColorFromARGBInt(static_cast<uint32_t>(argb));
    EXPECT_NEAR(color.alpha, original.alpha, 1e-6f);
    EXPECT_NEAR(color.red, original.red, 1e-6f);
    EXPECT_NEAR(color.green, original.green, 1e-6f);
    EXPECT_NEAR(color.blue, original.blue, 1e-6f);
}

TEST(ColorToARGBInt, AllChannelsOrdered) {
    auto color = tgfx::Color::FromRGBA(0x10, 0x20, 0x30, 0x40);
    int32_t argb = ColorToARGBInt(color);
    uint32_t val = static_cast<uint32_t>(argb);
    // Alpha is most significant byte
    EXPECT_EQ((val >> 24) & 0xFF, 0x40u);
    EXPECT_EQ((val >> 16) & 0xFF, 0x10u);
    EXPECT_EQ((val >> 8) & 0xFF, 0x20u);
    EXPECT_EQ(val & 0xFF, 0x30u);
}
