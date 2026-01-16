//
//  BaseMapConfig.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/5.
//

#ifndef BaseMapConfig_hpp
#define BaseMapConfig_hpp

#include <memory>

#include <tgfx/core/Size.h>

namespace kk::layer {
class BaseMapRootLayer;
};

namespace kk {
class BaseMapLayerManager;
class BaseMapConfig {
  public:
    explicit BaseMapConfig(std::shared_ptr<kk::BaseMapLayerManager> layerManager, std::shared_ptr<kk::layer::BaseMapRootLayer> baseMapLayer, std::shared_ptr<kk::layer::BaseMapRootLayer> minimapLayer, const tgfx::Size &baseMapSize);
    ~BaseMapConfig() = default;
    std::shared_ptr<kk::layer::BaseMapRootLayer> baseMapLayer() const;
    std::shared_ptr<kk::layer::BaseMapRootLayer> minimapLayer() const;
    const tgfx::Size &baseMapSize() const;

    const kk::BaseMapLayerManager *layerManager() const;

  private:
    std::shared_ptr<kk::layer::BaseMapRootLayer> _baseMapLayer;
    std::shared_ptr<kk::layer::BaseMapRootLayer> _minimapLayer;
    std::shared_ptr<kk::BaseMapLayerManager> _layerManager;
    tgfx::Size _baseMapSize;
};
};  // namespace kk

#endif /* BaseMapConfig_hpp */
