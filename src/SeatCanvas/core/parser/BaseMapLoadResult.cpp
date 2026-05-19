//
//  BaseMapLoadResult.cpp
//  SeatCanvas
//
//  Created by king on 2026/5/18.
//

#include "BaseMapLoadResult.hpp"

#include "core/BaseMapConfig.hpp"

namespace kk::parser {

BaseMapLoadResult::BaseMapLoadResult(std::unique_ptr<BaseMapParseResult> parseResult) {
    if (parseResult == nullptr) {
        return;
    }

    textLayer = std::move(parseResult->textLayer);
    baseMapSize = parseResult->size;
    miniLayer = std::move(parseResult->miniLayer);
    miniLayerMap = std::move(parseResult->miniLayerMap);
    meshBuilder = std::move(parseResult->meshBuilder);
}

std::shared_ptr<kk::BaseMapConfig> BaseMapLoadResult::makeBaseMapConfig() {
    return std::make_shared<kk::BaseMapConfig>(meshBuilder, textLayer, miniLayer, std::move(miniLayerMap), baseMapSize);
}

}  // namespace kk::parser
