#include "core/style/SeatStyleKey.hpp"

#include <gtest/gtest.h>
#include <map>
#include <unordered_map>
#include <unordered_set>

using namespace kk;

TEST(SeatStyleKey, DefaultConstructor) {
    SeatStyleKey key;
    EXPECT_EQ(key.getStatus(), 0u);
    EXPECT_FALSE(key.isSelected());
}

TEST(SeatStyleKey, ConstructorWithValues) {
    SeatStyleKey key(5, true);
    EXPECT_EQ(key.getStatus(), 5u);
    EXPECT_TRUE(key.isSelected());
}

TEST(SeatStyleKey, EqualitySameValues) {
    SeatStyleKey a(1, true);
    SeatStyleKey b(1, true);
    EXPECT_EQ(a, b);
}

TEST(SeatStyleKey, EqualityDifferentStatus) {
    SeatStyleKey a(1, true);
    SeatStyleKey b(2, true);
    EXPECT_FALSE(a == b);
}

TEST(SeatStyleKey, EqualityDifferentSelected) {
    SeatStyleKey a(1, true);
    SeatStyleKey b(1, false);
    EXPECT_FALSE(a == b);
}

TEST(SeatStyleKey, LessThanByStatus) {
    SeatStyleKey a(1, false);
    SeatStyleKey b(2, false);
    EXPECT_LT(a, b);
    EXPECT_FALSE(b < a);
}

TEST(SeatStyleKey, LessThanBySelected) {
    // When status is equal, selected=false comes before selected=true
    SeatStyleKey a(1, false);
    SeatStyleKey b(1, true);
    EXPECT_LT(a, b);
}

TEST(SeatStyleKey, HashSameKeys) {
    SeatStyleKey a(42, true);
    SeatStyleKey b(42, true);
    EXPECT_EQ(a.hash(), b.hash());
}

TEST(SeatStyleKey, HashDifferentKeys) {
    SeatStyleKey a(1, false);
    SeatStyleKey b(1, true);
    SeatStyleKey c(2, false);
    EXPECT_NE(a.hash(), b.hash());
    EXPECT_NE(a.hash(), c.hash());
}

TEST(SeatStyleKey, StdHashSpecialization) {
    std::hash<SeatStyleKey> hasher;
    SeatStyleKey a(42, true);
    SeatStyleKey b(42, true);
    EXPECT_EQ(hasher(a), hasher(b));
}

TEST(SeatStyleKey, UsableInUnorderedSet) {
    std::unordered_set<SeatStyleKey> set;
    set.insert(SeatStyleKey(0, false));
    set.insert(SeatStyleKey(0, true));
    set.insert(SeatStyleKey(1, false));
    EXPECT_EQ(set.size(), 3u);
    EXPECT_TRUE(set.find(SeatStyleKey(0, false)) != set.end());
    EXPECT_FALSE(set.find(SeatStyleKey(2, false)) != set.end());
}

TEST(SeatStyleKey, UsableInUnorderedMap) {
    std::unordered_map<SeatStyleKey, std::string> map;
    map[SeatStyleKey(0, false)] = "available";
    map[SeatStyleKey(0, true)] = "selected";
    EXPECT_EQ(map.size(), 2u);
    EXPECT_EQ(map[SeatStyleKey(0, false)], "available");
    EXPECT_EQ(map[SeatStyleKey(0, true)], "selected");
}

TEST(SeatStyleKey, UsableInStdMap) {
    std::map<SeatStyleKey, int> map;
    map[SeatStyleKey(2, false)] = 10;
    map[SeatStyleKey(1, true)] = 20;
    map[SeatStyleKey(1, false)] = 30;
    EXPECT_EQ(map.size(), 3u);
    // std::map orders by key
    auto it = map.begin();
    EXPECT_EQ(it->first, SeatStyleKey(1, false));
}
