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

};  // namespace kk::test
