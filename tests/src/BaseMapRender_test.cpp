#include "SeatCanvasTestFixture.hpp"

namespace kk::test {

TEST_F(SeatCanvasTestFixture, 73807) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/73807.svg"));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/73807"));
}

TEST_F(SeatCanvasTestFixture, 73808) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/73808.svg"));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/73808"));
}

TEST_F(SeatCanvasTestFixture, ZoneLevel) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    const auto &zoomLevel = renderer->zoomLevelConfig();
    EXPECT_FLOAT_EQ(zoomLevel.venue, 2.233333f);
    EXPECT_FLOAT_EQ(zoomLevel.zone, 3.722222f);
    EXPECT_FLOAT_EQ(zoomLevel.row, 6.203704f);
    EXPECT_FLOAT_EQ(zoomLevel.seat, 12.407408f);
}

TEST_F(SeatCanvasTestFixture, Initialize) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/Initialize"));
}

TEST_F(SeatCanvasTestFixture, ChangeZoneColor) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->updateSeatZoneAlternateColors({{"20024", tgfx::Color::Red()}});
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/ChangeZoneColor"));
}

TEST_F(SeatCanvasTestFixture, ZoneScale) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.zone);
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/ZoneScale"));
}

TEST_F(SeatCanvasTestFixture, RowScale) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.row);
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/RowScale"));
}

TEST_F(SeatCanvasTestFixture, SeatScale) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.seat);
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/SeatScale"));
}

TEST_F(SeatCanvasTestFixture, MaximumZoomScale) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->setZoomScale(renderer->getMaximumZoomScale());
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/MaximumZoomScale"));
}

TEST_F(SeatCanvasTestFixture, Performbg2Basemap) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg_2.svg"));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/Performbg2"));
}

TEST_F(SeatCanvasTestFixture, Performbg3Basemap) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg_3.svg"));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/Performbg3"));
}

TEST_F(SeatCanvasTestFixture, LandscapeViewport) {
    auto landscape = makeRenderer(1334, 750, 2.0f);
    ASSERT_NE(landscape, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*landscape, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    landscape->setBackgroundColor(tgfx::Color::White());
    renderFrame(*landscape);
    EXPECT_TRUE(compareBaseline("BaseMap/Landscape"));
}

TEST_F(SeatCanvasTestFixture, StandardDensity) {
    auto loDPI = makeRenderer(750, 1334, 1.0f);
    ASSERT_NE(loDPI, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*loDPI, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    loDPI->setBackgroundColor(tgfx::Color::White());
    renderFrame(*loDPI);
    EXPECT_TRUE(compareBaseline("BaseMap/StandardDensity"));
}

TEST_F(SeatCanvasTestFixture, PannedRight) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->setContentOffset(tgfx::Point::Make(-200.f, 0.f));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/PannedRight"));
}

TEST_F(SeatCanvasTestFixture, PannedDown) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->setContentOffset(tgfx::Point::Make(0.f, -200.f));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/PannedDown"));
}

TEST_F(SeatCanvasTestFixture, MinimumZoomScale) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->setZoomScale(renderer->getMinimumZoomScale());
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/MinimumZoomScale"));
}

TEST_F(SeatCanvasTestFixture, DarkBackground) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->setBackgroundColor(tgfx::Color::Black());
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/DarkBackground"));
}

};  // namespace kk::test
