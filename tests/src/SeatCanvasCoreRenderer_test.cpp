#include "SeatCanvasTestFixture.hpp"
#include "TestFileUtils.hpp"
#include "core/BaseMapColorState.h"
#include "core/SeatData.hpp"
#include "core/ZoomLevelConfig.hpp"
#include "core/gesture/ElasticZoomPanController.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/renderer/PlatformView.hpp"
#include "core/renderer/SeatCanvasCoreRenderer.hpp"
#include "core/renderer/SeatCanvasCoreRendererState.hpp"
#include "core/style/CircleSeatStyleConfig.hpp"
#include "core/style/SeatRenderStyleKey.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <tgfx/core/Color.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

using namespace kk::renderer;

namespace kk::test {

// ---------- coreID & state ----------

TEST_F(SeatCanvasTestFixture, CoreIdIsNonZero) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_GT(renderer->coreID(), 0u);
}

TEST_F(SeatCanvasTestFixture, StateIsNonNull) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_NE(renderer->state(), nullptr);
}

// ---------- debug HUD ----------

TEST_F(SeatCanvasTestFixture, DebugHUDDisabledByDefault) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_FALSE(renderer->isDebugHUDEnabled());
}

TEST_F(SeatCanvasTestFixture, DebugHUDToggle) {
    ASSERT_NE(renderer, nullptr);
    renderer->setDebugHUDEnabled(true);
    EXPECT_TRUE(renderer->isDebugHUDEnabled());
    renderer->setDebugHUDEnabled(false);
    EXPECT_FALSE(renderer->isDebugHUDEnabled());
}

TEST_F(SeatCanvasTestFixture, DebugHUDIdempotent) {
    ASSERT_NE(renderer, nullptr);
    renderer->setDebugHUDEnabled(true);
    renderer->setDebugHUDEnabled(true);
    EXPECT_TRUE(renderer->isDebugHUDEnabled());
}

// ---------- background color ----------

TEST_F(SeatCanvasTestFixture, BackgroundColorDefaultWhite) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_EQ(renderer->getBackgroundColor(), tgfx::Color::White());
}

TEST_F(SeatCanvasTestFixture, BackgroundColorSetAndGet) {
    ASSERT_NE(renderer, nullptr);
    renderer->setBackgroundColor(tgfx::Color::Red());
    EXPECT_EQ(renderer->getBackgroundColor(), tgfx::Color::Red());
}

TEST_F(SeatCanvasTestFixture, BackgroundColorIdempotent) {
    ASSERT_NE(renderer, nullptr);
    auto before = renderer->getBackgroundColor();
    renderer->setBackgroundColor(before);
    // Should not trigger invalidate (state unchanged)
    EXPECT_EQ(renderer->getBackgroundColor(), before);
}

// ---------- seat size ----------

TEST_F(SeatCanvasTestFixture, SeatSizeDefault) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_FLOAT_EQ(renderer->getSeatSize(), 36.0f);
}

TEST_F(SeatCanvasTestFixture, SeatSizeSetAndGet) {
    ASSERT_NE(renderer, nullptr);
    renderer->setSeatSize(48.0f);
    EXPECT_FLOAT_EQ(renderer->getSeatSize(), 48.0f);
}

// ---------- FPS ----------

TEST_F(SeatCanvasTestFixture, FPSInitiallyZero) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_FLOAT_EQ(renderer->getFPS(), 0.0f);
}

// ---------- seat render zoom threshold ----------

TEST_F(SeatCanvasTestFixture, SeatRenderZoomThresholdDefaultFallsBack) {
    ASSERT_NE(renderer, nullptr);
    // When threshold is not explicitly set, falls back to _zoomLevelConfig.venue
    // After construction with updateSize(), venue gets populated with min zoom
    auto threshold = renderer->getSeatRenderZoomThreshold();
    EXPECT_GT(threshold, 0.0f);
}

