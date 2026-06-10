#include "SeatCanvasTestFixture.hpp"
#include "TestFileUtils.hpp"
#include "core/SeatData.hpp"
#include "core/style/CircleSeatStyleConfig.hpp"
#include "core/style/SeatRenderStyleKey.hpp"

#include <nlohmann/json.hpp>
#include <set>

namespace kk::test {

// Parse seat data JSON. Returns (zoneId→seats, unique pricecode strings).
static std::pair<std::unordered_map<std::string, std::vector<kk::SeatData>>, std::vector<std::string>>
loadSeatData(const std::string &resourcePath) {

    std::unordered_map<std::string, std::vector<kk::SeatData>> result;
    std::set<std::string> pricecodeSet;

    auto raw = ReadFile(resourcePath);
    if (!raw) {
        return {result, {}};
    }
    std::string jsonStr(reinterpret_cast<const char *>(raw->data()), raw->size());
    auto json = nlohmann::json::parse(jsonStr, nullptr, false);
    if (json.is_discarded() || !json.is_object()) {
        return {result, {}};
    }

    for (auto &[zoneId, seats] : json.items()) {
        std::vector<kk::SeatData> seatVec;
        seatVec.reserve(seats.size());
        for (auto &seat : seats) {
            std::string seatId = seat.value("seatId", "");
            if (seatId.empty()) {
                continue;
            }
            kk::SeatData data;
            data.seatId = seatId;
            data.x = seat.value("x", 0.f);
            data.y = seat.value("y", 0.f);
            data.rotation = seat.value("rotation", 0.f);
            data.pricecodeIndex = kk::kNoPricecodeIndex;  // resolved later
            seatVec.push_back(data);

            // Collect pricecode string
            std::string pc = seat.value("pricecode", "");
            if (!pc.empty()) {
                pricecodeSet.insert(pc);
            }
        }
        if (!seatVec.empty()) {
            result[zoneId] = std::move(seatVec);
        }
    }

    return {result, std::vector<std::string>(pricecodeSet.begin(), pricecodeSet.end())};
}

// Resolve pricecode strings → indices using the pricecode order.
static void resolvePricecodeIndices(
    std::unordered_map<std::string, std::vector<kk::SeatData>> &seatDataMap,
    const std::string &resourcePath,
    const std::vector<std::string> &pricecodes) {

    // Build string→index map
    std::unordered_map<std::string, uint16_t> pcToIndex;
    for (size_t i = 0; i < pricecodes.size(); ++i) {
        pcToIndex[pricecodes[i]] = static_cast<uint16_t>(i);
    }

    // Re-read JSON to get pricecode per seat (avoid carrying extra state)
    auto raw = ReadFile(resourcePath);
    if (!raw)
        return;
    std::string jsonStr(reinterpret_cast<const char *>(raw->data()), raw->size());
    auto json = nlohmann::json::parse(jsonStr, nullptr, false);
    if (json.is_discarded() || !json.is_object())
        return;

    for (auto &[zoneId, seats] : json.items()) {
        auto it = seatDataMap.find(zoneId);
        if (it == seatDataMap.end())
            continue;
        auto &seatVec = it->second;
        size_t idx = 0;
        for (auto &seat : seats) {
            if (idx >= seatVec.size())
                break;
            std::string pc = seat.value("pricecode", "");
            auto pcIt = pcToIndex.find(pc);
            if (pcIt != pcToIndex.end()) {
                seatVec[idx].pricecodeIndex = pcIt->second;
            }
            ++idx;
        }
    }
}

static std::unordered_map<std::string, std::shared_ptr<kk::renderer::SeatStyleConfig>>
buildDefaultStyleConfigs(const std::vector<std::string> &pricecodes) {

    using namespace kk::renderer;

    auto available = CircleSeatStyleConfig::Make(tgfx::Color::FromRGBA(0xAA, 0xAA, 0xAA, 0xFF));
    auto sold = CircleSeatStyleConfig::Make(tgfx::Color::FromRGBA(0xDD, 0x44, 0x44, 0xFF));
    auto selected = CircleSeatStyleConfig::Make(tgfx::Color::FromRGBA(0x44, 0xAA, 0x44, 0xFF));
    auto reserved = CircleSeatStyleConfig::Make(tgfx::Color::FromRGBA(0x44, 0x88, 0xCC, 0xFF));

    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> configs;
    for (const auto &pc : pricecodes) {
        configs[composeSeatStyleId(pc, 0, false)] = available;
        configs[composeSeatStyleId(pc, 1, false)] = sold;
        configs[composeSeatStyleId(pc, 0, true)] = selected;
        configs[composeSeatStyleId(pc, 2, false)] = reserved;
    }
    return configs;
}

TEST_F(SeatCanvasTestFixture, SeatRenderingAtSeatLevel) {
    ASSERT_NE(renderer, nullptr);

    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    const char *seatDataPath = "resources/SeatCanvasSample.bundle/default/seatdata/performbg.json";
    auto [seatDataMap, pricecodes] = loadSeatData(seatDataPath);
    ASSERT_GT(seatDataMap.size(), 0u);
    ASSERT_GT(pricecodes.size(), 0u);

    renderer->setSeatSize(24.0f);
    renderer->registerPricecodes(pricecodes);
    resolvePricecodeIndices(seatDataMap, seatDataPath, pricecodes);

    auto styleConfigs = buildDefaultStyleConfigs(pricecodes);
    renderer->setStyleIdToConfig(styleConfigs);

    for (auto &[zoneId, seats] : seatDataMap) {
        renderer->setSeatData(zoneId, seats);
        std::vector<uint32_t> statuses(seats.size(), 0);
        for (size_t i = 0; i < statuses.size(); ++i) {
            statuses[i] = static_cast<uint32_t>(i % 3);
        }
        renderer->updateSeatStatusesForZone(zoneId, statuses.data(), statuses.size());
    }

    std::vector<std::string> selectedIds;
    for (auto &[zoneId, seats] : seatDataMap) {
        if (selectedIds.size() >= 20)
            break;
        for (size_t i = 0; i < std::min(size_t(3), seats.size()) && selectedIds.size() < 20; ++i) {
            selectedIds.push_back(seats[i].seatId);
        }
    }
    renderer->setSelectedSeatIds(selectedIds);

    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.seat);

    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/SeatRendering"));
}

