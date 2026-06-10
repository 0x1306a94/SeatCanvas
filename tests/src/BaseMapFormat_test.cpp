#include "core/parser/BaseMapFormat.hpp"
#include "core/parser/BaseMapParserFactory.hpp"

#include <gtest/gtest.h>

using namespace kk::parser;

TEST(ParseFormatName, SvgLowercase) {
    EXPECT_EQ(parseFormatName("svg"), BaseMapFormat::SVG);
}

TEST(ParseFormatName, SvgUppercase) {
    EXPECT_EQ(parseFormatName("SVG"), BaseMapFormat::SVG);
}

TEST(ParseFormatName, SvgMixedCase) {
    EXPECT_EQ(parseFormatName("Svg"), BaseMapFormat::SVG);
}

TEST(ParseFormatName, SvgDotPrefix) {
    EXPECT_EQ(parseFormatName(".svg"), BaseMapFormat::SVG);
}

TEST(ParseFormatName, JsonLowercase) {
    EXPECT_EQ(parseFormatName("json"), BaseMapFormat::JSON);
}

TEST(ParseFormatName, JsonDotPrefix) {
    EXPECT_EQ(parseFormatName(".json"), BaseMapFormat::JSON);
}

TEST(ParseFormatName, GeoJsonLowercase) {
    EXPECT_EQ(parseFormatName("geojson"), BaseMapFormat::GeoJSON);
}

TEST(ParseFormatName, GeoJsonDotPrefix) {
    EXPECT_EQ(parseFormatName(".geojson"), BaseMapFormat::GeoJSON);
}

TEST(ParseFormatName, EmptyString) {
    EXPECT_EQ(parseFormatName(""), BaseMapFormat::Unknown);
}

TEST(ParseFormatName, UnsupportedFormat) {
    EXPECT_EQ(parseFormatName("pdf"), BaseMapFormat::Unknown);
}

TEST(formatNameToString, Svg) {
    auto result = formatNameToString(BaseMapFormat::SVG);
    EXPECT_EQ(result, "svg");
}

TEST(formatNameToString, Json) {
    auto result = formatNameToString(BaseMapFormat::JSON);
    EXPECT_EQ(result, "json");
}

TEST(formatNameToString, GeoJson) {
    auto result = formatNameToString(BaseMapFormat::GeoJSON);
    EXPECT_EQ(result, "geojson");
}

TEST(formatNameToString, Unknown) {
    auto result = formatNameToString(BaseMapFormat::Unknown);
    EXPECT_EQ(result, "unknown");
}

TEST(formatNameToString, RoundTrip) {
    EXPECT_EQ(parseFormatName(formatNameToString(BaseMapFormat::SVG)), BaseMapFormat::SVG);
    EXPECT_EQ(parseFormatName(formatNameToString(BaseMapFormat::JSON)), BaseMapFormat::JSON);
    EXPECT_EQ(parseFormatName(formatNameToString(BaseMapFormat::GeoJSON)), BaseMapFormat::GeoJSON);
}

TEST(BaseMapParserFactory, CreateParserSVG) {
    auto parser = BaseMapParserFactory::createParser(BaseMapFormat::SVG);
    EXPECT_NE(parser, nullptr);
}

TEST(BaseMapParserFactory, CreateParserJSONReturnsNull) {
    auto parser = BaseMapParserFactory::createParser(BaseMapFormat::JSON);
    EXPECT_EQ(parser, nullptr);
}

TEST(BaseMapParserFactory, CreateParserGeoJSONReturnsNull) {
    auto parser = BaseMapParserFactory::createParser(BaseMapFormat::GeoJSON);
    EXPECT_EQ(parser, nullptr);
}

TEST(BaseMapParserFactory, CreateParserUnknownReturnsNull) {
    auto parser = BaseMapParserFactory::createParser(BaseMapFormat::Unknown);
    EXPECT_EQ(parser, nullptr);
}

TEST(BaseMapParserFactory, CreateParserFromStringSVG) {
    auto parser = BaseMapParserFactory::createParser("svg");
    EXPECT_NE(parser, nullptr);
}

TEST(BaseMapParserFactory, CreateParserFromStringUnknown) {
    auto parser = BaseMapParserFactory::createParser("unknown_format");
    EXPECT_EQ(parser, nullptr);
}
