//
//  GestureSample.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef GestureSample_hpp
#define GestureSample_hpp

#include "GestureState.hpp"

namespace kk::gesture {
struct GestureSample {
    GestureState state;
    float translation;
    double timestamp;
};

};  // namespace kk::gesture

#endif /* GestureSample_hpp */
