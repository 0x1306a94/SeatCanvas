//
//  ConvertSVGLayer.cpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#include "ConvertSVGLayer.hpp"

#include "core/FontManager.hpp"
#include "core/layers/BaseMapRootLayer.hpp"
#include "core/layers/SeatTextLayer.hpp"
#include "core/layers/SeatZoneLayer.hpp"

#include <tgfx/core/Path.h>
#include <tgfx/core/Rect.h>
#include <tgfx/layers/Layer.h>
#include <tgfx/layers/ShapeLayer.h>
#include <tgfx/svg/SVGDOM.h>
#include <tgfx/svg/SVGLengthContext.h>
#include <tgfx/svg/node/SVGCircle.h>
#include <tgfx/svg/node/SVGEllipse.h>
#include <tgfx/svg/node/SVGGroup.h>
#include <tgfx/svg/node/SVGLine.h>
#include <tgfx/svg/node/SVGPath.h>
#include <tgfx/svg/node/SVGPoly.h>
#include <tgfx/svg/node/SVGRect.h>
#include <tgfx/svg/node/SVGText.h>

namespace kk::svg {

// ========== 辅助函数 ==========

static std::shared_ptr<tgfx::Layer> buildTextLayerTreeFromNode(tgfx::SVGNode *node,
                                                               const tgfx::SVGLengthContext &lengthContext);

struct TextRun {
    std::string text;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

static std::shared_ptr<tgfx::Typeface> resolveTypeface(const tgfx::SVGText *node, const tgfx::SVGLengthContext &lengthContext) {
    using namespace tgfx;
    auto weight = [](const SVGFontWeight &w) {
        switch (w.type()) {
            case SVGFontWeight::Type::W100:
                return FontWeight::Thin;
            case SVGFontWeight::Type::W200:
                return FontWeight::ExtraLight;
            case SVGFontWeight::Type::W300:
                return FontWeight::Light;
            case SVGFontWeight::Type::W400:
                return FontWeight::Normal;
            case SVGFontWeight::Type::W500:
                return FontWeight::Medium;
            case SVGFontWeight::Type::W600:
                return FontWeight::SemiBold;
            case SVGFontWeight::Type::W700:
                return FontWeight::Bold;
            case SVGFontWeight::Type::W800:
                return FontWeight::ExtraBold;
            case SVGFontWeight::Type::W900:
                return FontWeight::Black;
            case SVGFontWeight::Type::Normal:
                return FontWeight::Normal;
            case SVGFontWeight::Type::Bold:
                return FontWeight::Bold;
            case SVGFontWeight::Type::Bolder:
                return FontWeight::ExtraBold;
            case SVGFontWeight::Type::Lighter:
                return FontWeight::Light;
            case SVGFontWeight::Type::Inherit:
                return FontWeight::Normal;
        }
    };

    auto slant = [](const SVGFontStyle &s) {
        switch (s.type()) {
            case SVGFontStyle::Type::Normal:
                return FontSlant::Upright;
            case SVGFontStyle::Type::Italic:
                return FontSlant::Italic;
            case SVGFontStyle::Type::Oblique:
                return FontSlant::Oblique;
            case SVGFontStyle::Type::Inherit:
                return FontSlant::Upright;
        }
    };

    auto fontFamily = node->getFontFamily().get();
    if (!fontFamily) {
        return nullptr;
    }
    const auto &family = fontFamily->family();
    const auto fontWeight = node->getFontWeight().get().value_or(SVGFontWeight(SVGFontWeight::Type::Normal));
    const auto fontStyle = node->getFontStyle().get().value_or(SVGFontStyle(SVGFontStyle::Type::Normal));

    const FontStyle style(weight(fontWeight), FontWidth::Normal, slant(fontStyle));
    return Typeface::MakeFromName(family, style);
}

static std::vector<float> resolveTextLengths(const tgfx::SVGLengthContext &lengthContext,
                                             const std::vector<tgfx::SVGLength> &lengths,
                                             tgfx::SVGLengthContext::LengthType lengthType,
                                             const tgfx::SVGFontSize &fontSize) {
    std::vector<float> resolved;
    resolved.reserve(lengths.size());

    for (const auto &length : lengths) {
        if (length.unit() == tgfx::SVGLength::Unit::EMS || length.unit() == tgfx::SVGLength::Unit::EXS) {
            auto resolvedLength =
                lengthContext.resolve(fontSize.size(), tgfx::SVGLengthContext::LengthType::Horizontal) *
                length.value();
            resolved.push_back(resolvedLength);
        } else {
            resolved.push_back(lengthContext.resolve(length, lengthType));
        }
    }

    return resolved;
}

static void collectTextRuns(tgfx::SVGTextContainer *container,
                            tgfx::SVGText *rootText,
                            const tgfx::SVGLengthContext &lengthContext,
                            float offsetX,
                            float offsetY,
                            std::vector<TextRun> &runs) {
    auto fontSize = tgfx::SVGFontSize(tgfx::SVGLength(10.0f, tgfx::SVGLength::Unit::PT));
    if (auto attr = rootText->getFontSize().get(); attr) {
        fontSize = attr.value();
    }

    auto x = resolveTextLengths(lengthContext, container->getX(), tgfx::SVGLengthContext::LengthType::Horizontal, fontSize);
    auto y = resolveTextLengths(lengthContext, container->getY(), tgfx::SVGLengthContext::LengthType::Vertical, fontSize);
    auto dx = resolveTextLengths(lengthContext, container->getDx(), tgfx::SVGLengthContext::LengthType::Horizontal, fontSize);
    auto dy = resolveTextLengths(lengthContext, container->getDy(), tgfx::SVGLengthContext::LengthType::Vertical, fontSize);

    const auto &children = container->getTextChildren();
    for (size_t i = 0; i < children.size(); i++) {
        const auto &child = children[i];
        float runX = offsetX + (i < x.size() ? x[i] : 0.0f) + (i < dx.size() ? dx[i] : 0.0f);
        float runY = offsetY + (i < y.size() ? y[i] : 0.0f) + (i < dy.size() ? dy[i] : 0.0f);

        if (child->tag() == tgfx::SVGTag::TextLiteral) {
            auto literal = std::static_pointer_cast<tgfx::SVGTextLiteral>(child);
            std::string text = literal->getText();
            if (!text.empty()) {
                runs.push_back({std::move(text), runX, runY});
            }
        } else if (child->tag() == tgfx::SVGTag::TSpan) {
            collectTextRuns(static_cast<tgfx::SVGTextContainer *>(child.get()), rootText, lengthContext, runX, runY, runs);
        }
    }
}

static void applyShapeLayerStyle(tgfx::ShapeLayer *shape, tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext) {
    auto hasStroke = false;
    if (const auto &attribute = node->getStroke().get(); attribute && attribute->type() == tgfx::SVGPaint::Type::Color) {
        auto color = attribute->color().color();
        shape->addStrokeStyle(tgfx::ShapeStyle::Make(color));
        hasStroke = true;
    }

    if (const auto &attribute = node->getStrokeWidth().get(); attribute && hasStroke) {
        auto width = attribute.value().value();
        hasStroke = !(width == 0.0f);
        shape->setLineWidth(width);
    }

    if (const auto &attribute = node->getFill().get(); attribute && attribute->type() == tgfx::SVGPaint::Type::Color) {
        auto color = attribute->color().color();
        shape->addFillStyle(tgfx::ShapeStyle::Make(color));
    } else if (!hasStroke) {
        shape->addFillStyle(tgfx::ShapeStyle::Make(tgfx::Color::Black()));
    }

    if (const auto &attribute = node->getStrokeDashArray().get(); attribute) {
        std::vector<float> dash{};
        for (const auto &item : attribute.value().dashArray()) {
            dash.push_back(item.value());
        }
        shape->setLineDashPattern(dash);
    }

    if (const auto &attribute = node->getStrokeDashOffset().get(); attribute) {
        shape->setLineDashPhase(attribute.value().value());
    }

    if (const auto &attribute = node->getStrokeLineCap().get(); attribute) {
        switch (attribute.value()) {
            case tgfx::SVGLineCap::Butt:
                shape->setLineCap(tgfx::LineCap::Butt);
                break;
            case tgfx::SVGLineCap::Round:
                shape->setLineCap(tgfx::LineCap::Round);
                break;
            case tgfx::SVGLineCap::Square:
                shape->setLineCap(tgfx::LineCap::Square);
                break;
            default:
                break;
        }
    }

    if (const auto &attribute = node->getStrokeLineJoin().get(); attribute) {
        switch (attribute.value().type()) {
            case tgfx::SVGLineJoin::Type::Miter:
                shape->setLineJoin(tgfx::LineJoin::Miter);
                break;
            case tgfx::SVGLineJoin::Type::Round:
                shape->setLineJoin(tgfx::LineJoin::Round);
                break;
            case tgfx::SVGLineJoin::Type::Bevel:
                shape->setLineJoin(tgfx::LineJoin::Bevel);
                break;
            default:
                break;
        }
    }

    if (const auto &attribute = node->getStrokeMiterLimit().get(); attribute) {
        shape->setMiterLimit(attribute.value());
    }
}

static void applyTextLayerStyle(kk::layer::SeatTextLayer *textLayer, tgfx::SVGText *node, const tgfx::SVGLengthContext &lengthContext) {
    auto hasStroke = false;
    if (const auto &attribute = node->getStroke().get(); attribute && attribute->type() == tgfx::SVGPaint::Type::Color) {
        auto color = attribute->color().color();
        textLayer->setStrokeColor(color);
        hasStroke = true;
    }

    if (const auto &attribute = node->getStrokeWidth().get(); attribute && hasStroke) {
        auto width = attribute.value().value();
        hasStroke = !(width == 0.0f);
        textLayer->setLineWidth(attribute.value().value());
    }

    if (const auto &attribute = node->getFill().get(); attribute && attribute->type() == tgfx::SVGPaint::Type::Color) {
        auto color = attribute->color().color();
        textLayer->setTextColor(color);
    } else if (!hasStroke) {
        textLayer->setTextColor(tgfx::Color::Black());
    }

    if (const auto &attribute = node->getTextAnchor().get(); attribute) {
        auto type = attribute.value().type();
        switch (type) {
            case tgfx::SVGTextAnchor::Type::Start:
                textLayer->setTextAlign(tgfx::TextAlign::Start);
                break;
            case tgfx::SVGTextAnchor::Type::Middle:
                textLayer->setTextAlign(tgfx::TextAlign::Center);
                break;
            case tgfx::SVGTextAnchor::Type::End:
                textLayer->setTextAlign(tgfx::TextAlign::End);
                break;
            default:
                break;
        }
    }

    if (const auto &attribute = node->getStrokeLineCap().get(); attribute) {
        switch (attribute.value()) {
            case tgfx::SVGLineCap::Butt:
                textLayer->setLineCap(tgfx::LineCap::Butt);
                break;
            case tgfx::SVGLineCap::Round:
                textLayer->setLineCap(tgfx::LineCap::Round);
                break;
            case tgfx::SVGLineCap::Square:
                textLayer->setLineCap(tgfx::LineCap::Square);
                break;
            default:
                break;
        }
    }

    if (const auto &attribute = node->getStrokeLineJoin().get(); attribute) {
        switch (attribute.value().type()) {
            case tgfx::SVGLineJoin::Type::Miter:
                textLayer->setLineJoin(tgfx::LineJoin::Miter);
                break;
            case tgfx::SVGLineJoin::Type::Round:
                textLayer->setLineJoin(tgfx::LineJoin::Round);
                break;
            case tgfx::SVGLineJoin::Type::Bevel:
                textLayer->setLineJoin(tgfx::LineJoin::Bevel);
                break;
            default:
                break;
        }
    }

    if (const auto &attribute = node->getStrokeMiterLimit().get(); attribute) {
        textLayer->setMiterLimit(attribute.value());
    }
}

// ========== 公开函数实现 ==========

std::unique_ptr<ConvertSVGLayerResult> convertSVGDomToLayer(std::shared_ptr<tgfx::SVGDOM> dom, const ConvertSVGLayerOptions &options) {
    if (dom == nullptr) {
        return nullptr;
    }

    auto &rootNode = dom->getRoot();
    if (!rootNode->hasChildren()) {
        return nullptr;
    }

    auto rootWidth = rootNode->getWidth();
    auto rootHeight = rootNode->getHeight();

    tgfx::SVGLengthContext viewportLengthContext(tgfx::Size::Make(100, 100));
    tgfx::Size containerSize{};

    if (rootNode->getViewBox().has_value()) {
        viewportLengthContext = tgfx::SVGLengthContext(rootNode->getViewBox()->size());
        containerSize = tgfx::Size::Make(
            viewportLengthContext.resolve(rootWidth, tgfx::SVGLengthContext::LengthType::Horizontal),
            viewportLengthContext.resolve(rootHeight, tgfx::SVGLengthContext::LengthType::Vertical));
    } else {
        containerSize = tgfx::Size::Make(
            viewportLengthContext.resolve(rootWidth, tgfx::SVGLengthContext::LengthType::Horizontal),
            viewportLengthContext.resolve(rootHeight, tgfx::SVGLengthContext::LengthType::Vertical));
    }

    tgfx::Path path;
    path.addRect(tgfx::Rect::MakeWH(containerSize.width, containerSize.height));

    auto container = kk::layer::BaseMapRootLayer::Make();
    container->setPath(path);

    auto &childrens = rootNode->getChildren();
    for (const auto &child : childrens) {
        auto layer = convertSVGNodeToLayer(child.get(), viewportLengthContext, options);
        if (layer) {
            container->addChild(layer);
        }
    }

    return std::make_unique<ConvertSVGLayerResult>(std::move(container), containerSize);
}

std::shared_ptr<tgfx::Layer> convertSVGDomTextNodeToLayer(std::shared_ptr<tgfx::SVGDOM> dom) {
    if (dom == nullptr) {
        return nullptr;
    }

    auto &rootNode = dom->getRoot();
    if (!rootNode->hasChildren()) {
        return nullptr;
    }

    auto rootWidth = rootNode->getWidth();
    auto rootHeight = rootNode->getHeight();

    tgfx::SVGLengthContext viewportLengthContext(tgfx::Size::Make(100, 100));
    tgfx::Size containerSize{};

    if (rootNode->getViewBox().has_value()) {
        viewportLengthContext = tgfx::SVGLengthContext(rootNode->getViewBox()->size());
        containerSize = tgfx::Size::Make(
            viewportLengthContext.resolve(rootWidth, tgfx::SVGLengthContext::LengthType::Horizontal),
            viewportLengthContext.resolve(rootHeight, tgfx::SVGLengthContext::LengthType::Vertical));
    } else {
        containerSize = tgfx::Size::Make(
            viewportLengthContext.resolve(rootWidth, tgfx::SVGLengthContext::LengthType::Horizontal),
            viewportLengthContext.resolve(rootHeight, tgfx::SVGLengthContext::LengthType::Vertical));
    }

    tgfx::Path path;
    path.addRect(tgfx::Rect::MakeWH(containerSize.width, containerSize.height));

    auto container = kk::layer::BaseMapRootLayer::Make();
    container->setPath(path);

    auto &childrens = rootNode->getChildren();
    for (const auto &child : childrens) {
        auto layer = buildTextLayerTreeFromNode(child.get(), viewportLengthContext);
        if (layer != nullptr) {
            container->addChild(layer);
        }
    }

    return container;
}

std::shared_ptr<tgfx::Layer> convertSVGNodeToLayer(tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext, const ConvertSVGLayerOptions &options) {
    auto tag = node->tag();
    switch (tag) {
        case tgfx::SVGTag::G:
            return convertGroup(options, static_cast<tgfx::SVGGroup *>(node), lengthContext);
        case tgfx::SVGTag::Line:
            return convertLine(static_cast<tgfx::SVGLine *>(node), lengthContext);
        case tgfx::SVGTag::Circle:
            return convertCircle(static_cast<tgfx::SVGCircle *>(node), lengthContext);
        case tgfx::SVGTag::Ellipse:
            return convertEllipse(static_cast<tgfx::SVGEllipse *>(node), lengthContext);
        case tgfx::SVGTag::Rect:
            return convertRect(static_cast<tgfx::SVGRect *>(node), lengthContext);
        case tgfx::SVGTag::Path:
            return convertPath(static_cast<tgfx::SVGPath *>(node), lengthContext);
        case tgfx::SVGTag::Polygon:
        case tgfx::SVGTag::Polyline:
            return convertPoly(static_cast<tgfx::SVGPoly *>(node), lengthContext);
        case tgfx::SVGTag::Text:
            if (!options.supportText) {
                return nullptr;
            }
            return convertText(static_cast<tgfx::SVGText *>(node), lengthContext);
        default:
            break;
    }
    return nullptr;
}

static std::shared_ptr<tgfx::Layer> buildTextLayerTreeFromNode(tgfx::SVGNode *node,
                                                               const tgfx::SVGLengthContext &lengthContext) {
    if (node == nullptr) {
        return nullptr;
    }

    if (node->tag() == tgfx::SVGTag::Text) {
        return convertText(static_cast<tgfx::SVGText *>(node), lengthContext);
    }

    if (node->tag() == tgfx::SVGTag::G) {
        auto group = static_cast<tgfx::SVGGroup *>(node);
        if (!group->hasChildren()) {
            return nullptr;
        }

        auto groupLayer = tgfx::Layer::Make();
        groupLayer->setMatrix(group->getTransform());

        for (const auto &child : group->getChildren()) {
            auto childLayer = buildTextLayerTreeFromNode(child.get(), lengthContext);
            if (childLayer != nullptr) {
                groupLayer->addChild(childLayer);
            }
        }

        if (groupLayer->children().empty()) {
            return nullptr;
        }

        return groupLayer;
    }

    return nullptr;
}

std::shared_ptr<tgfx::Layer> convertGroup(const ConvertSVGLayerOptions &options, tgfx::SVGGroup *node, const tgfx::SVGLengthContext &lengthContext) {
    if (!node->hasChildren()) {
        return nullptr;
    }
    auto groupLayer = tgfx::Layer::Make();
    groupLayer->setMatrix(node->getTransform());
    for (auto const &child : node->getChildren()) {
        auto layer = convertSVGNodeToLayer(child.get(), lengthContext, options);
        if (layer) {
            groupLayer->addChild(layer);
        }
    }
    return groupLayer;
}

std::shared_ptr<kk::layer::SeatZoneLayer> convertLine(tgfx::SVGLine *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto x1 = lengthContext.resolve(node->getX1(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto y1 = lengthContext.resolve(node->getY1(), tgfx::SVGLengthContext::LengthType::Vertical);
    const auto x2 = lengthContext.resolve(node->getX2(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto y2 = lengthContext.resolve(node->getY2(), tgfx::SVGLengthContext::LengthType::Vertical);
    tgfx::Path path;
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    path.transform(node->getTransform());

    auto shape = kk::layer::SeatZoneLayer::Make();
    shape->setPath(path);
    applyShapeLayerStyle(shape.get(), node, lengthContext);
    return shape;
}

std::shared_ptr<kk::layer::SeatZoneLayer> convertCircle(tgfx::SVGCircle *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto cx = lengthContext.resolve(node->getCx(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto cy = lengthContext.resolve(node->getCy(), tgfx::SVGLengthContext::LengthType::Vertical);
    const auto r = lengthContext.resolve(node->getR(), tgfx::SVGLengthContext::LengthType::Other);

    tgfx::Path path;
    path.addOval(tgfx::Rect::MakeXYWH(cx - r, cy - r, 2 * r, 2 * r));
    path.transform(node->getTransform());

    auto shape = kk::layer::SeatZoneLayer::Make();
    shape->setPath(path);
    applyShapeLayerStyle(shape.get(), node, lengthContext);
    return shape;
}

std::shared_ptr<kk::layer::SeatZoneLayer> convertEllipse(tgfx::SVGEllipse *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto cx = lengthContext.resolve(node->getCx(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto cy = lengthContext.resolve(node->getCy(), tgfx::SVGLengthContext::LengthType::Vertical);
    const auto [rx, ry] = lengthContext.resolveOptionalRadii(node->getRx(), node->getRy());

    if (rx <= 0 || ry <= 0) {
        return nullptr;
    }

    tgfx::Path path;
    path.addOval(tgfx::Rect::MakeXYWH(cx - rx, cy - ry, rx * 2, ry * 2));
    path.transform(node->getTransform());

    auto shape = kk::layer::SeatZoneLayer::Make();
    shape->setPath(path);
    applyShapeLayerStyle(shape.get(), node, lengthContext);
    return shape;
}

std::shared_ptr<kk::layer::SeatZoneLayer> convertPath(tgfx::SVGPath *node, const tgfx::SVGLengthContext &lengthContext) {
    auto shape = kk::layer::SeatZoneLayer::Make();

    auto path = node->getShapePath();
    auto clipRule = node->getClipRule().get();
    if (clipRule) {
        path.setFillType(clipRule->asFillType());
    }
    shape->setPath(std::move(path));
    shape->setMatrix(node->getTransform());
    applyShapeLayerStyle(shape.get(), node, lengthContext);
    return shape;
}

std::shared_ptr<kk::layer::SeatZoneLayer> convertPoly(tgfx::SVGPoly *node, const tgfx::SVGLengthContext &lengthContext) {
    auto points = node->getPoints();
    if (points.empty()) {
        return nullptr;
    }

    tgfx::Path path;
    path.moveTo(points[0]);
    for (uint32_t i = 1; i < points.size(); i++) {
        path.lineTo(points[i]);
    }
    path.close();
    path.transform(node->getTransform());

    auto clipRule = node->getClipRule().get();
    if (clipRule) {
        path.setFillType(clipRule->asFillType());
    }

    auto shape = kk::layer::SeatZoneLayer::Make();
    shape->setPath(path);
    applyShapeLayerStyle(shape.get(), node, lengthContext);
    return shape;
}

std::shared_ptr<kk::layer::SeatZoneLayer> convertRect(tgfx::SVGRect *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto rect = lengthContext.resolveRect(node->getX(), node->getY(), node->getWidth(), node->getHeight());
    const auto [rx, ry] = lengthContext.resolveOptionalRadii(node->getRx(), node->getRy());

    tgfx::RRect rrect;
    rrect.setRectXY(rect, std::min(rx, rect.width() / 2), std::min(ry, rect.height() / 2));

    tgfx::Path path;
    path.addRRect(rrect);
    path.transform(node->getTransform());

    auto shape = kk::layer::SeatZoneLayer::Make();
    shape->setPath(path);
    applyShapeLayerStyle(shape.get(), node, lengthContext);
    return shape;
}

std::shared_ptr<tgfx::Layer> convertText(tgfx::SVGText *node, const tgfx::SVGLengthContext &lengthContext) {
    auto fallbackTypefaces = FontManager::GetFallbackTypefaces();
    auto shaper = tgfx::TextShaper::Make(std::move(fallbackTypefaces));
    if (shaper == nullptr) {
        return nullptr;
    }

    std::vector<TextRun> runs;
    collectTextRuns(node, node, lengthContext, 0.0f, 0.0f, runs);
    if (runs.empty()) {
        return nullptr;
    }

    auto typeface = resolveTypeface(node, lengthContext);
    auto fontSize = tgfx::SVGFontSize(tgfx::SVGLength(10.0f, tgfx::SVGLength::Unit::PT));
    if (auto attr = node->getFontSize().get(); attr) {
        fontSize = attr.value();
    }
    auto finalFontSize = lengthContext.resolve(fontSize.size(), tgfx::SVGLengthContext::LengthType::Vertical);

    std::shared_ptr<tgfx::Layer> result;
    for (size_t i = 0; i < runs.size(); i++) {
        const auto &run = runs[i];
        auto textBlob = shaper->shape(run.text, typeface, finalFontSize);
        if (textBlob == nullptr) {
            continue;
        }
        auto layer = kk::layer::SeatTextLayer::Make();
#if DEBUG
        layer->setName(run.text);
#endif
        layer->setTextBlob(textBlob);
        applyTextLayerStyle(layer.get(), node, lengthContext);
        auto matrix = tgfx::Matrix::MakeTrans(run.offsetX, run.offsetY);
        matrix.postConcat(node->getTransform());
        layer->setMatrix(matrix);

        if (i == 0) {
            result = std::move(layer);
        } else {
            if (i == 1) {
                auto group = tgfx::Layer::Make();
                group->addChild(result);
                result = std::move(group);
            }
            result->addChild(layer);
        }
    }
    return result;
}

};  // namespace kk::svg
