//
//  SeatInfo.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/8.
//

#ifndef SeatInfo_hpp
#define SeatInfo_hpp

#include <cstdint>
#include <string>

#include <tgfx/core/Rect.h>

namespace kk {
/**
 * 座位信息
 * 状态值由业务层定义，可以使用 SeatStatus 枚举或自定义的 uint32_t 值
 */
struct SeatInfo {
    std::string seatId = {};
    uint32_t status = 0;
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
