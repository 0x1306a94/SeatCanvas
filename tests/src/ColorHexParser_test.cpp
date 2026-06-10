#include "core/style/ColorHexParser.hpp"

#include <gtest/gtest.h>

using namespace kk::renderer;

TEST(ColorHexParser, Parse8CharHex) {
    tgfx::Color color = tgfx::Color::Black();
    EXPECT_TRUE(ParseColorFromARGBHex("#FF804020", color));
    EXPECT_FLOAT_EQ(color.alpha, 1.0f);
    EXPECT_FLOAT_EQ(color.red, 128.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.green, 64.0f / 255.0f);
    EXPECT_FLOAT_EQ(color.blue, 32.0f / 255.0f);
}

TEST(ColorHexParser, Parse6CharHex) {
    tgfx::Color color = tgfx::Color::Black();
    EXPECT_TRUE(ParseColorFromARGBHex("#123456", color));
    EXPECT_FLOAT_EQ(color.alpha, 1.0f);
    EXPECT_FLOAT_EQ(color.red, 0x12 / 255.0f);
    EXPECT_FLOAT_EQ(color.green, 0x34 / 255.0f);
    EXPECT_FLOAT_EQ(color.blue, 0x56 / 255.0f);
}

TEST(ColorHexParser, RejectEmptyString) {
    tgfx::Color color = {};
    EXPECT_FALSE(ParseColorFromARGBHex("", color));
}

TEST(ColorHexParser, RejectNoHashPrefix) {
    tgfx::Color color = {};
    EXPECT_FALSE(ParseColorFromARGBHex("FF804020", color));
}

TEST(ColorHexParser, RejectInvalidLength) {
    tgfx::Color color = {};
    EXPECT_FALSE(ParseColorFromARGBHex("#123", color));
    EXPECT_FALSE(ParseColorFromARGBHex("#12345", color));
    EXPECT_FALSE(ParseColorFromARGBHex("#1234567", color));
    EXPECT_FALSE(ParseColorFromARGBHex("#123456789", color));
}

TEST(ColorToARGBHex, RoundTrip8Char) {
    auto original = tgfx::Color::FromRGBA(0x12, 0x34, 0x56, 0xFF);
    std::string hex = ColorToARGBHex(original);
    EXPECT_EQ(hex, "#FF123456");

    tgfx::Color parsed = tgfx::Color::Black();
    ASSERT_TRUE(ParseColorFromARGBHex(hex, parsed));
    EXPECT_FLOAT_EQ(original.alpha, parsed.alpha);
    EXPECT_FLOAT_EQ(original.red, parsed.red);
    EXPECT_FLOAT_EQ(original.green, parsed.green);
    EXPECT_FLOAT_EQ(original.blue, parsed.blue);
}

TEST(ColorToARGBHex, SemiTransparent) {
    auto color = tgfx::Color::FromRGBA(0xFF, 0x00, 0x00, 0x80);
    EXPECT_EQ(ColorToARGBHex(color), "#80FF0000");
}
