//
//  BaseMapConfig.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/5.
//

#include "BaseMapConfig.hpp"

#include "core/layers/BaseMapRootLayer.hpp"
#include "core/renderer/BaseMapMeshBuilder.hpp"

namespace kk {
BaseMapConfig::BaseMapConfig(std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder,
                             std::shared_ptr<tgfx::Layer> textLayer,
                             std::shared_ptr<kk::layer::BaseMapRootLayer> minimapLayer,
                             std::unordered_map<std::string, std::shared_ptr<tgfx::Layer>> miniLayerMap,
                             const tgfx::Size &baseMapSize)
    : _meshBuilder(std::move(meshBuilder))
    , _textLayer(std::move(textLayer))
    , _minimapLayer(std::move(minimapLayer))
    , _miniLayerMap(std::move(miniLayerMap))
    , _baseMapSize(baseMapSize) {
}

std::shared_ptr<tgfx::Layer> BaseMapConfig::textLayer() const {
    return _textLayer;
}

std::shared_ptr<kk::layer::BaseMapRootLayer> BaseMapConfig::minimapLayer() const {
    return _minimapLayer;
}

const std::unordered_map<std::string, std::shared_ptr<tgfx::Layer>> &BaseMapConfig::miniLayerMap() const {
    return _miniLayerMap;
}

const tgfx::Size &BaseMapConfig::baseMapSize() const {
    return _baseMapSize;
}

std::shared_ptr<kk::renderer::BaseMapMeshBuilder> BaseMapConfig::meshBuilder() const {
    return _meshBuilder;
}

};  // namespace kk
