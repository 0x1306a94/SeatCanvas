//
//  ConvertSVGLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef ConvertSVGLayer_hpp
#define ConvertSVGLayer_hpp

#include <memory>

#include <tgfx/core/Size.h>
#include <tgfx/svg/SVGLengthContext.h>

namespace tgfx {
class SVGDOM;
class SVGNode;
class SVGLine;
class SVGCircle;
class SVGEllipse;
class SVGPath;
class SVGPoly;
class SVGRect;
class SVGText;
class SVGGroup;
class Layer;
};  // namespace tgfx

namespace kk::layer {
class BaseMapRootLayer;
class SeatRegionLayer;
class SeatTextLayer;
};  // namespace kk::layer

namespace kk::svg {

struct ConvertSVGLayerOptions {
    bool supportText{false};
    static const ConvertSVGLayerOptions &Default() {
        static const ConvertSVGLayerOptions options{};
        return options;
    }
};

struct ConvertSVGLayerResult {
    std::shared_ptr<kk::layer::BaseMapRootLayer> layer;
    tgfx::Size size;
    explicit ConvertSVGLayerResult(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &size)
        : layer(std::move(layer))
        , size(size) {
    }
};

/// 将 SVGDOM 转为 Layer 树（用于 minimap）
/// @param dom SVG DOM
/// @param options 转换选项
std::unique_ptr<ConvertSVGLayerResult> convertSVGDomToLayer(std::shared_ptr<tgfx::SVGDOM> dom, const ConvertSVGLayerOptions &options = ConvertSVGLayerOptions::Default());

/// 将 SVGDOM 中的 Text 节点转为 Layer 树
/// @param dom SVG DOM
std::shared_ptr<tgfx::Layer> convertSVGDomTextNodeToLayer(std::shared_ptr<tgfx::SVGDOM> dom);

// 内部函数
std::shared_ptr<tgfx::Layer> convertSVGNodeToLayer(tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext, const ConvertSVGLayerOptions &options);
std::shared_ptr<tgfx::Layer> convertGroup(const ConvertSVGLayerOptions &options, tgfx::SVGGroup *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatRegionLayer> convertLine(tgfx::SVGLine *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatRegionLayer> convertCircle(tgfx::SVGCircle *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatRegionLayer> convertEllipse(tgfx::SVGEllipse *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatRegionLayer> convertPath(tgfx::SVGPath *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatRegionLayer> convertPoly(tgfx::SVGPoly *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatRegionLayer> convertRect(tgfx::SVGRect *node, const tgfx::SVGLengthContext &lengthContext);
std::shared_ptr<kk::layer::SeatTextLayer> convertText(tgfx::SVGText *node, const tgfx::SVGLengthContext &lengthContext);

};  // namespace kk::svg

#endif /* ConvertSVGLayer_hpp */
