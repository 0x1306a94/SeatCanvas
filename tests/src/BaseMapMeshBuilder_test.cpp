#include "core/renderer/BaseMapMeshBuilder.hpp"

#include <gtest/gtest.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Rect.h>

#include <memory>

using namespace kk::renderer;

static std::shared_ptr<ZoneMeshInfo> makeZoneInfo(const std::string &zoneId,
                                                  float bx, float by, float bw, float bh) {
    auto info = std::make_shared<ZoneMeshInfo>();
    info->zoneId = zoneId;
    info->path = std::make_shared<tgfx::Path>();
    info->path->addRect(tgfx::Rect::MakeXYWH(bx, by, bw, bh));
    info->fillBounds = tgfx::Rect::MakeXYWH(bx, by, bw, bh);
    info->strokeBounds = tgfx::Rect::MakeXYWH(bx, by, bw, bh);
    info->fillVertices.push_back({0.f, 0.f, 1.f, 0});
    return info;
}

// ---------- addZoneMeshInfo ----------

TEST(BaseMapMeshBuilder, AddNullZoneInfoIsNoOp) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(nullptr);
    EXPECT_TRUE(builder.getZoneMeshInfos().empty());
    EXPECT_EQ(builder.getTotalVertexCount(), 0u);
}

TEST(BaseMapMeshBuilder, AddInvalidZoneInfoIsNoOp) {
    BaseMapMeshBuilder builder;
    auto info = std::make_shared<ZoneMeshInfo>();
    info->zoneId = "test";
    // No path, no vertices → invalid
    builder.addZoneMeshInfo(info);
    EXPECT_TRUE(builder.getZoneMeshInfos().empty());
}

TEST(BaseMapMeshBuilder, AddValidZoneInfo) {
    BaseMapMeshBuilder builder;
    auto info = makeZoneInfo("zone1", 0.f, 0.f, 100.f, 100.f);
    builder.addZoneMeshInfo(info);

    EXPECT_EQ(builder.getZoneMeshInfos().size(), 1u);
    EXPECT_GT(builder.getTotalVertexCount(), 0u);
}

TEST(BaseMapMeshBuilder, AddMultipleZonesAccumulatesVertexCount) {
    BaseMapMeshBuilder builder;
    auto z1 = makeZoneInfo("z1", 0.f, 0.f, 100.f, 100.f);
    auto z2 = makeZoneInfo("z2", 100.f, 0.f, 100.f, 100.f);

    z1->fillVertices.push_back({1.f, 1.f, 1.f, 0});
    z2->fillVertices.push_back({2.f, 2.f, 1.f, 0});

    builder.addZoneMeshInfo(z1);
    size_t count1 = builder.getTotalVertexCount();

    builder.addZoneMeshInfo(z2);
    size_t count2 = builder.getTotalVertexCount();

    EXPECT_GT(count2, count1);
}

// ---------- findZoneById ----------

TEST(BaseMapMeshBuilder, FindZoneByIdExisting) {
    BaseMapMeshBuilder builder;
    auto info = makeZoneInfo("zone_a", 0.f, 0.f, 50.f, 50.f);
    builder.addZoneMeshInfo(info);

    auto found = builder.findZoneById("zone_a");
    EXPECT_EQ(found, info);
}

TEST(BaseMapMeshBuilder, FindZoneByIdNonExisting) {
    BaseMapMeshBuilder builder;
    auto info = makeZoneInfo("zone_a", 0.f, 0.f, 50.f, 50.f);
    builder.addZoneMeshInfo(info);

    EXPECT_EQ(builder.findZoneById("zone_b"), nullptr);
}

TEST(BaseMapMeshBuilder, FindZoneByIdEmptyString) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("zone_a", 0.f, 0.f, 50.f, 50.f));
    EXPECT_EQ(builder.findZoneById(""), nullptr);
}

// ---------- findZoneDrawRangeById ----------

TEST(BaseMapMeshBuilder, FindDrawRangeByIdExisting) {
    BaseMapMeshBuilder builder;
    auto info = makeZoneInfo("zone_x", 0.f, 0.f, 10.f, 10.f);
    builder.addZoneMeshInfo(info);

    auto range = builder.findZoneDrawRangeById("zone_x");
    EXPECT_TRUE(range.has_value());
    EXPECT_GT(range->vertexCount, 0u);
}

TEST(BaseMapMeshBuilder, FindDrawRangeByIdNonExisting) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("zone_x", 0.f, 0.f, 10.f, 10.f));

    auto range = builder.findZoneDrawRangeById("zone_y");
    EXPECT_FALSE(range.has_value());
}

TEST(BaseMapMeshBuilder, FindDrawRangeByIdEmptyString) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("zone_x", 0.f, 0.f, 10.f, 10.f));

    auto range = builder.findZoneDrawRangeById("");
    EXPECT_FALSE(range.has_value());
}

