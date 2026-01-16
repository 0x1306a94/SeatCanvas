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

namespace tgfx {
class Layer;
};

namespace kk::layer {
class BaseMapRootLayer;
};

namespace kk::renderer {
class BaseMapMeshBuilder;
};

namespace kk {
class BaseMapConfig {
  public:
    explicit BaseMapConfig(std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder,
                           std::shared_ptr<tgfx::Layer> textLayer,
                           std::shared_ptr<kk::layer::BaseMapRootLayer> minimapLayer,
                           const tgfx::Size &baseMapSize);
    ~BaseMapConfig() = default;
    std::shared_ptr<tgfx::Layer> textLayer() const;
    std::shared_ptr<kk::layer::BaseMapRootLayer> minimapLayer() const;
    const tgfx::Size &baseMapSize() const;

    std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder() const;

  private:
    std::shared_ptr<tgfx::Layer> _textLayer;
    std::shared_ptr<kk::layer::BaseMapRootLayer> _minimapLayer;
    std::shared_ptr<kk::renderer::BaseMapMeshBuilder> _meshBuilder;
    tgfx::Size _baseMapSize;
};
};  // namespace kk

#endif /* BaseMapConfig_hpp */
