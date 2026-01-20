//
//  SeatZoneLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatZoneLayer.hpp"

namespace kk::layer {
std::shared_ptr<SeatZoneLayer> SeatZoneLayer::Make() {
    return std::shared_ptr<SeatZoneLayer>(new SeatZoneLayer());
}

};  // namespace kk::layer
