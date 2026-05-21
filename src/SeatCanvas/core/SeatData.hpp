//
//  SeatData.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/18.
//

#ifndef SeatData_hpp
#define SeatData_hpp

#include <cstdint>
#include <limits>
#include <string>

namespace kk {

static constexpr uint16_t kNoPricecodeIndex = std::numeric_limits<uint16_t>::max();

struct SeatStatusUpdate {
    std::string seatId = {};
    uint32_t status = 0;
};

struct SeatData {
    std::string seatId = {};
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;  // degrees, clockwise from upright
    uint16_t pricecodeIndex = kNoPricecodeIndex;

    SeatData() = default;

    SeatData(const std::string &seatId, float x, float y, float rotationDegrees = 0.0f, uint16_t pricecodeIndexValue = kNoPricecodeIndex)
        : seatId(seatId)
        , x(x)
        , y(y)
        , rotation(rotationDegrees)
        , pricecodeIndex(pricecodeIndexValue) {
    }

    bool isValid() const {
        return !seatId.empty();
    }
};

}  // namespace kk

#endif /* SeatData_hpp */