TEST_F(SeatCanvasTestFixture, SeatRenderingAtRowLevel) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    const char *seatDataPath = "resources/SeatCanvasSample.bundle/default/seatdata/performbg.json";
    auto [seatDataMap, pricecodes] = loadSeatData(seatDataPath);
    ASSERT_GT(seatDataMap.size(), 0u);

    renderer->setSeatSize(24.0f);
    renderer->registerPricecodes(pricecodes);
    resolvePricecodeIndices(seatDataMap, seatDataPath, pricecodes);
    renderer->setStyleIdToConfig(buildDefaultStyleConfigs(pricecodes));

    for (auto &[zoneId, seats] : seatDataMap) {
        renderer->setSeatData(zoneId, seats);
        std::vector<uint32_t> statuses(seats.size(), 0);
        for (size_t i = 0; i < statuses.size(); ++i) {
            statuses[i] = static_cast<uint32_t>((i / 5) % 3);
        }
        renderer->updateSeatStatusesForZone(zoneId, statuses.data(), statuses.size());
    }

    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.row);

    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/SeatRenderingRow"));
}

TEST_F(SeatCanvasTestFixture, SeatRenderingAtVenueLevel) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    const char *seatDataPath = "resources/SeatCanvasSample.bundle/default/seatdata/performbg.json";
    auto [seatDataMap, pricecodes] = loadSeatData(seatDataPath);
    ASSERT_GT(seatDataMap.size(), 0u);

    renderer->setSeatSize(24.0f);
    renderer->registerPricecodes(pricecodes);
    resolvePricecodeIndices(seatDataMap, seatDataPath, pricecodes);
    renderer->setStyleIdToConfig(buildDefaultStyleConfigs(pricecodes));

    for (auto &[zoneId, seats] : seatDataMap) {
        renderer->setSeatData(zoneId, seats);
        std::vector<uint32_t> statuses(seats.size(), 0);
        for (size_t i = 0; i < statuses.size(); ++i) {
            statuses[i] = static_cast<uint32_t>(i % 3);
        }
        renderer->updateSeatStatusesForZone(zoneId, statuses.data(), statuses.size());
    }

    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.venue);

    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/SeatRenderingVenue"));
}

TEST_F(SeatCanvasTestFixture, MultiZoneColorChange) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    renderer->updateSeatZoneAlternateColors({
        {"10001", tgfx::Color::Red()},
        {"10002", tgfx::Color::FromRGBA(0x00, 0x80, 0x00, 0xFF)},
        {"20001", tgfx::Color::FromRGBA(0x00, 0x00, 0xFF, 0xFF)},
        {"20002", tgfx::Color::FromRGBA(0xFF, 0x80, 0x00, 0xFF)},
    });

    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/MultiZoneColor"));
}

TEST_F(SeatCanvasTestFixture, PanAndZoomCombined) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));

    const auto &zoomLevel = renderer->zoomLevelConfig();
    renderer->setZoomScale(zoomLevel.zone);
    renderer->setContentOffset(tgfx::Point::Make(-150.f, -100.f));

    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/PanZoomCombined"));
}

TEST_F(SeatCanvasTestFixture, ZoneColorOnSingleZone) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg.svg"));
    renderer->updateSeatZoneAlternateColors({{"30001", tgfx::Color::Red()}});
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/SingleZoneColor"));
}

TEST_F(SeatCanvasTestFixture, ZoneColorOnPerformbg3) {
    ASSERT_NE(renderer, nullptr);
    ASSERT_TRUE(loadSVGBaseMap(*renderer, "resources/SeatCanvasSample.bundle/default/basemap/performbg_3.svg"));
    renderer->updateSeatZoneAlternateColors({{"inner_105889", tgfx::Color::FromRGBA(0x00, 0x80, 0xFF, 0xFF)}});
    renderFrame(*renderer);
    EXPECT_TRUE(compareBaseline("BaseMap/Performbg3ZoneColor"));
}

};  // namespace kk::test
