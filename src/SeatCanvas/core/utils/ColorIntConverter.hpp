//
//  ColorIntConverter.hpp
//  SeatCanvas
//
//  Created by king on 2026/5/18.
//

#ifndef ColorIntConverter_hpp
#define ColorIntConverter_hpp

#include <cstdint>

#include <tgfx/core/Color.h>

namespace kk::utils {

tgfx::Color ColorFromARGBInt(uint32_t argb);
int32_t ColorToARGBInt(const tgfx::Color &color);

}  // namespace kk::utils

#endif /* ColorIntConverter_hpp */