TEST_F(SeatCanvasTestFixture, SeatRenderZoomThresholdSetAndGet) {
    ASSERT_NE(renderer, nullptr);
    renderer->setSeatRenderZoomThreshold(1.5f);
    EXPECT_FLOAT_EQ(renderer->getSeatRenderZoomThreshold(), 1.5f);
}

TEST_F(SeatCanvasTestFixture, SeatRenderZoomThresholdZeroFallsBackToVenue) {
    ASSERT_NE(renderer, nullptr);
    auto venueLevel = renderer->zoomLevelConfig().venue;
    renderer->setSeatRenderZoomThreshold(1.5f);
    renderer->setSeatRenderZoomThreshold(0.0f);
    // Setting to 0 means "use venue" again
    EXPECT_FLOAT_EQ(renderer->getSeatRenderZoomThreshold(), venueLevel);
}

// ---------- coordinate conversions ----------

TEST_F(SeatCanvasTestFixture, ConvertScreenToContentBasic) {
    ASSERT_NE(renderer, nullptr);
    auto result = renderer->convertScreenToContent(
        tgfx::Point::Make(100.f, 200.f),
        tgfx::Point::Make(0.f, 0.f),
        1.0f);
    EXPECT_FLOAT_EQ(result.x, 100.f);
    EXPECT_FLOAT_EQ(result.y, 200.f);
}

TEST_F(SeatCanvasTestFixture, ConvertScreenToContentWithOffset) {
    ASSERT_NE(renderer, nullptr);
    // screen(200, 300), offset(50, 100), scale(2.0) → content((200-50)/2, (300-100)/2)
    auto result = renderer->convertScreenToContent(
        tgfx::Point::Make(200.f, 300.f),
        tgfx::Point::Make(50.f, 100.f),
        2.0f);
    EXPECT_FLOAT_EQ(result.x, 75.f);
    EXPECT_FLOAT_EQ(result.y, 100.f);
}

TEST_F(SeatCanvasTestFixture, ConvertContentToScreenBasic) {
    ASSERT_NE(renderer, nullptr);
    auto result = renderer->convertContentToScreen(
        tgfx::Point::Make(50.f, 60.f),
        tgfx::Point::Make(0.f, 0.f),
        1.0f);
    EXPECT_FLOAT_EQ(result.x, 50.f);
    EXPECT_FLOAT_EQ(result.y, 60.f);
}

TEST_F(SeatCanvasTestFixture, ConvertContentToScreenWithOffsetAndScale) {
    ASSERT_NE(renderer, nullptr);
    // content(10, 20), offset(5, 15), scale(3.0) → screen(10*3+5, 20*3+15)
    auto result = renderer->convertContentToScreen(
        tgfx::Point::Make(10.f, 20.f),
        tgfx::Point::Make(5.f, 15.f),
        3.0f);
    EXPECT_FLOAT_EQ(result.x, 35.f);
    EXPECT_FLOAT_EQ(result.y, 75.f);
}

TEST_F(SeatCanvasTestFixture, ConvertScreenContentRoundtrip) {
    ASSERT_NE(renderer, nullptr);
    tgfx::Point original(150.f, 250.f);
    tgfx::Point offset(20.f, 30.f);
    float scale = 2.5f;

    auto content = renderer->convertScreenToContent(original, offset, scale);
    auto back = renderer->convertContentToScreen(content, offset, scale);

    EXPECT_NEAR(back.x, original.x, 0.001f);
    EXPECT_NEAR(back.y, original.y, 0.001f);
}

TEST_F(SeatCanvasTestFixture, ConvertNormalizedToOriginalRoundtrip) {
    ASSERT_NE(renderer, nullptr);
    tgfx::Point originalLoc(100.f, 200.f);
    auto normalized = renderer->convertOriginalToNormalizedContent(originalLoc);
    auto back = renderer->convertNormalizedContentToOriginal(normalized);

    EXPECT_NEAR(back.x, originalLoc.x, 0.001f);
    EXPECT_NEAR(back.y, originalLoc.y, 0.001f);
}

