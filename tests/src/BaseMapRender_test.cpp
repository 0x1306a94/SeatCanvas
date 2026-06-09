#include "SeatCanvasTestFixture.hpp"

namespace kk::test {

TEST_F(SeatCanvasTestFixture, ZoneLevel) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    const auto &zoomLevel = renderer->zoomLevelConfig();
    EXPECT_FLOAT_EQ(zoomLevel.venue, 2.233333f);
    EXPECT_FLOAT_EQ(zoomLevel.zone, 3.722222f);
    EXPECT_FLOAT_EQ(zoomLevel.row, 6.203704f);
    EXPECT_FLOAT_EQ(zoomLevel.seat, 12.407408f);
}

TEST_F(SeatCanvasTestFixture, PerformBgOverview) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/PerformBgOverview"));
}

};  // namespace kk::test
