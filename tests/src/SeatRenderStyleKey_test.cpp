#include "core/style/SeatRenderStyleKey.hpp"

#include <gtest/gtest.h>
#include <unordered_map>
#include <unordered_set>

using namespace kk::renderer;

TEST(SeatRenderStyleKey, Equality) {
    SeatRenderStyleKey a{0, 1, false};
    SeatRenderStyleKey b{0, 1, false};
    SeatRenderStyleKey c{0, 1, true};
    SeatRenderStyleKey d{1, 1, false};
    EXPECT_EQ(a, b);
    EXPECT_FALSE(a == c);
    EXPECT_FALSE(a == d);
}

TEST(SeatRenderStyleKeyHash, DifferentKeysProduceDifferentHash) {
    SeatRenderStyleKeyHash hasher;
    SeatRenderStyleKey a{0, 0, false};
    SeatRenderStyleKey b{0, 0, true};
    SeatRenderStyleKey c{0, 1, false};
    SeatRenderStyleKey d{1, 0, false};
    EXPECT_NE(hasher(a), hasher(b));
    EXPECT_NE(hasher(a), hasher(c));
    EXPECT_NE(hasher(a), hasher(d));
}

TEST(SeatRenderStyleKeyHash, SameKeysProduceSameHash) {
    SeatRenderStyleKeyHash hasher;
    SeatRenderStyleKey a{42, 3, true};
    SeatRenderStyleKey b{42, 3, true};
    EXPECT_EQ(hasher(a), hasher(b));
}

TEST(SeatRenderStyleKeyHash, UsableInUnorderedSet) {
    std::unordered_set<SeatRenderStyleKey, SeatRenderStyleKeyHash> set;
    set.insert({0, 0, false});
    set.insert({0, 1, false});
    set.insert({1, 0, false});
    EXPECT_EQ(set.size(), 3u);
    EXPECT_TRUE(set.find({0, 0, false}) != set.end());
    EXPECT_FALSE(set.find({0, 0, true}) != set.end());
}

TEST(ComposeSeatStyleId, Basic) {
    auto id = composeSeatStyleId("A", 0, false);
    EXPECT_EQ(id, "pricecode_A_status_0_selected_0");
}

TEST(ComposeSeatStyleId, Selected) {
    auto id = composeSeatStyleId("VIP", 5, true);
    EXPECT_EQ(id, "pricecode_VIP_status_5_selected_1");
}

TEST(ComposeSeatStyleId, EmptyPricecode) {
    auto id = composeSeatStyleId("", 0, false);
    EXPECT_EQ(id, "pricecode__status_0_selected_0");
}

TEST(ParseSeatStyleId, RoundTrip) {
    auto original = composeSeatStyleId("A1", 42, true);
    auto parsed = parseSeatStyleId(original);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->pricecode, "A1");
    EXPECT_EQ(parsed->status, 42u);
    EXPECT_TRUE(parsed->selected);
}

TEST(ParseSeatStyleId, RoundTripNotSelected) {
    auto original = composeSeatStyleId("B2", 100, false);
    auto parsed = parseSeatStyleId(original);
    ASSERT_TRUE(parsed.has_value());
    EXPECT_EQ(parsed->pricecode, "B2");
    EXPECT_EQ(parsed->status, 100u);
    EXPECT_FALSE(parsed->selected);
}

TEST(ParseSeatStyleId, RejectInvalidPrefix) {
    auto parsed = parseSeatStyleId("invalid_key");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, RejectMissingStatus) {
    auto parsed = parseSeatStyleId("pricecode_A");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, RejectMissingSelected) {
    auto parsed = parseSeatStyleId("pricecode_A_status_1");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, RejectInvalidSelected) {
    auto parsed = parseSeatStyleId("pricecode_A_status_1_selected_2");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, RejectInvalidStatus) {
    auto parsed = parseSeatStyleId("pricecode_A_status_abc_selected_0");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, RejectEmpty) {
    auto parsed = parseSeatStyleId("");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, RejectShortString) {
    auto parsed = parseSeatStyleId("p");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ParseSeatStyleId, StatusOverflow) {
    // status value超過 uint32_t 最大值
    auto parsed = parseSeatStyleId("pricecode_X_status_999999999999_selected_0");
    EXPECT_FALSE(parsed.has_value());
}

TEST(ResolvePricecodeIndex, Found) {
    std::unordered_map<std::string, uint16_t> map = {{"A", 0}, {"B", 1}};
    EXPECT_EQ(resolvePricecodeIndex(map, "A"), 0);
    EXPECT_EQ(resolvePricecodeIndex(map, "B"), 1);
}

TEST(ResolvePricecodeIndex, NotFound) {
    std::unordered_map<std::string, uint16_t> map = {{"A", 0}};
    EXPECT_EQ(resolvePricecodeIndex(map, "C"), kNoPricecodeIndex);
}

TEST(ResolvePricecodeIndex, EmptyPricecode) {
    std::unordered_map<std::string, uint16_t> map = {{"A", 0}};
    EXPECT_EQ(resolvePricecodeIndex(map, ""), kNoPricecodeIndex);
}