TEST_F(SeatCanvasTestFixture, ConvertScreenToOriginalRoundtrip) {
    ASSERT_NE(renderer, nullptr);
    // Set known zoom/offset state
    renderer->setZoomScale(1.5f);
    renderer->setContentOffset(tgfx::Point::Make(100.f, 200.f));

    auto screenLoc = renderer->convertOriginalToScreen(tgfx::Point::Make(50.f, 60.f));
    auto originalLoc = renderer->convertScreenToOriginal(screenLoc);

    EXPECT_NEAR(originalLoc.x, 50.f, 0.1f);
    EXPECT_NEAR(originalLoc.y, 60.f, 0.1f);
}

TEST_F(SeatCanvasTestFixture, ConvertOriginalToScreenRoundtrip) {
    ASSERT_NE(renderer, nullptr);
    renderer->setZoomScale(1.5f);
    renderer->setContentOffset(tgfx::Point::Make(100.f, 200.f));

    auto screenLoc = tgfx::Point::Make(300.f, 400.f);
    auto original = renderer->convertScreenToOriginal(screenLoc);
    auto back = renderer->convertOriginalToScreen(original);

    EXPECT_NEAR(back.x, screenLoc.x, 0.1f);
    EXPECT_NEAR(back.y, screenLoc.y, 0.1f);
}

TEST_F(SeatCanvasTestFixture, ConvertNormalizedToOriginalOrigin) {
    ASSERT_NE(renderer, nullptr);
    auto result = renderer->convertNormalizedContentToOriginal(tgfx::Point::Make(0.f, 0.f));
    EXPECT_FLOAT_EQ(result.x, 0.f);
    EXPECT_FLOAT_EQ(result.y, 0.f);
}

TEST_F(SeatCanvasTestFixture, ConvertOriginalToNormalizedOrigin) {
    ASSERT_NE(renderer, nullptr);
    auto result = renderer->convertOriginalToNormalizedContent(tgfx::Point::Make(0.f, 0.f));
    EXPECT_FLOAT_EQ(result.x, 0.f);
    EXPECT_FLOAT_EQ(result.y, 0.f);
}

// ---------- zoom & offset accessors ----------

TEST_F(SeatCanvasTestFixture, DefaultZoomScale) {
    ASSERT_NE(renderer, nullptr);
    EXPECT_GT(renderer->getZoomScale(), 0.0f);
}

TEST_F(SeatCanvasTestFixture, ZoomScaleSetAndGetAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto minZoom = renderer->getMinimumZoomScale();
    auto maxZoom = renderer->getMaximumZoomScale();
    float midZoom = (minZoom + maxZoom) / 2.0f;
    renderer->setZoomScale(midZoom);
    EXPECT_FLOAT_EQ(renderer->getZoomScale(), midZoom);
}

TEST_F(SeatCanvasTestFixture, ZoomScaleClampedToMinimum) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto minZoom = renderer->getMinimumZoomScale();
    ASSERT_GT(minZoom, 0.0f);
    // Setting well below minimum should clamp
    renderer->setZoomScale(minZoom * 0.5f);
    EXPECT_NEAR(renderer->getZoomScale(), minZoom, 0.01f);
}

TEST_F(SeatCanvasTestFixture, DefaultContentOffset) {
    ASSERT_NE(renderer, nullptr);
    auto offset = renderer->getContentOffset();
    // Initial offset is centered in viewport (not necessarily (0,0))
    EXPECT_GE(offset.x, 0.f);
    EXPECT_GE(offset.y, 0.f);
}

TEST_F(SeatCanvasTestFixture, ContentOffsetChangeAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // Content offset should be finite and valid
    auto offset = renderer->getContentOffset();
    EXPECT_TRUE(std::isfinite(offset.x));
    EXPECT_TRUE(std::isfinite(offset.y));

    // Changing offset should not crash
    renderer->setContentOffset(tgfx::Point::Make(-50.f, -50.f));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, MinMaxZoomScaleAreSet) {
    ASSERT_NE(renderer, nullptr);
    // After construction, min/max should be set (not 0)
    EXPECT_GE(renderer->getMaximumZoomScale(), renderer->getMinimumZoomScale());
}

