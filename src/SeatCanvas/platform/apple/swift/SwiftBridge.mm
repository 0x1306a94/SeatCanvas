//
//  SwiftBridge.mm
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#import "SwiftBridge.h"

#import "core/utils/SystemProperties.hpp"

namespace kk {

void SeatCanvasInitSystemProperties(CGFloat density, CGFloat fontScale) {
    auto &properties = kk::utils::SystemProperties::Instance();
    properties.updateDensity(static_cast<float>(density));
    properties.updateFontScale(static_cast<float>(fontScale));
}

}  // namespace kk
