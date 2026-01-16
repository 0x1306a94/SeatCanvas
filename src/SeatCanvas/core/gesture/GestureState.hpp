//
//  GestureState.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef GestureState_hpp
#define GestureState_hpp

namespace kk::gesture {
enum class GestureState {
    POSSIBLE,
    BEGAN,
    CHANGED,
    ENDED,
    CANCELLED,
};
};  // namespace kk::gesture

#endif /* GestureState_hpp */
