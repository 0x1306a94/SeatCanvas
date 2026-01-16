//
//  BaseMapRootLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "BaseMapRootLayer.hpp"

namespace kk::layer {
std::shared_ptr<BaseMapRootLayer> BaseMapRootLayer::Make() {
    return std::shared_ptr<BaseMapRootLayer>(new BaseMapRootLayer());
}

};  // namespace kk::layer
