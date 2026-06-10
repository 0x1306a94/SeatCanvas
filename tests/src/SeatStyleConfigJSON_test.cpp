#include "core/style/SeatStyleConfigJSONHelper.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using namespace kk::renderer;

TEST(CircleSeatStyleConfig, ToJsonBasic) {
    auto fillColor = tgfx::Color::FromRGBA(0x12, 0x34, 0x56, 0xFF);
    auto config = CircleSeatStyleConfig::Make(fillColor);

    nlohmann::json j = config;
    EXPECT_EQ(j["type"], static_cast<int>(SeatStyleType::Circle));
    EXPECT_EQ(j["fill"], "#FF123456");
}

TEST(CircleSeatStyleConfig, ToJsonWithOverlayAndCheckmark) {
    auto fillColor = tgfx::Color::FromRGBA(0xFF, 0x00, 0x00, 0xFF);
    auto overlayColor = tgfx::Color::FromRGBA(0x00, 0xFF, 0x00, 0x80);
    auto checkmarkColor = tgfx::Color::FromRGBA(0x00, 0x00, 0xFF, 0xFF);
    auto config = CircleSeatStyleConfig::Make(fillColor, overlayColor, checkmarkColor);

    nlohmann::json j = config;
    EXPECT_EQ(j["fill"], "#FFFF0000");
    EXPECT_EQ(j["overlay"], "#8000FF00");
    EXPECT_EQ(j["checkmark"], "#FF0000FF");
}

TEST(CircleSeatStyleConfig, NullConfigToJson) {
    std::shared_ptr<CircleSeatStyleConfig> config = nullptr;
    nlohmann::json j = config;
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
}

TEST(CircleSeatStyleConfig, FromJsonBasic) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::Circle)},
        {"fill", "#FF123456"}};

    std::shared_ptr<CircleSeatStyleConfig> config = nullptr;
    j.get_to(config);

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->getFillColor(), tgfx::Color::FromRGBA(0x12, 0x34, 0x56, 0xFF));
    EXPECT_FALSE(config->getOverlayColor().has_value());
    EXPECT_FALSE(config->getCheckmarkColor().has_value());
}

TEST(CircleSeatStyleConfig, FromJsonWithOverlayAndCheckmark) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::Circle)},
        {"fill", "#FFFF0000"},
        {"overlay", "#8000FF00"},
        {"checkmark", "#FF0000FF"}};

    std::shared_ptr<CircleSeatStyleConfig> config = nullptr;
    j.get_to(config);

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->getFillColor(), tgfx::Color::FromRGBA(0xFF, 0x00, 0x00, 0xFF));
    ASSERT_TRUE(config->getOverlayColor().has_value());
    EXPECT_EQ(*config->getOverlayColor(), tgfx::Color::FromRGBA(0x00, 0xFF, 0x00, 0x80));
    ASSERT_TRUE(config->getCheckmarkColor().has_value());
    EXPECT_EQ(*config->getCheckmarkColor(), tgfx::Color::FromRGBA(0x00, 0x00, 0xFF, 0xFF));
}

TEST(CircleSeatStyleConfig, FromJsonMissingFillReturnsNull) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::Circle)}};

    std::shared_ptr<CircleSeatStyleConfig> config = nullptr;
    // Should not crash; parsing failure produces nullptr
    j.get_to(config);
    EXPECT_EQ(config, nullptr);
}

TEST(CircleSeatStyleConfig, FromJsonInvalidFillColorReturnsNull) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::Circle)},
        {"fill", "not_a_color"}};

    std::shared_ptr<CircleSeatStyleConfig> config = nullptr;
    j.get_to(config);
    EXPECT_EQ(config, nullptr);
}

TEST(CircleSeatStyleConfig, JsonRoundTrip) {
    auto fill = tgfx::Color::FromRGBA(0xAB, 0xCD, 0xEF, 0xFF);
    auto overlay = tgfx::Color::FromRGBA(0x11, 0x22, 0x33, 0x44);
    auto checkmark = tgfx::Color::FromRGBA(0x99, 0x88, 0x77, 0xFF);
    auto original = CircleSeatStyleConfig::Make(fill, overlay, checkmark);

    nlohmann::json j = original;
    std::shared_ptr<CircleSeatStyleConfig> restored = nullptr;
    j.get_to(restored);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(*original, *restored);
}

