//
//  SeatRegionLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatRegionLayer.hpp"

namespace kk::layer {
std::shared_ptr<SeatRegionLayer> SeatRegionLayer::Make() {
    return std::shared_ptr<SeatRegionLayer>(new SeatRegionLayer());
}

};  // namespace kk::layer
