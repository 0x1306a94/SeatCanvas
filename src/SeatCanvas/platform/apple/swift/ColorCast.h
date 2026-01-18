//
//  ColorCast.h
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import <UIKit/UIColor.h>
#import <optional>
#import <tgfx/core/Color.h>

namespace kk::bridge {
tgfx::Color UIColorToTGFX(UIColor *color);
std::optional<tgfx::Color> UIColorToTGFXOptional(UIColor *color);
UIColor *UIColorFromTGFX(const tgfx::Color &color);
UIColor *_Nullable UIColorFromTGFXOptional(const std::optional<tgfx::Color> &color);
};  // namespace kk::bridge
