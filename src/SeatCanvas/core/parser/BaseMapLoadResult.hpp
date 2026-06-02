//
//  BaseMapLoadResult.hpp
//  SeatCanvas
//
//  Created by king on 2026/5/18.
//

#ifndef BaseMapLoadResult_hpp
#define BaseMapLoadResult_hpp

#include <memory>
#include <string>
#include <unordered_map>

#include <tgfx/core/Size.h>

#include "core/parser/BaseMapParseResult.hpp"

namespace tgfx {
class Layer;
class Picture;
};  // namespace tgfx

namespace kk {
class BaseMapConfig;
};

namespace kk::layer {
class BaseMapRootLayer;
};

namespace kk::renderer {
class BaseMapMeshBuilder;
};

namespace kk::parser {

struct BaseMapLoadResult {
    std::shared_ptr<tgfx::Layer> textLayer = {nullptr};
    std::shared_ptr<tgfx::Picture> textPicture = {nullptr};
    tgfx::Size baseMapSize = {};
    std::shared_ptr<kk::layer::BaseMapRootLayer> miniLayer = {nullptr};
    std::unordered_map<std::string, std::shared_ptr<tgfx::Layer>> miniLayerMap = {};
    std::shared_ptr<kk::renderer::BaseMapMeshBuilder> meshBuilder = {nullptr};

    explicit BaseMapLoadResult(std::unique_ptr<BaseMapParseResult> parseResult);

    std::shared_ptr<kk::BaseMapConfig> makeBaseMapConfig();
};

}  // namespace kk::parser

#endif /* BaseMapLoadResult_hpp */
