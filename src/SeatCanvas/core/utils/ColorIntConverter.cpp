//
//  ColorIntConverter.cpp
//  SeatCanvas
//
//  Created by king on 2026/5/18.
//

#include "ColorIntConverter.hpp"

namespace kk::utils {

tgfx::Color ColorFromARGBInt(uint32_t argb) {
    auto alpha = static_cast<uint8_t>((argb >> 24) & 0xFF);
    auto red = static_cast<uint8_t>((argb >> 16) & 0xFF);
    auto green = static_cast<uint8_t>((argb >> 8) & 0xFF);
    auto blue = static_cast<uint8_t>(argb & 0xFF);
    return tgfx::Color::FromRGBA(red, green, blue, alpha);
}

int32_t ColorToARGBInt(const tgfx::Color &color) {
    auto alpha = static_cast<uint8_t>(color.alpha * 255);
    auto red = static_cast<uint8_t>(color.red * 255);
    auto green = static_cast<uint8_t>(color.green * 255);
    auto blue = static_cast<uint8_t>(color.blue * 255);
    uint32_t argb = (static_cast<uint32_t>(alpha) << 24) |
        (static_cast<uint32_t>(red) << 16) |
        (static_cast<uint32_t>(green) << 8) |
        static_cast<uint32_t>(blue);
    return static_cast<int32_t>(argb);
}

}  // namespace kk::utils
