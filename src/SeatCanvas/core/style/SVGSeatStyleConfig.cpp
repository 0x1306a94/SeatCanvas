//
//  SVGSeatStyleConfig.cpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#include "SVGSeatStyleConfig.hpp"

namespace kk::renderer {
SVGSeatStyleConfig::SVGSeatStyleConfig(const std::string &content)
    : SeatStyleConfig(SeatStyleType::SVG)
    , content(content) {
}

std::shared_ptr<SVGSeatStyleConfig> SVGSeatStyleConfig::Make(const std::string &content) {
    return std::shared_ptr<SVGSeatStyleConfig>(new SVGSeatStyleConfig(content));
}

bool SVGSeatStyleConfig::isEqual(const SeatStyleConfig &other) const {
    const auto *otherConfig = static_cast<const SVGSeatStyleConfig *>(&other);
    if (!otherConfig) {
        return false;
    }
    return content == otherConfig->content;
}
};  // namespace kk::renderer
