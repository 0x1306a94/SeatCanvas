//
//  SeatDataManager.cpp
//  SeatCanvas
//
//  Created by king on 2026/05/23.
//

#include "SeatDataManager.hpp"

#include <cmath>

#include <nlohmann/json.hpp>

#include <tgfx/core/Rect.h>
#include <tgfx/platform/Print.h>

#include "core/renderer/BaseMapMeshBuilder.hpp"
#include "core/style/SeatRenderStyleKey.hpp"
#include "core/style/SeatStyleAtlasManager.hpp"
#include "core/style/SeatStyleConfigJSONHelper.hpp"

namespace kk::renderer {

SeatDataManager::SeatDataManager(SeatStyleAtlasManager *atlasManager)
    : _atlasManager(atlasManager) {
}

bool SeatDataManager::setSeatData(const std::string &zoneId, const std::vector<kk::SeatData> &seats) {
    if (zoneId.empty()) {
        return false;
    }

    if (auto oldIter = _seatDataMap.find(zoneId); oldIter != _seatDataMap.end()) {
        for (const auto &seat : oldIter->second) {
            _seatIndexById.erase(seat.seatId);
            _selectedSeatIds.erase(seat.seatId);
        }
    }

    std::vector<kk::SeatData> validSeats = {};
    validSeats.reserve(seats.size());
    for (const auto &seat : seats) {
        if (!seat.isValid()) {
            continue;
        }
        validSeats.push_back(seat);
    }

    _seatDataMap[zoneId] = std::move(validSeats);

    ZoneSeatRuntimeState runtimeState = {};
    runtimeState.statuses.assign(_seatDataMap[zoneId].size(), 0);
    _seatStateByZone[zoneId] = std::move(runtimeState);

    const auto &storedSeats = _seatDataMap[zoneId];
    for (size_t index = 0; index < storedSeats.size(); ++index) {
        _seatIndexById[storedSeats[index].seatId] = SeatLocation{zoneId, index};
    }

    return true;
}

bool SeatDataManager::clearSeatData() {
    if (_seatDataMap.empty() && _seatStateByZone.empty() && _seatIndexById.empty() && _selectedSeatIds.empty()) {
        return false;
    }
    _seatDataMap.clear();
    _seatStateByZone.clear();
    _seatIndexById.clear();
    _selectedSeatIds.clear();
    return true;
}

void SeatDataManager::registerPricecodes(const std::vector<std::string> &pricecodes) {
    _pricecodes = pricecodes;
    _pricecodeToIndex.clear();
    _pricecodeToIndex.reserve(pricecodes.size());
    for (size_t index = 0; index < pricecodes.size(); ++index) {
        if (pricecodes[index].empty()) {
            continue;
        }
        _pricecodeToIndex.emplace(pricecodes[index], static_cast<uint16_t>(index));
    }
    if (_atlasManager->getUVOffsetCount() > 0) {
        rebuildStyleKeyLookup();
    }
}

uint16_t SeatDataManager::pricecodeIndexForCode(const std::string &pricecode) const {
    return resolvePricecodeIndex(_pricecodeToIndex, pricecode);
}

bool SeatDataManager::updateSeatStatuses(const std::vector<kk::SeatStatusUpdate> &updates) {
    if (updates.empty()) {
        return false;
    }

    bool changed = false;
    for (const auto &update : updates) {
        if (update.seatId.empty()) {
            continue;
        }
        auto locationIter = _seatIndexById.find(update.seatId);
        if (locationIter == _seatIndexById.end()) {
            continue;
        }
        auto stateIter = _seatStateByZone.find(locationIter->second.zoneId);
        if (stateIter == _seatStateByZone.end()) {
            continue;
        }
        if (locationIter->second.index >= stateIter->second.statuses.size()) {
            continue;
        }
        stateIter->second.statuses[locationIter->second.index] = update.status;
        changed = true;
    }

    return changed;
}

bool SeatDataManager::updateSeatStatusesForZone(const std::string &zoneId, const uint32_t *statuses, size_t count) {
    if (zoneId.empty() || statuses == nullptr || count == 0) {
        return false;
    }

    auto dataIter = _seatDataMap.find(zoneId);
    auto stateIter = _seatStateByZone.find(zoneId);
    if (dataIter == _seatDataMap.end() || stateIter == _seatStateByZone.end()) {
        return false;
    }
    if (count != dataIter->second.size()) {
        return false;
    }

    stateIter->second.statuses.assign(statuses, statuses + count);
    return true;
}

void SeatDataManager::setSelectedSeatIds(const std::vector<std::string> &seatIds) {
    _selectedSeatIds.clear();
    _selectedSeatIds.insert(seatIds.begin(), seatIds.end());
}

bool SeatDataManager::updateSelectedSeatIds(const std::vector<std::string> &added, const std::vector<std::string> &removed) {
    if (added.empty() && removed.empty()) {
        return false;
    }

    for (const auto &seatId : removed) {
        _selectedSeatIds.erase(seatId);
    }
    for (const auto &seatId : added) {
        if (!seatId.empty()) {
            _selectedSeatIds.insert(seatId);
        }
    }
    return true;
}

bool SeatDataManager::setStyleIdToConfig(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig) {
    _registeredStyleIdToConfig = styleIdToConfig;
    auto changed = _atlasManager->setStyleIdToConfigs(styleIdToConfig);
    if (changed) {
        _uvIndexByStyleKey.clear();
    }
    return changed;
}

bool SeatDataManager::setStyleKeyToConfigFromJSON(const void *bytes, size_t len) {
    if (!bytes || len == 0) {
        return setStyleIdToConfig({});
    }

    std::string jsonString(reinterpret_cast<const char *>(bytes), len);

    if (!nlohmann::json::accept(jsonString)) {
        tgfx::PrintError("Invalid JSON format");
        return false;
    }

    auto json = nlohmann::json::parse(jsonString, nullptr, false);
    if (json.is_discarded()) {
        tgfx::PrintError("Failed to parse JSON");
        return false;
    }

    if (!json.is_array()) {
        tgfx::PrintError("Invalid JSON: expected array");
        return false;
    }

    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> styleIdToConfig = {};

    for (const auto &entry : json) {
        if (!entry.contains("key") || !entry.contains("config")) {
            tgfx::PrintError("Invalid JSON entry: missing 'key' or 'config'");
            continue;
        }

        if (!entry["key"].is_string()) {
            tgfx::PrintError("Invalid JSON entry: 'key' is not a string");
            continue;
        }

        auto styleId = entry["key"].get<std::string>();
        if (styleId.empty()) {
            tgfx::PrintError("Invalid JSON entry: styleId is empty");
            continue;
        }

        if (!entry["config"].is_object()) {
            tgfx::PrintError("Invalid JSON entry: 'config' is not an object");
            continue;
        }

        std::shared_ptr<SeatStyleConfig> config = nullptr;
        entry["config"].get_to(config);
        if (!config) {
            tgfx::PrintError("Failed to parse config");
            continue;
        }

        styleIdToConfig[styleId] = config;
    }

    return setStyleIdToConfig(styleIdToConfig);
}

void SeatDataManager::rebuildStyleKeyLookup() {
    _uvIndexByStyleKey.clear();
    if (_registeredStyleIdToConfig.empty()) {
        return;
    }

    for (const auto &entry : _registeredStyleIdToConfig) {
        auto parsed = parseSeatStyleId(entry.first);
        if (!parsed.has_value()) {
            continue;
        }

        uint16_t pricecodeIndex = kNoPricecodeIndex;
        if (!parsed->pricecode.empty()) {
            auto pricecodeIter = _pricecodeToIndex.find(parsed->pricecode);
            if (pricecodeIter == _pricecodeToIndex.end()) {
                continue;
            }
            pricecodeIndex = pricecodeIter->second;
        }

        auto uvIndex = _atlasManager->getUVOffsetIndex(entry.first);
        if (uvIndex < 0) {
            continue;
        }

        SeatRenderStyleKey styleKey{pricecodeIndex, parsed->status, parsed->selected};
        _uvIndexByStyleKey[styleKey] = uvIndex;
    }
}

SeatDataManager::CollectResult SeatDataManager::collectVisibleSeatInstances(
    const tgfx::Rect &visibleOriginalRect,
    BaseMapMeshBuilder *meshBuilder,
    float seatSize) const {

    CollectResult result = {};

    if (!meshBuilder || visibleOriginalRect.isEmpty()) {
        return result;
    }

    auto zones = meshBuilder->findZoneIntersectingRect(visibleOriginalRect);
    if (zones.empty()) {
        return result;
    }

    std::vector<SeatInstanceData> instances = {};
    size_t renderedZoneCount = 0;
    for (const auto &zone : zones) {
        if (!zone) {
            continue;
        }

        auto iter = _seatDataMap.find(zone->zoneId);
        if (iter == _seatDataMap.end()) {
            continue;
        }

        auto stateIter = _seatStateByZone.find(zone->zoneId);
        if (stateIter == _seatStateByZone.end()) {
            continue;
        }

        const auto &seats = iter->second;
        const auto &statuses = stateIter->second.statuses;
        auto partial = !visibleOriginalRect.contains(zone->fillBounds);
        const size_t instanceCountBefore = instances.size();
        for (size_t index = 0; index < seats.size(); ++index) {
            const auto &seat = seats[index];
            if (partial && !tgfx::Rect::Intersects(visibleOriginalRect, tgfx::Rect::MakeXYWH(seat.x, seat.y, seatSize, seatSize))) {
                continue;
            }

            uint32_t status = index < statuses.size() ? statuses[index] : 0;
            bool selected = _selectedSeatIds.find(seat.seatId) != _selectedSeatIds.end();
            SeatRenderStyleKey styleKey{seat.pricecodeIndex, status, selected};

            auto uvIter = _uvIndexByStyleKey.find(styleKey);
            if (uvIter == _uvIndexByStyleKey.end() || uvIter->second < 0) {
                continue;
            }

            float rotationRad = seat.rotation * (M_PI / 180.0f);
            instances.emplace_back(seat.x, seat.y, uvIter->second, rotationRad);
        }

        if (instances.size() > instanceCountBefore) {
            renderedZoneCount++;
        }
    }

    result.instances = std::move(instances);
    result.zoneCount = renderedZoneCount;
    return result;
}

const std::vector<kk::SeatData> *SeatDataManager::getSeatDataForZone(const std::string &zoneId) const {
    auto iter = _seatDataMap.find(zoneId);
    if (iter == _seatDataMap.end()) {
        return nullptr;
    }
    return &iter->second;
}

size_t SeatDataManager::getZoneCount() const {
    return _seatDataMap.size();
}

size_t SeatDataManager::getTotalSeatCount() const {
    size_t count = 0;
    for (const auto &entry : _seatDataMap) {
        count += entry.second.size();
    }
    return count;
}

};  // namespace kk::renderer
