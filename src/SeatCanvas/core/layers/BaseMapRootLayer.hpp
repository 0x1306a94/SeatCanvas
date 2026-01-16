//
//  BaseMapRootLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/10.
//

#ifndef BaseMapRootLayer_hpp
#define BaseMapRootLayer_hpp

#include <tgfx/layers/ShapeLayer.h>

#include "CustomLayerType.hpp"

namespace kk::layer {
class BaseMapRootLayer : public tgfx::ShapeLayer {
  public:
    static std::shared_ptr<BaseMapRootLayer> Make();

    virtual ~BaseMapRootLayer() override = default;

    tgfx::LayerType type() const override {
        return static_cast<tgfx::LayerType>(CustomLayerType::BaseMapRoot);
    }

  protected:
    BaseMapRootLayer() = default;
};
};  // namespace kk::layer

#endif /* BaseMapRootLayer_hpp */
