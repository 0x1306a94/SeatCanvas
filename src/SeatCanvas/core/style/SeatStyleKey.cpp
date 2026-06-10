//
//  SeatStyleKey.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#include "SeatStyleKey.hpp"

#include <nlohmann/json.hpp>
#include <tuple>

namespace kk {

SeatStyleKey::SeatStyleKey(uint32_t status, bool selected)
    : status(status)
    , selected(selected) {
}

bool SeatStyleKey::operator<(const SeatStyleKey &other) const {
    return std::tie(status, selected) < std::tie(other.status, other.selected);
}

bool SeatStyleKey::operator==(const SeatStyleKey &other) const {
    return status == other.status && selected == other.selected;
}

std::size_t SeatStyleKey::hash() const {
    std::size_t h1 = std::hash<uint32_t>{}(status);
    std::size_t h2 = std::hash<bool>{}(selected);
    return (h1 << 4) | h2;
}

}  // namespace kk
