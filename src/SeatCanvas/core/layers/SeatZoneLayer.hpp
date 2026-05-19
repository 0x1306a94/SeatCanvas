//
//  SeatZoneLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatZoneLayer_hpp
#define SeatZoneLayer_hpp

#include <optional>

#include <tgfx/layers/ShapeLayer.h>

#include "CustomLayerType.hpp"

namespace kk::layer {
class SeatZoneLayer : public tgfx::ShapeLayer {
  public:
    static std::shared_ptr<SeatZoneLayer> Make();

    virtual ~SeatZoneLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::Zone);
    }

    void setPrimitiveFillColor(const tgfx::Color &color);

    const tgfx::Color &primitiveFillColor() const {
        return _primitiveFillColor;
    }

    void setAlternateFillColor(std::optional<tgfx::Color> color);

    const std::optional<tgfx::Color> alternateFillColor() const {
        return _alternateFillColor;
    }

  protected:
    SeatZoneLayer() = default;

  private:
    tgfx::Color _primitiveFillColor = tgfx::Color::Transparent();
    std::optional<tgfx::Color> _alternateFillColor = std::nullopt;
};
};  // namespace kk::layer

#endif /* SeatZoneLayer_hpp */
