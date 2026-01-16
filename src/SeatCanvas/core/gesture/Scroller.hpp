//
//  Scroller.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef Scroller_hpp
#define Scroller_hpp

#include "GestureConstant.hpp"
#include "ScrollerValue.hpp"

#include <optional>

namespace kk::gesture {

class Scroller {
  public:
    explicit Scroller(float deceleration_rate = DecelerationRateNormal);

    void setDecelerationRate(float rate);

    void fling(float velocity);

    std::optional<ScrollerValue> value(float time) const;

    void reset();

  private:
    float _deceleration_rate;
    float _initial_velocity;
};
};  // namespace kk::gesture

#endif /* Scroller_hpp */
