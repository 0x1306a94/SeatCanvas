//
//  ColorHexParser.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "ColorHexParser.hpp"

#include <sstream>
#include <tgfx/platform/Print.h>

namespace kk::renderer {

bool ParseColorFromARGBHex(const std::string &hexString, tgfx::Color &outColor) {
    if (hexString.empty() || hexString[0] != '#') {
        tgfx::PrintError("Invalid color hex string: must start with #");
        return false;
    }

    std::string hex = hexString.substr(1);
    if (hex.length() != 8 && hex.length() != 6) {
        tgfx::PrintError("Invalid color hex string: must be 6 or 8 characters");
        return false;
    }

    uint32_t hexValue = 0;
    std::stringstream ss;
    ss << std::hex << hex;
    ss >> hexValue;

    uint8_t r, g, b, a;

    if (hex.length() == 8) {
        // #RRGGBBAA format
        a = (hexValue >> 24) & 0xFF;
        r = (hexValue >> 16) & 0xFF;
        g = (hexValue >> 8) & 0xFF;
        b = hexValue & 0xFF;
    } else {
        // #RRGGBB format (assume alpha = 255)
        a = 255;
        r = (hexValue >> 16) & 0xFF;
        g = (hexValue >> 8) & 0xFF;
        b = hexValue & 0xFF;
    }

    outColor = tgfx::Color::FromRGBA(r, g, b, a);
    return true;
}

std::string ColorToARGBHex(const tgfx::Color &color) {
    uint8_t r = static_cast<uint8_t>(color.red * 255);
    uint8_t g = static_cast<uint8_t>(color.green * 255);
    uint8_t b = static_cast<uint8_t>(color.blue * 255);
    uint8_t a = static_cast<uint8_t>(color.alpha * 255);
    char hex[10];
    snprintf(hex, sizeof(hex), "#%02X%02X%02X%02X", a, r, g, b);
    return std::string(hex);
}

}  // namespace kk::renderer
