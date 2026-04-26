//
//  SeatData.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/18.
//

#ifndef SeatData_hpp
#define SeatData_hpp

#include <string>

namespace kk {

struct SeatData {
    std::string seatId = {};
    float x = 0.0f;
    float y = 0.0f;
    float rotation = 0.0f;  // degrees, clockwise from upright

    SeatData() = default;

    SeatData(const std::string &seatId, float x, float y, float rotationDegrees = 0.0f)
        : seatId(seatId)
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
