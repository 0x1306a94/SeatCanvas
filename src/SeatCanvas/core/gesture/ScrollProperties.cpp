//
//  ScrollProperties.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/21.
//

#include "ScrollProperties.hpp"

#include "Scroller.hpp"
#include "SpringBack.hpp"
#include "core/Platform.hpp"

#include <cassert>

namespace kk::gesture {
ScrollProperties::ScrollProperties() {
}

void ScrollProperties::clear() {
    is_decelerating = false;
    is_bouncing = false;
    bounce_edge = BounceEdge::NONE;
    animation_begin_time = 0;
    animation_begin_offset = 0;
    animation_begin_velocity = 0;
}

void ScrollProperties::reset(float velocity, float offset) {
    if (scroller) {
        scroller->reset();
    }
    if (springBack) {
        springBack->reset();
    }

    assert(!std::isnan(velocity) && !std::isnan(offset));
    animation_begin_offset = offset;
    animation_begin_time = Platform::Current()->currentMediaTime();
    animation_begin_velocity = velocity;
}

void ScrollProperties::prepareScroller(float rate) {
    if (scroller == nullptr) {
        scroller = std::make_unique<Scroller>(rate);
    } else {
        scroller->setDecelerationRate(rate);
    }
}

void ScrollProperties::prepareSpringBack() {
    if (springBack == nullptr) {
        springBack = std::make_unique<SpringBack>();
    }
}

};  // namespace kk::gesture
