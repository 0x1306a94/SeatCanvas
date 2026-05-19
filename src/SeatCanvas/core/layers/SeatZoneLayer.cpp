//
//  SeatZoneLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#include "SeatZoneLayer.hpp"

namespace kk::layer {
std::shared_ptr<SeatZoneLayer> SeatZoneLayer::Make() {
    return std::shared_ptr<SeatZoneLayer>(new SeatZoneLayer());
}

void SeatZoneLayer::setPrimitiveFillColor(const tgfx::Color &color) {
    _primitiveFillColor = color;
    setFillStyle(tgfx::ShapeStyle::Make(color));
}

void SeatZoneLayer::setAlternateFillColor(std::optional<tgfx::Color> color) {
    if (!color) {
        _alternateFillColor.reset();
        setFillStyle(tgfx::ShapeStyle::Make(_primitiveFillColor));
        return;
    }
    _alternateFillColor = std::move(color);
    setFillStyle(tgfx::ShapeStyle::Make(_alternateFillColor.value()));
}

};  // namespace kk::layer
