//
//  SeatInfo.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/8.
//

#include "SeatInfo.hpp"

namespace kk {
bool SeatInfo::isValid() const {
    if (seatId.empty()) {
        return false;
    }

    if (rect.isEmpty()) {
        return false;
    }

    return true;
}
};  // namespace kk
