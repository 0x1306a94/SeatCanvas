//
//  BaseMapConfig.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/5.
//

#include "BaseMapConfig.hpp"

#include "core/BaseMapLayerManager.hpp"
#include "core/layers/BaseMapRootLayer.hpp"

namespace kk {
BaseMapConfig::BaseMapConfig(std::shared_ptr<kk::BaseMapLayerManager> layerManager, std::shared_ptr<kk::layer::BaseMapRootLayer> baseMapLayer, std::shared_ptr<kk::layer::BaseMapRootLayer> minimapLayer, const tgfx::Size &baseMapSize)
    : _layerManager(std::move(layerManager))
    , _baseMapLayer(std::move(baseMapLayer))
    , _minimapLayer(std::move(minimapLayer))
    , _baseMapSize(baseMapSize) {
}

std::shared_ptr<kk::layer::BaseMapRootLayer> BaseMapConfig::baseMapLayer() const {
    return _baseMapLayer;
}

std::shared_ptr<kk::layer::BaseMapRootLayer> BaseMapConfig::minimapLayer() const {
    return _minimapLayer;
}

const tgfx::Size &BaseMapConfig::baseMapSize() const {
    return _baseMapSize;
}

const kk::BaseMapLayerManager *BaseMapConfig::layerManager() const {
    return _layerManager.get();
}

};  // namespace kk
