//
//  SVGMeshParser.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "SVGMeshParser.hpp"

#include "core/renderer/BaseMapMeshBuilder.hpp"
#include "core/renderer/ZoneMeshInfo.hpp"

#include <core/PathTriangulator.h>

#include <tgfx/core/Path.h>
#include <tgfx/core/PathEffect.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Shape.h>
#include <tgfx/svg/SVGDOM.h>
#include <tgfx/svg/SVGLengthContext.h>
#include <tgfx/svg/node/SVGCircle.h>
#include <tgfx/svg/node/SVGEllipse.h>
#include <tgfx/svg/node/SVGGroup.h>
#include <tgfx/svg/node/SVGLine.h>
#include <tgfx/svg/node/SVGPath.h>
#include <tgfx/svg/node/SVGPoly.h>
#include <tgfx/svg/node/SVGRect.h>

#include <memory>
#include <unordered_set>

namespace kk::svg {

std::unique_ptr<SVGMeshParseResult> SVGMeshParser::parse(std::shared_ptr<tgfx::SVGDOM> dom) {
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

    auto result = std::make_unique<SVGMeshParseResult>();
    result->size = containerSize;
    result->meshBuilder = std::make_shared<kk::renderer::BaseMapMeshBuilder>();

    meshBuilder = result->meshBuilder.get();
    shapeOrders.clear();

    auto &children = rootNode->getChildren();
    for (const auto &child : children) {
        parseNode(child.get(), viewportLengthContext);
    }

    for (auto &item : shapeOrders) {
        auto zoneMeshInfo = processPath(item.second, item.first, viewportLengthContext);
        if (zoneMeshInfo) {
            meshBuilder->addZoneMeshInfo(std::move(zoneMeshInfo));
        }
    }

    meshBuilder = nullptr;
    shapeOrders.clear();

    return result;
}

void SVGMeshParser::parseNode(tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext) {
    if (node == nullptr) {
        return;
    }

    auto tag = node->tag();
    switch (tag) {
        case tgfx::SVGTag::G: {
            parseGroup(static_cast<tgfx::SVGGroup *>(node), lengthContext);
            break;
        }
        case tgfx::SVGTag::Line: {
            parseLine(static_cast<tgfx::SVGLine *>(node), lengthContext);
            break;
        }
        case tgfx::SVGTag::Circle: {
            parseCircle(static_cast<tgfx::SVGCircle *>(node), lengthContext);
            break;
        }
        case tgfx::SVGTag::Ellipse: {
            parseEllipse(static_cast<tgfx::SVGEllipse *>(node), lengthContext);
            break;
        }
        case tgfx::SVGTag::Rect: {
            parseRect(static_cast<tgfx::SVGRect *>(node), lengthContext);
            break;
        }
        case tgfx::SVGTag::Path: {
            parsePath(static_cast<tgfx::SVGPath *>(node), lengthContext);
            break;
        }
        case tgfx::SVGTag::Polygon:
        case tgfx::SVGTag::Polyline: {
            parsePoly(static_cast<tgfx::SVGPoly *>(node), lengthContext);
            break;
        }
        default:
            // 忽略 Text 和其他节点
            break;
    }
}

void SVGMeshParser::parseGroup(tgfx::SVGGroup *node, const tgfx::SVGLengthContext &lengthContext) {
    if (!node->hasChildren()) {
        return;
    }
    for (const auto &child : node->getChildren()) {
        parseNode(child.get(), lengthContext);
    }
}

void SVGMeshParser::parseLine(tgfx::SVGLine *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto x1 = lengthContext.resolve(node->getX1(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto y1 = lengthContext.resolve(node->getY1(), tgfx::SVGLengthContext::LengthType::Vertical);
    const auto x2 = lengthContext.resolve(node->getX2(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto y2 = lengthContext.resolve(node->getY2(), tgfx::SVGLengthContext::LengthType::Vertical);
    tgfx::Path path;
    path.moveTo(x1, y1);
    path.lineTo(x2, y2);
    path.transform(node->getTransform());

    shapeOrders.push_back(std::make_pair(node, std::move(path)));
}

void SVGMeshParser::parseCircle(tgfx::SVGCircle *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto cx = lengthContext.resolve(node->getCx(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto cy = lengthContext.resolve(node->getCy(), tgfx::SVGLengthContext::LengthType::Vertical);
    const auto r = lengthContext.resolve(node->getR(), tgfx::SVGLengthContext::LengthType::Other);

    tgfx::Path path;
    path.addOval(tgfx::Rect::MakeXYWH(cx - r, cy - r, 2 * r, 2 * r));
    path.transform(node->getTransform());

    shapeOrders.push_back(std::make_pair(node, std::move(path)));
}

void SVGMeshParser::parseEllipse(tgfx::SVGEllipse *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto cx = lengthContext.resolve(node->getCx(), tgfx::SVGLengthContext::LengthType::Horizontal);
    const auto cy = lengthContext.resolve(node->getCy(), tgfx::SVGLengthContext::LengthType::Vertical);
    const auto [rx, ry] = lengthContext.resolveOptionalRadii(node->getRx(), node->getRy());

    if (rx <= 0 || ry <= 0) {
        return;
    }

    tgfx::Path path;
    path.addOval(tgfx::Rect::MakeXYWH(cx - rx, cy - ry, rx * 2, ry * 2));
    path.transform(node->getTransform());

    shapeOrders.push_back(std::make_pair(node, std::move(path)));
}

void SVGMeshParser::parseRect(tgfx::SVGRect *node, const tgfx::SVGLengthContext &lengthContext) {
    const auto rect = lengthContext.resolveRect(node->getX(), node->getY(), node->getWidth(), node->getHeight());
    const auto [rx, ry] = lengthContext.resolveOptionalRadii(node->getRx(), node->getRy());

    tgfx::RRect rrect;
    rrect.setRectXY(rect, std::min(rx, rect.width() / 2), std::min(ry, rect.height() / 2));

    tgfx::Path path;
    path.addRRect(rrect);
    path.transform(node->getTransform());

    shapeOrders.push_back(std::make_pair(node, std::move(path)));
}

void SVGMeshParser::parsePath(tgfx::SVGPath *node, const tgfx::SVGLengthContext &lengthContext) {
    (void)lengthContext;

    auto path = node->getShapePath();
    auto clipRule = node->getClipRule().get();
    if (clipRule) {
        path.setFillType(clipRule->asFillType());
    }
    path.transform(node->getTransform());

    shapeOrders.push_back(std::make_pair(node, std::move(path)));
}

void SVGMeshParser::parsePoly(tgfx::SVGPoly *node, const tgfx::SVGLengthContext &lengthContext) {
    (void)lengthContext;

    auto points = node->getPoints();
    if (points.empty()) {
        return;
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

    shapeOrders.push_back(std::make_pair(node, std::move(path)));
}

std::vector<float> SimplifyLineDashPattern(const std::vector<float> &pattern,
                                           const tgfx::Stroke &stroke) {
    // When LineCap is Square, the endpoints extend by half the line width.
    // If an unpainted dash segment is less than or equal to the line width, the painted segments will
    // connect seamlessly. Therefore, such a dash segment can be omitted for simplification.
    if (stroke.cap != tgfx::LineCap::Square) {
        return pattern;
    }
    float addedPaintLength = 0.0f;
    std::vector<float> simplifiedDashes = {};
    for (uint32_t i = 0; i < pattern.size(); i += 2) {
        auto paintedLength = pattern[i];
        auto unpaintedLength = pattern[i + 1];
        if (unpaintedLength <= stroke.width) {
            addedPaintLength += paintedLength + unpaintedLength;
        } else {
            simplifiedDashes.push_back(paintedLength + addedPaintLength);
            simplifiedDashes.push_back(unpaintedLength);
            addedPaintLength = 0.0f;
        }
    }
    return simplifiedDashes;
}

std::shared_ptr<kk::renderer::ZoneMeshInfo> SVGMeshParser::processPath(const tgfx::Path &path, tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext) {
    if (meshBuilder == nullptr) {
        return nullptr;
    }

    auto bounds = path.getBounds();

    std::string zoneId;
    std::unordered_map<std::string, std::string> attributes;

    const auto &customAttributes = node->getCustomAttributes();
    for (const auto &item : customAttributes) {
        if (item.value.empty()) {
            continue;
        }

        if (item.name == "zoneId") {
            zoneId = item.value;
        }
        attributes[item.name] = item.value;
    }

    if (const auto &id = node->getID().get(); id) {
        attributes["id"] = id.value();
    }

    auto zoneMeshInfo = std::make_shared<kk::renderer::ZoneMeshInfo>();
    zoneMeshInfo->zoneId = zoneId;
    zoneMeshInfo->attributes = attributes;
    zoneMeshInfo->fillBounds = bounds;
    zoneMeshInfo->path = std::make_shared<tgfx::Path>(path);

    if (const auto &attribute = node->getFill().get(); attribute && attribute->type() == tgfx::SVGPaint::Type::Color) {
        zoneMeshInfo->fillColor = attribute->color().color();
    }

    std::vector<float> fillTriangleVertices;
    tgfx::PathTriangulator::ToAATriangles(path, bounds, &fillTriangleVertices);
    if (!fillTriangleVertices.empty()) {
        for (size_t idx = 0; idx < fillTriangleVertices.size(); idx += 3) {
            kk::renderer::BaseMapZoneVertex vertex{};
            vertex.x = fillTriangleVertices[idx];
            vertex.y = fillTriangleVertices[idx + 1];
            vertex.coverage = fillTriangleVertices[idx + 2];
            zoneMeshInfo->fillVertices.push_back(vertex);
        }
    }

    tgfx::Stroke stroke{};

    if (const auto &strokeAttr = node->getStroke().get(); strokeAttr && strokeAttr->type() == tgfx::SVGPaint::Type::Color) {
        zoneMeshInfo->strokeColor = strokeAttr->color().color();
    };

    bool hasStrokeWidth = false;
    if (const auto &widthAttr = node->getStrokeWidth().get(); widthAttr) {
        hasStrokeWidth = true;
        stroke.width = lengthContext.resolve(widthAttr.value(), tgfx::SVGLengthContext::LengthType::Horizontal);
    }

    if (!zoneMeshInfo->strokeColor && !hasStrokeWidth) {
        return zoneMeshInfo;
    }

    if (!zoneMeshInfo->strokeColor) {
        // default stroke color
        zoneMeshInfo->strokeColor = tgfx::Color::Black();
    }

    if (const auto &joinAttr = node->getStrokeLineJoin().get(); joinAttr) {
        switch (joinAttr.value().type()) {
            case tgfx::SVGLineJoin::Type::Miter:
                stroke.join = tgfx::LineJoin::Miter;
                break;
            case tgfx::SVGLineJoin::Type::Round:
                stroke.join = tgfx::LineJoin::Round;
                break;
            case tgfx::SVGLineJoin::Type::Bevel:
                stroke.join = tgfx::LineJoin::Bevel;
                break;
            default:
                break;
        }
    }

    if (const auto &miterAttr = node->getStrokeMiterLimit().get(); miterAttr) {
        stroke.miterLimit = miterAttr.value();
    }

    zoneMeshInfo->strokeWidth = stroke.width;

    std::vector<float> dashes{};
    float dashOffset = 0.0f;
    if (const auto &dashAttr = node->getStrokeDashArray().get(); dashAttr) {
        for (const auto &item : dashAttr.value().dashArray()) {
            dashes.push_back(item.value());
        }
        dashes = SimplifyLineDashPattern(dashes, stroke);
    }

    if (const auto &attribute = node->getStrokeDashOffset().get(); attribute) {
        dashOffset = attribute.value().value();
    }

    auto strokeShape = tgfx::Shape::MakeFrom(path);
    if (!dashes.empty()) {
        auto dash = tgfx::PathEffect::MakeDash(dashes.data(), static_cast<int>(dashes.size()),
                                               dashOffset, false);
        strokeShape = tgfx::Shape::ApplyEffect(std::move(strokeShape), std::move(dash));
    }
    strokeShape = tgfx::Shape::ApplyStroke(std::move(strokeShape), &stroke);
    if (!strokeShape) {
        return zoneMeshInfo;
    }

    if (!dashes.empty() || !zoneMeshInfo->fillColor) {
        auto strokePath = strokeShape->getPath();
        zoneMeshInfo->strokeOnTop = true;
        zoneMeshInfo->strokeBounds = strokePath.getBounds();
        std::vector<float> strokeTriangleVertices;
        tgfx::PathTriangulator::ToAATriangles(strokePath, zoneMeshInfo->strokeBounds,
                                              &strokeTriangleVertices);
        if (!strokeTriangleVertices.empty()) {
            for (size_t idx = 0; idx < strokeTriangleVertices.size(); idx += 3) {
                kk::renderer::BaseMapZoneVertex vertex{};
                vertex.x = strokeTriangleVertices[idx];
                vertex.y = strokeTriangleVertices[idx + 1];
                vertex.coverage = strokeTriangleVertices[idx + 2];
                zoneMeshInfo->strokeVertices.push_back(vertex);
            }
        }
    } else {
        tgfx::Path expandedPath = path;
        expandedPath.addPath(strokeShape->getPath(), tgfx::PathOp::Union);
        zoneMeshInfo->strokeOnTop = false;
        zoneMeshInfo->strokeBounds = expandedPath.getBounds();
        std::vector<float> strokeTriangleVertices;
        tgfx::PathTriangulator::ToAATriangles(expandedPath, zoneMeshInfo->strokeBounds,
                                              &strokeTriangleVertices);
        if (!strokeTriangleVertices.empty()) {
            for (size_t idx = 0; idx < strokeTriangleVertices.size(); idx += 3) {
                kk::renderer::BaseMapZoneVertex vertex{};
                vertex.x = strokeTriangleVertices[idx];
                vertex.y = strokeTriangleVertices[idx + 1];
                vertex.coverage = strokeTriangleVertices[idx + 2];
                zoneMeshInfo->strokeVertices.push_back(vertex);
            }
        }
    }

    return zoneMeshInfo;
}
};  // namespace kk::svg
