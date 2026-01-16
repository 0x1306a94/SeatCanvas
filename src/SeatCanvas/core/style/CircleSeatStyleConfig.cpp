//
//  CircleSeatStyleConfig.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "CircleSeatStyleConfig.hpp"

namespace kk::renderer {

CircleSeatStyleConfig::CircleSeatStyleConfig(const tgfx::Color &fillColor,
                                             const tgfx::Color &overlayColor,
                                             const tgfx::Color &checkmarkColor)
    : SeatStyleConfig(SeatStyleType::Circle)
    , fillColor(fillColor)
    , overlayColor(overlayColor)
    , checkmarkColor(checkmarkColor) {
}

std::shared_ptr<CircleSeatStyleConfig> CircleSeatStyleConfig::Make(const tgfx::Color &fillColor,
                                                                   const tgfx::Color &overlayColor,
                                                                   const tgfx::Color &checkmarkColor) {
    return std::shared_ptr<CircleSeatStyleConfig>(new CircleSeatStyleConfig(fillColor, overlayColor, checkmarkColor));
}

bool CircleSeatStyleConfig::isEqual(const SeatStyleConfig &other) const {
    const auto *otherConfig = static_cast<const CircleSeatStyleConfig *>(&other);
    if (!otherConfig) {
        return false;
    }
    return fillColor == otherConfig->fillColor &&
        overlayColor == otherConfig->overlayColor &&
        checkmarkColor == otherConfig->checkmarkColor;
}

}  // namespace kk::renderer
