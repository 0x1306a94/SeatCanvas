#include "core/EdgeInsets.h"
#include "core/SeatData.hpp"
#include "core/ZoomLevelConfig.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"

#include <gtest/gtest.h>

using namespace kk;
using namespace kk::renderer;

// ---------- SeatCanvasCoreRendererState ----------

TEST(SeatCanvasCoreRendererState, DefaultConstructor) {
    SeatCanvasCoreRendererState state;
    EXPECT_FLOAT_EQ(state.getBoundsSize().width, 1280.f);
    EXPECT_FLOAT_EQ(state.getBoundsSize().height, 720.f);
    EXPECT_FLOAT_EQ(state.getDensity(), 1.f);
    EXPECT_FLOAT_EQ(state.getContentScale(), 1.f);
    EXPECT_FLOAT_EQ(state.getZoomScale(), 1.f);
}

TEST(SeatCanvasCoreRendererState, CustomConstructor) {
    SeatCanvasCoreRendererState state(640, 480, 2.f);
    EXPECT_FLOAT_EQ(state.getBoundsSize().width, 640.f);
    EXPECT_FLOAT_EQ(state.getBoundsSize().height, 480.f);
    EXPECT_FLOAT_EQ(state.getDensity(), 2.f);
}

TEST(SeatCanvasCoreRendererState, UpdateNormalizedContentSizeValid) {
    SeatCanvasCoreRendererState state;
    EXPECT_TRUE(state.updateNormalizedContentSize(tgfx::Size::Make(1024.f, 768.f)));
    EXPECT_FLOAT_EQ(state.getNormalizedContentSize().width, 1024.f);
    EXPECT_FLOAT_EQ(state.getNormalizedContentSize().height, 768.f);
}

TEST(SeatCanvasCoreRendererState, UpdateNormalizedContentSizeRejectsZero) {
    SeatCanvasCoreRendererState state;
    EXPECT_FALSE(state.updateNormalizedContentSize(tgfx::Size::Make(0.f, 100.f)));
    EXPECT_FALSE(state.updateNormalizedContentSize(tgfx::Size::Make(100.f, 0.f)));
    EXPECT_FALSE(state.updateNormalizedContentSize(tgfx::Size::Make(-1.f, 100.f)));
}

TEST(SeatCanvasCoreRendererState, UpdateNormalizedContentSizeNoChangeReturnsFalse) {
    SeatCanvasCoreRendererState state;
    EXPECT_TRUE(state.updateNormalizedContentSize(tgfx::Size::Make(100.f, 200.f)));
    // Same size should return false
    EXPECT_FALSE(state.updateNormalizedContentSize(tgfx::Size::Make(100.f, 200.f)));
}

TEST(SeatCanvasCoreRendererState, UpdateOriginSize) {
    SeatCanvasCoreRendererState state;
    EXPECT_TRUE(state.updateOriginSize(tgfx::Size::Make(800.f, 600.f)));
    EXPECT_FLOAT_EQ(state.getOriginSize().width, 800.f);
    EXPECT_FLOAT_EQ(state.getOriginSize().height, 600.f);
}

TEST(SeatCanvasCoreRendererState, UpdateContentScale) {
    SeatCanvasCoreRendererState state;
    EXPECT_TRUE(state.updateContentScale(2.5f));
    EXPECT_FLOAT_EQ(state.getContentScale(), 2.5f);
}

TEST(SeatCanvasCoreRendererState, UpdateScreenValid) {
    SeatCanvasCoreRendererState state;
    EXPECT_TRUE(state.updateScreen(1920, 1080, 2.f));
    EXPECT_FLOAT_EQ(state.getBoundsSize().width, 1920.f);
    EXPECT_FLOAT_EQ(state.getBoundsSize().height, 1080.f);
    EXPECT_FLOAT_EQ(state.getDensity(), 2.f);
}

