//
//  SeatRootLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatRootLayer.hpp"

namespace kk::layer {
std::shared_ptr<SeatRootLayer> SeatRootLayer::Make() {
    return std::shared_ptr<SeatRootLayer>(new SeatRootLayer());
}

};  // namespace kk::layer
