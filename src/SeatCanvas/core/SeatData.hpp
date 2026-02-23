//
//  SeatData.hpp
//  SeatCanvas
//
//  Created by king on 2025/1/XX.
//

#ifndef SeatData_hpp
#define SeatData_hpp

#include <cstdint>
#include <string>

namespace kk {

struct SeatData {
    std::string seatId = {};
    uint32_t status = 0;
    bool selected = false;
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;  // degrees, clockwise from upright

    SeatData() = default;

    SeatData(const std::string &seatId, uint32_t status, bool selected, float x, float y, float rotationDegrees = 0.0f)
        : seatId(seatId)
        , status(status)
        , selected(selected)
        , x(x)
        , y(y)
        , rotation(rotationDegrees) {
    }

    bool isValid() const {
        return !seatId.empty();
    }
};

}  // namespace kk

#endif /* SeatData_hpp */