TEST(SeatCanvasCoreRendererState, UpdateScreenRejectsInvalidWidthHeight) {
    SeatCanvasCoreRendererState state;
    EXPECT_FALSE(state.updateScreen(0, 100, 1.f));
    EXPECT_FALSE(state.updateScreen(100, 0, 1.f));
    EXPECT_FALSE(state.updateScreen(-1, 100, 1.f));
}

TEST(SeatCanvasCoreRendererState, UpdateScreenRejectsInvalidDensity) {
    SeatCanvasCoreRendererState state;
    EXPECT_FALSE(state.updateScreen(100, 100, 0.5f));
}

TEST(SeatCanvasCoreRendererState, UpdateScreenNoChangeReturnsFalse) {
    SeatCanvasCoreRendererState state(100, 200, 1.f);
    EXPECT_FALSE(state.updateScreen(100, 200, 1.f));
}

TEST(SeatCanvasCoreRendererState, UpdateZoomAndOffset) {
    SeatCanvasCoreRendererState state;
    EXPECT_TRUE(state.updateZoomAndOffset(2.f, tgfx::Point::Make(10.f, 20.f)));
    EXPECT_FLOAT_EQ(state.getZoomScale(), 2.f);
    EXPECT_FLOAT_EQ(state.getContentOffset().x, 10.f);
    EXPECT_FLOAT_EQ(state.getContentOffset().y, 20.f);
}

TEST(SeatCanvasCoreRendererState, UpdateZoomAndOffsetNoChangeReturnsFalse) {
    SeatCanvasCoreRendererState state;
    state.updateZoomAndOffset(2.f, tgfx::Point::Make(10.f, 20.f));
    EXPECT_FALSE(state.updateZoomAndOffset(2.f, tgfx::Point::Make(10.f, 20.f)));
}

TEST(SeatCanvasCoreRendererState, GetMVMatrixReturnsIdentityForInvalidState) {
    SeatCanvasCoreRendererState state;
    // No origin/normalized size set, should return identity
    auto matrix = state.getMVMatrix();
    EXPECT_TRUE(matrix.isIdentity());
}

TEST(SeatCanvasCoreRendererState, GetMVMatrixComputesCorrectly) {
    SeatCanvasCoreRendererState state(1000, 1000, 1.f);
    state.updateOriginSize(tgfx::Size::Make(500.f, 500.f));
    state.updateNormalizedContentSize(tgfx::Size::Make(1000.f, 1000.f));
    state.updateZoomAndOffset(1.f, tgfx::Point::Make(0.f, 0.f));

    auto matrix = state.getMVMatrix();
    // Model: scale 1000/500 = 2, View: scale 1, translate 0
    // So MV should scale by 2
    auto point = tgfx::Point::Make(100.f, 100.f);
    matrix.mapPoints(&point, 1);
    EXPECT_FLOAT_EQ(point.x, 200.f);
    EXPECT_FLOAT_EQ(point.y, 200.f);
}

TEST(SeatCanvasCoreRendererState, GetMVMatrixWithZoomAndOffset) {
    SeatCanvasCoreRendererState state(1000, 1000, 1.f);
    state.updateOriginSize(tgfx::Size::Make(500.f, 500.f));
    state.updateNormalizedContentSize(tgfx::Size::Make(1000.f, 1000.f));
    state.updateZoomAndOffset(2.f, tgfx::Point::Make(100.f, 200.f));

    auto matrix = state.getMVMatrix();
    // Model: scale 2 (1000/500), View: scale 2, translate (100, 200)
    // MV = View * Model: point * scale2 * scale2 + translate
    auto point = tgfx::Point::Make(10.f, 10.f);
    matrix.mapPoints(&point, 1);
    EXPECT_FLOAT_EQ(point.x, 10.f * 4.f + 100.f);  // 10*2*2 + 100 = 140
    EXPECT_FLOAT_EQ(point.y, 10.f * 4.f + 200.f);  // 10*2*2 + 200 = 240
}

