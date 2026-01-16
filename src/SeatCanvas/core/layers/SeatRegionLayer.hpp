//
//  SeatRegionLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatRegionLayer_hpp
#define SeatRegionLayer_hpp

#include <tgfx/layers/ShapeLayer.h>

#include "CustomLayerType.hpp"

namespace kk::layer {
class SeatRegionLayer : public tgfx::ShapeLayer {
  public:
    static std::shared_ptr<SeatRegionLayer> Make();

    virtual ~SeatRegionLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::Region);
    }

  protected:
    SeatRegionLayer() = default;
};
};  // namespace kk::layer

#endif /* SeatRegionLayer_hpp */
