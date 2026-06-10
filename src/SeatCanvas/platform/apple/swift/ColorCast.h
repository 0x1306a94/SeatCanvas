//
//  ColorCast.h
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import <TargetConditionals.h>

#import <optional>
#import <tgfx/core/Color.h>

#import "platform/apple/PlatformTypes.h"

NS_ASSUME_NONNULL_BEGIN

namespace kk::bridge {
tgfx::Color PlatformColorToTGFX(PlatformColor *_Nullable color);
std::optional<tgfx::Color> PlatformColorToTGFXOptional(PlatformColor *_Nullable color);
PlatformColor *PlatformColorFromTGFX(const tgfx::Color &color);
PlatformColor *_Nullable PlatformColorFromTGFXOptional(const std::optional<tgfx::Color> &color);
};  // namespace kk::bridge

NS_ASSUME_NONNULL_END
