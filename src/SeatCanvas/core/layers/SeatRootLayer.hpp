//
//  SeatRootLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef SeatRootLayer_hpp
#define SeatRootLayer_hpp

#include <tgfx/layers/ShapeLayer.h>

#include "CustomLayerType.hpp"

namespace kk::layer {
class SeatRootLayer : public tgfx::ShapeLayer {
  public:
    static std::shared_ptr<SeatRootLayer> Make();

    virtual ~SeatRootLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::SeatRoot);
    }

  protected:
    SeatRootLayer() = default;
};
};  // namespace kk::layer

#endif /* SeatRootLayer_hpp */
