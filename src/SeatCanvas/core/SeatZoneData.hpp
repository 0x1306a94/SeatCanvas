//
//  SeatZoneData.hpp
//  SeatCanvas
//
//  Created by king on 2025/1/XX.
//

#ifndef SeatZoneData_hpp
#define SeatZoneData_hpp

#include <tgfx/core/Color.h>

#include <optional>
#include <string>

namespace kk {
struct SeatZoneData {
    std::string zoneId = {};
    std::optional<tgfx::Color> color = {std::nullopt};
    std::optional<tgfx::Color> priceColor = {std::nullopt};

    SeatZoneData() = default;

    SeatZoneData(const std::string &zoneId, const std::optional<tgfx::Color> &color, const std::optional<tgfx::Color> &priceColor)
        : zoneId(zoneId)
        , color(color)
        , priceColor(priceColor) {
    }

    bool isValid() const {
        return !zoneId.empty();
    }
};

}  // namespace kk

#endif /* SeatZoneData_hpp */