// ---------- isPointInContentArea ----------

TEST_F(SeatCanvasTestFixture, IsPointInContentAreaAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // With origin (100, 100) in normalized content and default zoom/offset,
    // the content rect should be non-empty
    auto rect = renderer->getVisibleOriginalRect();
    ASSERT_FALSE(rect.isEmpty());

    // A point at the center of the viewport should be in content area
    auto center = tgfx::Point::Make(375.f, 667.f);  // half of 750x1334
    EXPECT_TRUE(renderer->isPointInContentArea(center));
}

TEST_F(SeatCanvasTestFixture, IsPointInContentAreaFarOutside) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // A point far outside the viewport should not be in content area
    auto farOut = tgfx::Point::Make(5000.f, 5000.f);
    EXPECT_FALSE(renderer->isPointInContentArea(farOut));
}

// ---------- getVisibleOriginalRect & getZoneIdsInOriginalRect ----------

TEST_F(SeatCanvasTestFixture, GetVisibleOriginalRectAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto rect = renderer->getVisibleOriginalRect();
    EXPECT_FALSE(rect.isEmpty());
    EXPECT_GT(rect.width(), 0.f);
    EXPECT_GT(rect.height(), 0.f);
}

TEST_F(SeatCanvasTestFixture, GetZoneIdsInVisibleRect) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto visibleRect = renderer->getVisibleOriginalRect();
    ASSERT_FALSE(visibleRect.isEmpty());

    auto zoneIds = renderer->getZoneIdsInOriginalRect(visibleRect);
    EXPECT_GT(zoneIds.size(), 0u);
}

TEST_F(SeatCanvasTestFixture, GetZoneIdsInEmptyRect) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto zoneIds = renderer->getZoneIdsInOriginalRect(tgfx::Rect::MakeEmpty());
    EXPECT_EQ(zoneIds.size(), 0u);
}

TEST_F(SeatCanvasTestFixture, GetZoneIdsBeforeLoad) {
    ASSERT_NE(renderer, nullptr);
    // No basemap loaded — should return empty
    auto zoneIds = renderer->getZoneIdsInOriginalRect(tgfx::Rect::MakeXYWH(0, 0, 100, 100));
    EXPECT_EQ(zoneIds.size(), 0u);
}

// ---------- zoomLevelConfig & venue helpers ----------

TEST_F(SeatCanvasTestFixture, ZoomLevelConfigAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    const auto &config = renderer->zoomLevelConfig();
    // performbg.svg has venue ~2.233, so it's not small
    EXPECT_GT(config.venue, 0.0f);
    EXPECT_FALSE(renderer->isSmallVenue());
}

TEST_F(SeatCanvasTestFixture, SmallVenueWithPerformbg2) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg_2.svg"));

    // performbg_2.svg has venue < 1.0, so it's a small venue
    EXPECT_TRUE(renderer->isSmallVenue());
}

TEST_F(SeatCanvasTestFixture, ShowBackZoomThresholdAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto threshold = renderer->showBackZoomThreshold();
    // For non-small venue, showBackZoomThreshold returns venue
    EXPECT_GT(threshold, 0.0f);
}

TEST_F(SeatCanvasTestFixture, ShowBackZoomThresholdSmallVenue) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg_2.svg"));

    // For small venue, showBackZoomThreshold returns zone (not venue)
    auto threshold = renderer->showBackZoomThreshold();
    EXPECT_GT(threshold, 0.0f);
    EXPECT_LT(threshold, renderer->zoomLevelConfig().venue);
}

// ---------- pricecodeIndexForCode ----------