// ---------- findZoneDrawRangeByIndex ----------

TEST(BaseMapMeshBuilder, FindDrawRangeByValidIndex) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("z1", 0.f, 0.f, 10.f, 10.f));
    builder.addZoneMeshInfo(makeZoneInfo("z2", 10.f, 0.f, 10.f, 10.f));

    auto range = builder.findZoneDrawRangeByIndex(0);
    EXPECT_TRUE(range.has_value());

    auto range2 = builder.findZoneDrawRangeByIndex(1);
    EXPECT_TRUE(range2.has_value());
}

TEST(BaseMapMeshBuilder, FindDrawRangeByOutOfBoundsIndex) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("z1", 0.f, 0.f, 10.f, 10.f));

    EXPECT_FALSE(builder.findZoneDrawRangeByIndex(10).has_value());
    EXPECT_FALSE(builder.findZoneDrawRangeByIndex(1).has_value());
}

TEST(BaseMapMeshBuilder, FindDrawRangeByIndexEmptyBuilder) {
    BaseMapMeshBuilder builder;
    EXPECT_FALSE(builder.findZoneDrawRangeByIndex(0).has_value());
}

// ---------- findZoneIntersectingRect ----------

TEST(BaseMapMeshBuilder, FindZoneIntersectingRectReturnsMatchingZones) {
    BaseMapMeshBuilder builder;
    auto z1 = makeZoneInfo("z1", 0.f, 0.f, 50.f, 50.f);
    auto z2 = makeZoneInfo("z2", 100.f, 0.f, 50.f, 50.f);
    auto z3 = makeZoneInfo("z3", 0.f, 100.f, 50.f, 50.f);
    builder.addZoneMeshInfo(z1);
    builder.addZoneMeshInfo(z2);
    builder.addZoneMeshInfo(z3);

    tgfx::Rect query = tgfx::Rect::MakeXYWH(0.f, 0.f, 60.f, 60.f);
    auto result = builder.findZoneIntersectingRect(query);

    // z1 (0,0,50,50) intersects, z3 (0,100,50,50) does not
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0]->zoneId, "z1");
}

TEST(BaseMapMeshBuilder, FindZoneIntersectingRectNoMatchReturnsEmpty) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("z1", 0.f, 0.f, 50.f, 50.f));

    tgfx::Rect query = tgfx::Rect::MakeXYWH(200.f, 200.f, 10.f, 10.f);
    auto result = builder.findZoneIntersectingRect(query);
    EXPECT_TRUE(result.empty());
}

TEST(BaseMapMeshBuilder, FindZoneIntersectingRectPopulatesVisibleIndices) {
    BaseMapMeshBuilder builder;
    auto z1 = makeZoneInfo("z1", 0.f, 0.f, 50.f, 50.f);
    auto z2 = makeZoneInfo("z2", 25.f, 25.f, 50.f, 50.f);
    builder.addZoneMeshInfo(z1);
    builder.addZoneMeshInfo(z2);

    std::vector<size_t> indices;
    tgfx::Rect query = tgfx::Rect::MakeXYWH(0.f, 0.f, 100.f, 100.f);
    auto result = builder.findZoneIntersectingRect(query, &indices);

    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(indices.size(), 2u);
    EXPECT_EQ(indices[0], 0u);
    EXPECT_EQ(indices[1], 1u);
}

// ---------- findZoneContainingPoint ----------

TEST(BaseMapMeshBuilder, FindZoneContainingPointInside) {
    BaseMapMeshBuilder builder;
    auto info = makeZoneInfo("zone_in", 0.f, 0.f, 100.f, 100.f);
    builder.addZoneMeshInfo(info);

    auto found = builder.findZoneContainingPoint(tgfx::Point::Make(50.f, 50.f));
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->zoneId, "zone_in");
}

TEST(BaseMapMeshBuilder, FindZoneContainingPointOutside) {
    BaseMapMeshBuilder builder;
    builder.addZoneMeshInfo(makeZoneInfo("zone_in", 0.f, 0.f, 100.f, 100.f));

    auto found = builder.findZoneContainingPoint(tgfx::Point::Make(200.f, 200.f));
    EXPECT_EQ(found, nullptr);
}

TEST(BaseMapMeshBuilder, FindZoneContainingPointReturnsTopmostZone) {
    BaseMapMeshBuilder builder;
    // Add z1 then z2 — z2 is on top (last added)
    auto z1 = makeZoneInfo("z1", 0.f, 0.f, 100.f, 100.f);
    auto z2 = makeZoneInfo("z2", 10.f, 10.f, 80.f, 80.f);
    builder.addZoneMeshInfo(z1);
    builder.addZoneMeshInfo(z2);

    // Point (50,50) is inside both — should return z2 (topmost)
    auto found = builder.findZoneContainingPoint(tgfx::Point::Make(50.f, 50.f));
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(found->zoneId, "z2");
}
