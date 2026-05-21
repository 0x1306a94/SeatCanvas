//
//  SeatRenderStyleKey.cpp
//  SeatCanvas
//

#include "SeatRenderStyleKey.hpp"

#include <unordered_map>

namespace kk::renderer {

namespace {

static constexpr char kPricecodePrefix[] = "pricecode_";
static constexpr char kStatusMarker[] = "_status_";
static constexpr char kSelectedMarker[] = "_selected_";

}  // namespace

bool SeatRenderStyleKey::operator==(const SeatRenderStyleKey &other) const {
    return pricecodeIndex == other.pricecodeIndex && status == other.status && selected == other.selected;
}

size_t SeatRenderStyleKeyHash::operator()(const SeatRenderStyleKey &key) const {
    size_t hash = std::hash<uint16_t>{}(key.pricecodeIndex);
    hash ^= std::hash<uint32_t>{}(key.status) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    hash ^= std::hash<bool>{}(key.selected) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    return hash;
}

std::string composeSeatStyleId(const std::string &pricecode, uint32_t status, bool selected) {
    return std::string(kPricecodePrefix) + pricecode + kStatusMarker + std::to_string(status) + kSelectedMarker +
        (selected ? "1" : "0");
}

std::optional<ParsedSeatStyleId> parseSeatStyleId(const std::string &styleId) {
    if (styleId.size() <= sizeof(kPricecodePrefix) - 1) {
        return std::nullopt;
    }
    if (styleId.compare(0, sizeof(kPricecodePrefix) - 1, kPricecodePrefix) != 0) {
        return std::nullopt;
    }

    auto statusPos = styleId.find(kStatusMarker);
    if (statusPos == std::string::npos) {
        return std::nullopt;
    }

    auto selectedPos = styleId.find(kSelectedMarker, statusPos + sizeof(kStatusMarker) - 1);
    if (selectedPos == std::string::npos) {
        return std::nullopt;
    }

    ParsedSeatStyleId parsed = {};
    parsed.pricecode = styleId.substr(sizeof(kPricecodePrefix) - 1, statusPos - (sizeof(kPricecodePrefix) - 1));

    auto statusText = styleId.substr(statusPos + sizeof(kStatusMarker) - 1, selectedPos - (statusPos + sizeof(kStatusMarker) - 1));
    if (statusText.empty()) {
        return std::nullopt;
    }
    uint64_t statusValue = 0;
    for (char ch : statusText) {
        if (ch < '0' || ch > '9') {
            return std::nullopt;
        }
        statusValue = statusValue * 10 + static_cast<uint64_t>(ch - '0');
        if (statusValue > std::numeric_limits<uint32_t>::max()) {
            return std::nullopt;
        }
    }
    parsed.status = static_cast<uint32_t>(statusValue);

    auto selectedText = styleId.substr(selectedPos + sizeof(kSelectedMarker) - 1);
    if (selectedText == "1") {
        parsed.selected = true;
    } else if (selectedText == "0") {
        parsed.selected = false;
    } else {
        return std::nullopt;
    }

    return parsed;
}

uint16_t resolvePricecodeIndex(const std::unordered_map<std::string, uint16_t> &pricecodeToIndex, const std::string &pricecode) {
    if (pricecode.empty()) {
        return kNoPricecodeIndex;
    }
    auto iter = pricecodeToIndex.find(pricecode);
    if (iter == pricecodeToIndex.end()) {
        return kNoPricecodeIndex;
    }
    return iter->second;
}

}  // namespace kk::renderer
