//
//  ColorCast.mm
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "ColorCast.h"

namespace kk::bridge {
tgfx::Color PlatformColorToTGFX(PlatformColor *_Nullable color) {
    if (!color || color == PlatformColor.clearColor) {
        return tgfx::Color::Transparent();
    }
    CGFloat r, g, b, a;
    [color getRed:&r green:&g blue:&b alpha:&a];

    return tgfx::Color{
        static_cast<float>(r),
        static_cast<float>(g),
        static_cast<float>(b),
        static_cast<float>(a),
    };
}

std::optional<tgfx::Color> PlatformColorToTGFXOptional(PlatformColor *_Nullable color) {
    if (!color) {
        return std::nullopt;
    }

    return PlatformColorToTGFX(color);
}

PlatformColor *PlatformColorFromTGFX(const tgfx::Color &color) {
    return [PlatformColor colorWithRed:static_cast<CGFloat>(color.red) green:static_cast<CGFloat>(color.green) blue:static_cast<CGFloat>(color.blue) alpha:static_cast<CGFloat>(color.alpha)];
}

PlatformColor *_Nullable PlatformColorFromTGFXOptional(const std::optional<tgfx::Color> &color) {
    if (!color) {
        return nil;
    }

    return PlatformColorFromTGFX(color.value());
}
};  // namespace kk::bridge
