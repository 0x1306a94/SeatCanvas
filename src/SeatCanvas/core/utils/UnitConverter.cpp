//
//  UnitConverter.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/6.
//

#include "UnitConverter.hpp"

#include "SystemProperties.hpp"

namespace kk::utils {

float vp2px(float vp) {
    auto density = SystemProperties::Instance().density;
    return vp * density;
}

float px2vp(float px) {
    auto density = SystemProperties::Instance().density;
    return px / density;
}

float fp2px(float fp) {
    auto fontScale = SystemProperties::Instance().fontScale;
    return fp * fontScale;
}

float px2fp(float px) {
    auto fontScale = SystemProperties::Instance().fontScale;
    return px / fontScale;
}

}  // namespace kk::utils