TEST(SeatCanvasCoreRendererState, GetMVPMatrixReturnsIdentityForInvalidState) {
    SeatCanvasCoreRendererState state;
    auto matrix = state.getMVPMatrix();
    EXPECT_TRUE(matrix.isIdentity());
}

TEST(SeatCanvasCoreRendererState, GetMVPMatrixComputesCorrectly) {
    SeatCanvasCoreRendererState state(200.f, 200.f, 1.f);
    state.updateOriginSize(tgfx::Size::Make(100.f, 100.f));
    state.updateNormalizedContentSize(tgfx::Size::Make(100.f, 100.f));
    state.updateZoomAndOffset(2.f, tgfx::Point::Make(0.f, 0.f));

    // At the center of a 200x200 viewport with zoom 2:
    // Content coord (50, 50) -> Screen (100, 100) -> NDC (0, 0)
    auto matrix = state.getMVPMatrix();
    auto point = tgfx::Point::Make(50.f, 50.f);
    matrix.mapPoints(&point, 1);
    EXPECT_NEAR(point.x, 0.f, 1e-6f);
    EXPECT_NEAR(point.y, 0.f, 1e-6f);
}

TEST(SeatCanvasCoreRendererState, GetVisibleOriginalRectReturnsEmptyForInvalidState) {
    SeatCanvasCoreRendererState state;
    auto rect = state.getVisibleOriginalRect();
    EXPECT_TRUE(rect.isEmpty());
}

TEST(SeatCanvasCoreRendererState, GetVisibleOriginalRectBasic) {
    SeatCanvasCoreRendererState state(200.f, 200.f, 2.f);
    state.updateOriginSize(tgfx::Size::Make(100.f, 100.f));
    state.updateNormalizedContentSize(tgfx::Size::Make(400.f, 400.f));
    // normalizedContentSize = originSize * contentScale * density
    // 400 = 100 * contentScale * 2 => contentScale = 2
    state.updateContentScale(2.f);
    state.updateZoomAndOffset(1.f, tgfx::Point::Make(0.f, 0.f));

    auto rect = state.getVisibleOriginalRect();
    EXPECT_FALSE(rect.isEmpty());
    // Screen 200x200, zoom 1 → content area 200x200 in normalized space
    // Scale factor = 2 * 2 = 4
    // Original visible: 200/4 = 50
    EXPECT_NEAR(rect.width(), 50.f, 1e-6f);
    EXPECT_NEAR(rect.height(), 50.f, 1e-6f);
}

TEST(SeatCanvasCoreRendererState, GetContentOffsetInitiallyZero) {
    SeatCanvasCoreRendererState state;
    EXPECT_FLOAT_EQ(state.getContentOffset().x, 0.f);
    EXPECT_FLOAT_EQ(state.getContentOffset().y, 0.f);
}

// ---------- ZoomLevelConfig ----------

TEST(ZoomLevelConfig, DefaultValues) {
    ZoomLevelConfig config;
    EXPECT_FLOAT_EQ(config.seat, 1.f);
    EXPECT_FLOAT_EQ(config.row, 1.f);
    EXPECT_FLOAT_EQ(config.zone, 1.f);
    EXPECT_FLOAT_EQ(config.venue, 1.f);
}

TEST(IsSmallVenue, VenueGreaterThanOne) {
    ZoomLevelConfig config;
    config.venue = 2.f;
    EXPECT_FALSE(isSmallVenue(config));
}

TEST(IsSmallVenue, VenueEqualToOne) {
    ZoomLevelConfig config;
    config.venue = 1.f;
    EXPECT_FALSE(isSmallVenue(config));
}

TEST(IsSmallVenue, VenueLessThanOne) {
    ZoomLevelConfig config;
    config.venue = 0.5f;
    EXPECT_TRUE(isSmallVenue(config));
}

TEST(ShowBackZoomThreshold, SmallVenueUsesZone) {
    ZoomLevelConfig config;
    config.venue = 0.5f;
    config.zone = 2.f;
    EXPECT_FLOAT_EQ(showBackZoomThreshold(config), 2.f);
}

