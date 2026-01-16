//
//  ScrollProperties.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#ifndef ScrollProperties_hpp
#define ScrollProperties_hpp

#include "BounceEdge.hpp"

#include <memory>

namespace kk::gesture {

class Scroller;
class SpringBack;
class ScrollProperties {
  public:
    bool is_decelerating = false;
    bool is_bouncing = false;

    BounceEdge bounce_edge = BounceEdge::NONE;

    double animation_begin_time = 0;
    float animation_begin_offset = 0;
    float animation_begin_velocity = 0;

    std::unique_ptr<Scroller> scroller;
    std::unique_ptr<SpringBack> springBack;

    explicit ScrollProperties();

    void clear();

    void reset(float velocity, float offset);

    void prepareScroller(float rate);

    void prepareSpringBack();
};

};  // namespace kk::gesture

#endif /* ScrollProperties_hpp */