TEST(SVGSeatStyleConfig, ToJson) {
    auto config = SVGSeatStyleConfig::Make("<svg></svg>");
    nlohmann::json j = config;
    EXPECT_EQ(j["type"], static_cast<int>(SeatStyleType::SVG));
    EXPECT_EQ(j["content"], "<svg></svg>");
}

TEST(SVGSeatStyleConfig, NullConfigToJson) {
    std::shared_ptr<SVGSeatStyleConfig> config = nullptr;
    nlohmann::json j = config;
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
}

TEST(SVGSeatStyleConfig, FromJsonBasic) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::SVG)},
        {"content", "<svg></svg>"}};

    std::shared_ptr<SVGSeatStyleConfig> config = nullptr;
    j.get_to(config);

    ASSERT_NE(config, nullptr);
    EXPECT_EQ(config->getContent(), "<svg></svg>");
}

TEST(SVGSeatStyleConfig, FromJsonMissingContentReturnsNull) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::SVG)}};

    std::shared_ptr<SVGSeatStyleConfig> config = nullptr;
    j.get_to(config);
    EXPECT_EQ(config, nullptr);
}

TEST(SVGSeatStyleConfig, FromJsonEmptyContentReturnsNull) {
    nlohmann::json j = {
        {"type", static_cast<int>(SeatStyleType::SVG)},
        {"content", ""}};

    std::shared_ptr<SVGSeatStyleConfig> config = nullptr;
    j.get_to(config);
    EXPECT_EQ(config, nullptr);
}

TEST(SVGSeatStyleConfig, JsonRoundTrip) {
    auto original = SVGSeatStyleConfig::Make("<svg viewBox=\"0 0 10 10\"></svg>");

    nlohmann::json j = original;
    std::shared_ptr<SVGSeatStyleConfig> restored = nullptr;
    j.get_to(restored);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(*original, *restored);
}

TEST(SeatStyleConfig, PolymorphicCircleRoundTrip) {
    auto fill = tgfx::Color::FromRGBA(0xFF, 0x00, 0x00, 0xFF);
    std::shared_ptr<SeatStyleConfig> original = CircleSeatStyleConfig::Make(fill);

    nlohmann::json j = original;
    std::shared_ptr<SeatStyleConfig> restored = nullptr;
    j.get_to(restored);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->getType(), SeatStyleType::Circle);
    EXPECT_EQ(*original, *restored);
}

TEST(SeatStyleConfig, PolymorphicSVGRoundTrip) {
    std::shared_ptr<SeatStyleConfig> original = SVGSeatStyleConfig::Make("<svg/>");

    nlohmann::json j = original;
    std::shared_ptr<SeatStyleConfig> restored = nullptr;
    j.get_to(restored);

    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->getType(), SeatStyleType::SVG);
    EXPECT_EQ(*original, *restored);
}

TEST(SeatStyleConfig, NullConfigToJson) {
    std::shared_ptr<SeatStyleConfig> config = nullptr;
    nlohmann::json j = config;
    EXPECT_TRUE(j.is_object());
    EXPECT_TRUE(j.empty());
}

TEST(SeatStyleConfig, FromJsonMissingTypeReturnsNull) {
    nlohmann::json j = {
        {"fill", "#FF000000"}};

    std::shared_ptr<SeatStyleConfig> config = nullptr;
    j.get_to(config);
    EXPECT_EQ(config, nullptr);
}

TEST(SeatStyleConfig, FromJsonNegativeTypeReturnsNull) {
    nlohmann::json j = {
        {"type", -1}};

    std::shared_ptr<SeatStyleConfig> config = nullptr;
    j.get_to(config);
    EXPECT_EQ(config, nullptr);
}
