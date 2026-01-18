//
//  FontManagerBridge.m
//  SeatCanvas
//
//  Created by KK on 2026/1/18.
//

#import "FontManagerBridge.h"
#import "core/Platform.hpp"

namespace kk::bridge {
void RegisterFallbackFonts() {
    kk::Platform::Current()->registerFallbackFonts();
}
};  // namespace kk::bridge
