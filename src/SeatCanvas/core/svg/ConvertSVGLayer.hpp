//
//  ConvertSVGLayer.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef ConvertSVGLayer_hpp
#define ConvertSVGLayer_hpp

#include <memory>
#include <optional>
#include <tuple>

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
class ShapeLayer;
};  // namespace tgfx

namespace kk {
class BaseMapLayerManager;
};  // namespace kk

namespace kk::layer {
class BaseMapRootLayer;
class SeatRegionLayer;
class SeatTextLayer;
};  // namespace kk::layer

namespace kk::svg {

struct ConvertSVGLayerOptions {
    bool collectRegionInfo{false};
    bool supportText{false};
    static const ConvertSVGLayerOptions &Default() {
        static const ConvertSVGLayerOptions options{};
        return options;
    }
};

struct ConvertSVGLayerResult {
    std::shared_ptr<kk::layer::BaseMapRootLayer> layer;
    tgfx::Size size;
    std::shared_ptr<kk::BaseMapLayerManager> layerManager;
    explicit ConvertSVGLayerResult(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &size, std::shared_ptr<kk::BaseMapLayerManager> layerManager)
        : layer(std::move(layer))
        , size(size)
        , layerManager(std::move(layerManager)) {
    }
};

/// 将 SVGDOM 转为 Layer 树
/// - Parameter dom: svg dom
/// - Parameter options: 转换选项
std::unique_ptr<ConvertSVGLayerResult> convertSVGDomToLayer(std::shared_ptr<tgfx::SVGDOM> dom, const ConvertSVGLayerOptions &options = ConvertSVGLayerOptions::Default());

std::shared_ptr<tgfx::Layer> convertSVGNodeToLayer(tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager, const ConvertSVGLayerOptions &options);
std::shared_ptr<tgfx::Layer> convertGroup(const ConvertSVGLayerOptions &options, tgfx::SVGGroup *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatRegionLayer> convertLine(tgfx::SVGLine *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatRegionLayer> convertCircle(tgfx::SVGCircle *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatRegionLayer> convertEllipse(tgfx::SVGEllipse *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatRegionLayer> convertPath(tgfx::SVGPath *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatRegionLayer> convertPoly(tgfx::SVGPoly *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatRegionLayer> convertRect(tgfx::SVGRect *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
std::shared_ptr<kk::layer::SeatTextLayer> convertText(tgfx::SVGText *node, const tgfx::SVGLengthContext &lengthContext, kk::BaseMapLayerManager *layerManager);
};  // namespace kk::svg

#endif /* ConvertSVGLayer_hpp */