TEST_F(SeatCanvasTestFixture, PricecodeIndexForCodeKnown) {
    ASSERT_NE(renderer, nullptr);
    renderer->registerPricecodes({"A", "B", "C"});

    EXPECT_EQ(renderer->pricecodeIndexForCode("A"), 0u);
    EXPECT_EQ(renderer->pricecodeIndexForCode("B"), 1u);
    EXPECT_EQ(renderer->pricecodeIndexForCode("C"), 2u);
}

TEST_F(SeatCanvasTestFixture, PricecodeIndexForCodeUnknown) {
    ASSERT_NE(renderer, nullptr);
    renderer->registerPricecodes({"A", "B"});

    // Unknown pricecode returns kNoPricecodeIndex
    EXPECT_EQ(renderer->pricecodeIndexForCode("Z"), kk::kNoPricecodeIndex);
}

TEST_F(SeatCanvasTestFixture, PricecodeIndexForCodeEmpty) {
    ASSERT_NE(renderer, nullptr);
    // No pricecodes registered
    EXPECT_EQ(renderer->pricecodeIndexForCode("A"), kk::kNoPricecodeIndex);
}

TEST_F(SeatCanvasTestFixture, PricecodeIndexForCodeEmptyString) {
    ASSERT_NE(renderer, nullptr);
    renderer->registerPricecodes({"A", "B"});
    EXPECT_EQ(renderer->pricecodeIndexForCode(""), kk::kNoPricecodeIndex);
}

// ---------- clearSeatData ----------

TEST_F(SeatCanvasTestFixture, ClearSeatDataWhenEmpty) {
    ASSERT_NE(renderer, nullptr);
    // Clearing when no data is set should not crash
    renderer->clearSeatData();
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, ClearSeatDataAfterSet) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {
        {"s1", 10.f, 20.f, 0.f, 0},
        {"s2", 30.f, 40.f, 0.f, 0},
    };
    renderer->setSeatData("zone_a", seats);

    // Clear should not crash
    renderer->clearSeatData();
    SUCCEED();
}

// ---------- updateSeatStatuses (bulk, not ForZone) ----------

TEST_F(SeatCanvasTestFixture, UpdateSeatStatusesBulk) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {
        {"s1", 10.f, 20.f, 0.f, 0},
        {"s2", 30.f, 40.f, 0.f, 0},
        {"s3", 50.f, 60.f, 0.f, 0},
    };
    renderer->setSeatData("zone_a", seats);

    // Bulk update using SeatStatusUpdate
    std::vector<kk::SeatStatusUpdate> updates = {
        {"s1", 1},
        {"s2", 2},
    };
    renderer->updateSeatStatuses(updates);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, UpdateSeatStatusesBulkEmpty) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {{"s1", 10.f, 20.f, 0.f, 0}};
    renderer->setSeatData("zone_a", seats);

    // Empty updates should not crash
    std::vector<kk::SeatStatusUpdate> updates = {};
    renderer->updateSeatStatuses(updates);
    SUCCEED();
}

// ---------- updateSelectedSeatIds (incremental) ----------

TEST_F(SeatCanvasTestFixture, UpdateSelectedSeatIdsIncremental) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {
        {"s1", 10.f, 20.f, 0.f, 0},
        {"s2", 30.f, 40.f, 0.f, 0},
        {"s3", 50.f, 60.f, 0.f, 0},
    };
    renderer->setSeatData("zone_a", seats);

    // Add selection
    renderer->updateSelectedSeatIds({"s1", "s2"}, {});
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, UpdateSelectedSeatIdsAddAndRemove) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {
        {"s1", 10.f, 20.f, 0.f, 0},
        {"s2", 30.f, 40.f, 0.f, 0},
    };
    renderer->setSeatData("zone_a", seats);

    // Full set then incremental remove
    renderer->setSelectedSeatIds({"s1", "s2"});
    renderer->updateSelectedSeatIds({}, {"s1"});
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, UpdateSelectedSeatIdsEmpty) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {{"s1", 10.f, 20.f, 0.f, 0}};
    renderer->setSeatData("zone_a", seats);

    // Empty add and remove should not crash
    renderer->updateSelectedSeatIds({}, {});
    SUCCEED();
}

