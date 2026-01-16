//
//  SeatItemImageKey.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/17.
//

#include "SeatItemImageKey.hpp"

#include <tuple>

namespace kk {

SeatItemImageKey::SeatItemImageKey(kk::SeatStatus status, bool selected)
    : status(status)
    , selected(selected) {
}

bool SeatItemImageKey::operator<(const SeatItemImageKey &other) const {
    return std::tie(status, selected) < std::tie(other.status, other.selected);
}

bool SeatItemImageKey::operator==(const SeatItemImageKey &other) const {
    return status == other.status && selected == other.selected;
}

std::size_t SeatItemImageKey::hash() const {
    std::size_t h1 = std::hash<int>{}(static_cast<int>(status));
    std::size_t h2 = std::hash<int>{}(selected);
    return (h1 << 4) | h2;
}

};  // namespace kk
