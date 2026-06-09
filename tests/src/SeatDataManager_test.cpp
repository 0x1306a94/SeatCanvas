#include "core/renderer/SeatDataManager.hpp"

#include <gtest/gtest.h>

using namespace kk;
using namespace kk::renderer;

class SeatDataManagerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        manager = new SeatDataManager(nullptr);
    }

    void TearDown() override {
        delete manager;
    }

    SeatDataManager *manager = nullptr;
};

TEST_F(SeatDataManagerTest, SetSeatDataRejectsEmptyZone) {
    std::vector<SeatData> seats = {SeatData{"S1", 100.f, 200.f}};
    EXPECT_FALSE(manager->setSeatData("", seats));
}

TEST_F(SeatDataManagerTest, SetSeatDataSkipsInvalidSeats) {
    std::vector<SeatData> seats = {
        SeatData{"S1", 100.f, 200.f},
        SeatData{"", 0.f, 0.f},
        SeatData{"S2", 300.f, 400.f},
    };
    EXPECT_TRUE(manager->setSeatData("zone1", seats));

    auto *data = manager->getSeatDataForZone("zone1");
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->size(), 2u);
    EXPECT_EQ((*data)[0].seatId, "S1");
    EXPECT_EQ((*data)[1].seatId, "S2");
}

TEST_F(SeatDataManagerTest, SetSeatDataReplacesExistingZone) {
    std::vector<SeatData> seats1 = {SeatData{"S1", 0.f, 0.f}};
    std::vector<SeatData> seats2 = {SeatData{"S2", 10.f, 10.f}};

    EXPECT_TRUE(manager->setSeatData("zone1", seats1));
    EXPECT_TRUE(manager->setSeatData("zone1", seats2));

    auto *data = manager->getSeatDataForZone("zone1");
    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->size(), 1u);
    EXPECT_EQ((*data)[0].seatId, "S2");
}

TEST_F(SeatDataManagerTest, GetSeatDataForZoneNotFound) {
    EXPECT_EQ(manager->getSeatDataForZone("nonexistent"), nullptr);
}

TEST_F(SeatDataManagerTest, ZoneAndSeatCount) {
    EXPECT_EQ(manager->getZoneCount(), 0u);
    EXPECT_EQ(manager->getTotalSeatCount(), 0u);

    std::vector<SeatData> seats1 = {SeatData{"A", 0.f, 0.f}, SeatData{"B", 0.f, 0.f}};
    std::vector<SeatData> seats2 = {SeatData{"C", 0.f, 0.f}};
    manager->setSeatData("z1", seats1);
    manager->setSeatData("z2", seats2);

    EXPECT_EQ(manager->getZoneCount(), 2u);
    EXPECT_EQ(manager->getTotalSeatCount(), 3u);
}

TEST_F(SeatDataManagerTest, ClearSeatDataEmpty) {
    EXPECT_FALSE(manager->clearSeatData());
}

TEST_F(SeatDataManagerTest, ClearSeatData) {
    manager->setSeatData("zone1", {SeatData{"S1", 0.f, 0.f}});
    EXPECT_TRUE(manager->clearSeatData());
    EXPECT_EQ(manager->getZoneCount(), 0u);
    EXPECT_EQ(manager->getTotalSeatCount(), 0u);
}

TEST_F(SeatDataManagerTest, PricecodeIndexForCodeUnregistered) {
    // Without registerPricecodes(), all codes return kNoPricecodeIndex
    EXPECT_EQ(manager->pricecodeIndexForCode("A"), kk::kNoPricecodeIndex);
    EXPECT_EQ(manager->pricecodeIndexForCode(""), kk::kNoPricecodeIndex);
}

TEST_F(SeatDataManagerTest, UpdateSeatStatuses) {
    manager->setSeatData("zone1", {SeatData{"S1", 0.f, 0.f}, SeatData{"S2", 0.f, 0.f}});

    std::vector<SeatStatusUpdate> updates = {
        {"S1", 5},
        {"S2", 3},
    };
    EXPECT_TRUE(manager->updateSeatStatuses(updates));
}

TEST_F(SeatDataManagerTest, UpdateSeatStatusesEmptyUpdates) {
    EXPECT_FALSE(manager->updateSeatStatuses({}));
}

TEST_F(SeatDataManagerTest, UpdateSeatStatusesUnknownSeat) {
    std::vector<SeatStatusUpdate> updates = {{"unknown", 1}};
    EXPECT_FALSE(manager->updateSeatStatuses(updates));
}

TEST_F(SeatDataManagerTest, UpdateSeatStatusesSkipsEmptySeatId) {
    manager->setSeatData("zone1", {SeatData{"S1", 0.f, 0.f}});
    std::vector<SeatStatusUpdate> updates = {{"", 1}};
    EXPECT_FALSE(manager->updateSeatStatuses(updates));
}

TEST_F(SeatDataManagerTest, UpdateSeatStatusesForZone) {
    manager->setSeatData("zone1", {SeatData{"A", 0.f, 0.f}, SeatData{"B", 0.f, 0.f}});

    std::vector<uint32_t> statuses = {10, 20};
    EXPECT_TRUE(manager->updateSeatStatusesForZone("zone1", statuses.data(), statuses.size()));
}

TEST_F(SeatDataManagerTest, UpdateSeatStatusesForZoneRejectsMismatchedCount) {
    manager->setSeatData("zone1", {SeatData{"A", 0.f, 0.f}});
    std::vector<uint32_t> statuses = {1, 2};
    EXPECT_FALSE(manager->updateSeatStatusesForZone("zone1", statuses.data(), statuses.size()));
}

TEST_F(SeatDataManagerTest, UpdateSeatStatusesForZoneInvalidArgs) {
    EXPECT_FALSE(manager->updateSeatStatusesForZone("", nullptr, 1));
    EXPECT_FALSE(manager->updateSeatStatusesForZone("zone1", nullptr, 0));

    uint32_t s = 1;
    EXPECT_FALSE(manager->updateSeatStatusesForZone("zone1", &s, 0));
    EXPECT_FALSE(manager->updateSeatStatusesForZone("unknown", &s, 1));
}

TEST_F(SeatDataManagerTest, SetSelectedSeatIds) {
    manager->setSeatData("zone1", {SeatData{"S1", 0.f, 0.f}, SeatData{"S2", 0.f, 0.f}});
    manager->setSelectedSeatIds({"S1"});
    manager->setSelectedSeatIds({"S2"});
    manager->setSelectedSeatIds({});
}

TEST_F(SeatDataManagerTest, UpdateSelectedSeatIds) {
    manager->setSeatData("zone1", {SeatData{"S1", 0.f, 0.f}});

    EXPECT_TRUE(manager->updateSelectedSeatIds({"S1"}, {}));
    EXPECT_FALSE(manager->updateSelectedSeatIds({}, {}));
    EXPECT_TRUE(manager->updateSelectedSeatIds({}, {"S1"}));
}

TEST_F(SeatDataManagerTest, UpdateSelectedSeatIdsSkipsEmpty) {
    EXPECT_TRUE(manager->updateSelectedSeatIds({"valid", ""}, {}));
}