// ---------- setStyleKeyToConfigFromJSON ----------

TEST_F(SeatCanvasTestFixture, SetStyleKeyToConfigFromJSONValid) {
    ASSERT_NE(renderer, nullptr);

    const char *json = R"({
        "A_0_0": {"type": "circle", "fillColor": "#FF0000"},
        "A_1_0": {"type": "circle", "fillColor": "#00FF00"}
    })";
    renderer->setStyleKeyToConfigFromJSON(json, strlen(json));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, SetStyleKeyToConfigFromJSONEmpty) {
    ASSERT_NE(renderer, nullptr);
    const char *json = "{}";
    renderer->setStyleKeyToConfigFromJSON(json, strlen(json));
    SUCCEED();
}

// ---------- zoomToRect non-animated ----------

TEST_F(SeatCanvasTestFixture, ZoomToRectNonAnimated) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto rect = tgfx::Rect::MakeXYWH(0.f, 0.f, 100.f, 100.f);
    // Non-animated zoom should not crash
    renderer->zoomToRect(rect, false, 10.0f, 300.0);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, ZoomToRectNonAnimatedEmpty) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    auto rect = tgfx::Rect::MakeEmpty();
    renderer->zoomToRect(rect, false, 0.0f, 0.0);
    SUCCEED();
}

// ---------- replacePlatformView ----------

TEST_F(SeatCanvasTestFixture, ReplacePlatformViewWithNull) {
    ASSERT_NE(renderer, nullptr);
    // Replace with null — should not crash (moves null into _platformView)
    renderer->replacePlatformView(nullptr);
    SUCCEED();
}

// ---------- gesture handlers ----------

TEST_F(SeatCanvasTestFixture, HandleTapWithoutBaseMap) {
    ASSERT_NE(renderer, nullptr);
    // Tap without a basemap loaded should not crash
    renderer->handleTap(tgfx::Point::Make(200.f, 300.f));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandleTapAfterLoad) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // Tap on the center of the viewport
    renderer->handleTap(tgfx::Point::Make(375.f, 667.f));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePanBeganWithoutBaseMap) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePan(kk::gesture::GestureState::BEGAN,
                        tgfx::Point::Make(10.f, 0.f), 1000.0);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePanChanged) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePan(kk::gesture::GestureState::CHANGED,
                        tgfx::Point::Make(10.f, 20.f), 1000.0);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePanEnded) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePan(kk::gesture::GestureState::ENDED,
                        tgfx::Point::Make(0.f, 0.f), 1000.0);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePanCancelled) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePan(kk::gesture::GestureState::CANCELLED,
                        tgfx::Point::Make(0.f, 0.f), 1000.0);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePinchBegan) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePinch(kk::gesture::GestureState::BEGAN,
                          1.5f, tgfx::Point::Make(375.f, 667.f));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePinchChanged) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePinch(kk::gesture::GestureState::CHANGED,
                          1.2f, tgfx::Point::Make(375.f, 667.f));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePinchEnded) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePinch(kk::gesture::GestureState::ENDED,
                          2.0f, tgfx::Point::Make(200.f, 300.f));
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, HandlePinchCancelled) {
    ASSERT_NE(renderer, nullptr);
    renderer->handlePinch(kk::gesture::GestureState::CANCELLED,
                          1.0f, tgfx::Point::Make(200.f, 300.f));
    SUCCEED();
}

// ---------- updateMiniMapZoneAlternateColors ----------

TEST_F(SeatCanvasTestFixture, UpdateMiniMapZoneColorsWithEmptyMap) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // Empty color map should not crash
    std::unordered_map<std::string, tgfx::Color> empty = {};
    renderer->updateMiniMapZoneAlternateColors(empty);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, UpdateMiniMapZoneColorsWithData) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    std::unordered_map<std::string, tgfx::Color> colors = {
        {"10001", tgfx::Color::Red()},
        {"20001", tgfx::Color::Blue()},
    };
    renderer->updateMiniMapZoneAlternateColors(colors);
    SUCCEED();
}