TEST(ShowBackZoomThreshold, NormalVenueUsesVenue) {
    ZoomLevelConfig config;
    config.venue = 3.f;
    config.zone = 2.f;
    EXPECT_FLOAT_EQ(showBackZoomThreshold(config), 3.f);
}

// ---------- SeatData ----------

TEST(SeatData, DefaultConstructorIsInvalid) {
    SeatData data;
    EXPECT_FALSE(data.isValid());
    EXPECT_EQ(data.pricecodeIndex, kNoPricecodeIndex);
}

TEST(SeatData, ConstructorWithSeatIdIsValid) {
    SeatData data("S1", 10.f, 20.f);
    EXPECT_TRUE(data.isValid());
    EXPECT_EQ(data.seatId, "S1");
    EXPECT_FLOAT_EQ(data.x, 10.f);
    EXPECT_FLOAT_EQ(data.y, 20.f);
}

TEST(SeatData, EmptySeatIdIsInvalid) {
    SeatData data("", 0.f, 0.f);
    EXPECT_FALSE(data.isValid());
}

TEST(SeatData, FullConstructor) {
    SeatData data("S1", 100.f, 200.f, 45.f, 3);
    EXPECT_TRUE(data.isValid());
    EXPECT_FLOAT_EQ(data.rotation, 45.f);
    EXPECT_EQ(data.pricecodeIndex, 3u);
}

TEST(SeatData, KNoPricecodeIndex) {
    EXPECT_EQ(kNoPricecodeIndex, std::numeric_limits<uint16_t>::max());
}

// ---------- EdgeInsets ----------

TEST(EdgeInsets, DefaultConstructor) {
    EdgeInsets insets;
    EXPECT_FLOAT_EQ(insets.top, 0.f);
    EXPECT_FLOAT_EQ(insets.left, 0.f);
    EXPECT_FLOAT_EQ(insets.bottom, 0.f);
    EXPECT_FLOAT_EQ(insets.right, 0.f);
    EXPECT_TRUE(insets.isEmpty());
}

TEST(EdgeInsets, UniformConstructor) {
    EdgeInsets insets(10.f);
    EXPECT_FLOAT_EQ(insets.top, 10.f);
    EXPECT_FLOAT_EQ(insets.left, 10.f);
    EXPECT_FLOAT_EQ(insets.bottom, 10.f);
    EXPECT_FLOAT_EQ(insets.right, 10.f);
    EXPECT_FALSE(insets.isEmpty());
}

TEST(EdgeInsets, FourValueConstructor) {
    EdgeInsets insets(1.f, 2.f, 3.f, 4.f);
    EXPECT_FLOAT_EQ(insets.top, 1.f);
    EXPECT_FLOAT_EQ(insets.left, 2.f);
    EXPECT_FLOAT_EQ(insets.bottom, 3.f);
    EXPECT_FLOAT_EQ(insets.right, 4.f);
}

TEST(EdgeInsets, Horizontal) {
    EdgeInsets insets(0.f, 5.f, 0.f, 10.f);
    EXPECT_FLOAT_EQ(insets.horizontal(), 15.f);
}

TEST(EdgeInsets, Vertical) {
    EdgeInsets insets(5.f, 0.f, 10.f, 0.f);
    EXPECT_FLOAT_EQ(insets.vertical(), 15.f);
}

TEST(EdgeInsets, Equality) {
    EdgeInsets a(1.f, 2.f, 3.f, 4.f);
    EdgeInsets b(1.f, 2.f, 3.f, 4.f);
    EdgeInsets c(1.f, 2.f, 3.f, 5.f);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
}

TEST(EdgeInsets, IsEmpty) {
    EXPECT_TRUE(EdgeInsets().isEmpty());
    EXPECT_TRUE(EdgeInsets(0.f).isEmpty());
    EXPECT_FALSE(EdgeInsets(1.f).isEmpty());
    EXPECT_FALSE(EdgeInsets(0.f, 1.f, 0.f, 0.f).isEmpty());
}
