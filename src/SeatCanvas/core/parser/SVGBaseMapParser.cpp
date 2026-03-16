//
//  SVGBaseMapParser.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "SVGBaseMapParser.hpp"

#include "core/renderer/BaseMapMeshBuilder.hpp"
#include "core/svg/ConvertSVGLayer.hpp"
#include "core/svg/SVGMeshParser.hpp"

#include <tgfx/core/Data.h>
#include <tgfx/core/Stream.h>
#include <tgfx/svg/SVGDOM.h>

namespace kk::parser {

std::unique_ptr<BaseMapParseResult> SVGBaseMapParser::parse(std::shared_ptr<tgfx::Data> data, std::shared_ptr<tgfx::Data> configData) {
    if (!data || data->empty()) {
        return nullptr;
    }

    // 解析 SVG DOM
    auto stream = tgfx::Stream::MakeFromData(data);
    if (!stream) {
        return nullptr;
    }

    auto dom = tgfx::SVGDOM::Make(*stream);
    if (!dom) {
        return nullptr;
    }

    // 使用 SVGMeshParser 解析 mesh 数据
    auto parseConfig = kk::svg::SVGParseConfig::FromJSON(configData);
    kk::svg::SVGMeshParser meshParser(parseConfig);
    auto meshResult = meshParser.parse(dom);
    if (!meshResult) {
        return nullptr;
    }

    // 解析 Text Layer
    auto textLayer = kk::svg::convertSVGDomTextNodeToLayer(dom);

    // Minimap 需要完整的 Layer 树
    auto minimapResult = kk::svg::convertSVGDomToLayer(dom);
    if (!minimapResult) {
        return nullptr;
    }

    // 构建统一结果
    auto result = std::make_unique<BaseMapParseResult>();
    result->size = meshResult->size;
    result->meshBuilder = std::move(meshResult->meshBuilder);
    result->textLayer = std::move(textLayer);
    result->miniLayer = std::move(minimapResult->layer);

    return result;
}

}  // namespace kk::parser