// ---------- invalidateContent ----------

TEST_F(SeatCanvasTestFixture, InvalidateContentDoesNotCrash) {
    ASSERT_NE(renderer, nullptr);
    renderer->invalidateContent();
    SUCCEED();
}

// ---------- stop without start ----------

TEST_F(SeatCanvasTestFixture, StopWithoutStartDoesNotCrash) {
    ASSERT_NE(renderer, nullptr);
    renderer->stop();
    SUCCEED();
}

// ---------- setBaseMapConfig with null ----------

TEST_F(SeatCanvasTestFixture, SetBaseMapConfigNullClears) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // Set null should clear the basemap
    renderer->setBaseMapConfig(nullptr);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, SetBaseMapConfigNullBeforeLoad) {
    ASSERT_NE(renderer, nullptr);
    renderer->setBaseMapConfig(nullptr);
    SUCCEED();
}

// ---------- updateSeatStatusesForZone (edge cases) ----------

TEST_F(SeatCanvasTestFixture, UpdateSeatStatusesForZoneUnknownZone) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    // Update statuses for a zone that has no seat data
    uint32_t statuses[] = {1, 2};
    renderer->updateSeatStatusesForZone("nonexistent_zone", statuses, 2);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, SetSeatDataForZoneThenUpdateStatus) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->registerPricecodes({"P1"});
    std::vector<kk::SeatData> seats = {
        {"s1", 10.f, 20.f, 0.f, 0},
        {"s2", 30.f, 40.f, 0.f, 0},
        {"s3", 50.f, 60.f, 0.f, 0},
    };
    renderer->setSeatData("zone_x", seats);

    uint32_t statuses[] = {0, 1, 2};
    renderer->updateSeatStatusesForZone("zone_x", statuses, 3);
    SUCCEED();
}

// ---------- setSeatData edge cases ----------

TEST_F(SeatCanvasTestFixture, SetSeatDataEmptySeats) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    std::vector<kk::SeatData> empty = {};
    renderer->setSeatData("zone_x", empty);
    SUCCEED();
}

// ---------- setMaxWidth ----------

TEST_F(SeatCanvasTestFixture, SetMaxWidthLargerThanViewport) {
    ASSERT_NE(renderer, nullptr);
    renderer->setMaxWidth(2000.f);
    EXPECT_FLOAT_EQ(renderer->getMaxWidth(), 2000.f);
}

TEST_F(SeatCanvasTestFixture, SetMaxWidthZero) {
    ASSERT_NE(renderer, nullptr);
    renderer->setMaxWidth(0.f);
    EXPECT_FLOAT_EQ(renderer->getMaxWidth(), 0.f);
}

// ---------- lifecycle: start/stop ----------

TEST_F(SeatCanvasTestFixture, StartStopCycle) {
    ASSERT_NE(renderer, nullptr);
    renderer->start();
    renderer->stop();
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, DoubleStop) {
    ASSERT_NE(renderer, nullptr);
    renderer->start();
    renderer->stop();
    renderer->stop();  // second stop should be safe
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, DoubleStart) {
    ASSERT_NE(renderer, nullptr);
    renderer->start();
    renderer->start();  // second start should be safe
    renderer->stop();
    SUCCEED();
}

// ---------- setStyleIdToConfig ----------

TEST_F(SeatCanvasTestFixture, SetStyleIdToConfigEmpty) {
    ASSERT_NE(renderer, nullptr);
    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> empty = {};
    renderer->setStyleIdToConfig(empty);
    SUCCEED();
}

TEST_F(SeatCanvasTestFixture, SetStyleIdToConfigWithData) {
    ASSERT_NE(renderer, nullptr);
    auto config = CircleSeatStyleConfig::Make(tgfx::Color::Red());
    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> map = {
        {"test_key", config},
    };
    renderer->setStyleIdToConfig(map);
    SUCCEED();
}

};  // namespace kk::test
