//
//  SeatZoneLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatZoneLayer_hpp
#define SeatZoneLayer_hpp

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

  protected:
    SeatZoneLayer() = default;
};
};  // namespace kk::layer

#endif /* SeatZoneLayer_hpp */
