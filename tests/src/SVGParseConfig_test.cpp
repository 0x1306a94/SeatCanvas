#include "core/svg/SVGParseConfig.hpp"

#include <cstring>
#include <gtest/gtest.h>

using namespace kk::svg;

TEST(SVGParseConfig, DefaultConstructor) {
    SVGParseConfig config;
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONNullData) {
    auto config = SVGParseConfig::FromJSON(nullptr, 0);
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONNullSharedData) {
    auto config = SVGParseConfig::FromJSON(nullptr);
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONZoneIdAttributeNamesArray) {
    const char *json = R"({"zoneIdAttributeNames": ["data-zone-id", "zone-id"]})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 2u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "data-zone-id");
    EXPECT_EQ(config.zoneIdAttributeNames[1], "zone-id");
}

TEST(SVGParseConfig, FromJSONSingleZoneIdAttribute) {
    const char *json = R"({"zoneIdAttributeName": "my-custom-zone"})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "my-custom-zone");
}

TEST(SVGParseConfig, FromJSONZoneIdAttributeNameTakesPrecedence) {
    // zoneIdAttributeName should replace the array if both are present
    const char *json = R"({"zoneIdAttributeName": "single-attr", "zoneIdAttributeNames": ["attr1", "attr2"]})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "single-attr");
}

TEST(SVGParseConfig, FromJSONEmptyObject) {
    const char *json = "{}";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONInvalidJSON) {
    const char *json = "{invalid json";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONNonObject) {
    const char *json = "[1, 2, 3]";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONNonArrayAttributeNames) {
    const char *json = R"({"zoneIdAttributeNames": "not-an-array"})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONNonStringArrayElements) {
    const char *json = R"({"zoneIdAttributeNames": [123, 456]})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONEmptyStringInArray) {
    const char *json = R"({"zoneIdAttributeNames": [""]})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONEmptyArray) {
    const char *json = R"({"zoneIdAttributeNames": []})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}

TEST(SVGParseConfig, FromJSONEmptyZoneIdAttributeName) {
    const char *json = R"({"zoneIdAttributeName": ""})";
    auto config = SVGParseConfig::FromJSON(json, std::strlen(json));
    // Falls back to default
    ASSERT_EQ(config.zoneIdAttributeNames.size(), 1u);
    EXPECT_EQ(config.zoneIdAttributeNames[0], "zoneId");
}
