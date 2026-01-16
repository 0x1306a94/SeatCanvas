//
//  SeatInfo.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/8.
//

#ifndef SeatInfo_hpp
#define SeatInfo_hpp

#include <string>

#include <tgfx/core/Rect.h>

#include "core/SeatShapeStyle.h"
#include "core/SeatStatus.h"

namespace kk {
struct SeatInfo {
    std::string seatId{""};
    SeatStatus status{SeatStatus::Available};
    SeatShapeStyle style{SeatShapeStyle::Circle};
    bool selected{false};
    tgfx::Rect rect{0, 0, 0, 0};

    explicit SeatInfo(const std::string &seatId, const tgfx::Rect &rect)
        : seatId(seatId)
        , rect(rect) {
    }

    bool isValid() const;
};
};  // namespace kk

#endif /* SeatInfo_hpp */
