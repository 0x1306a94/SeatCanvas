//
//  SVGMeshParser.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "SVGMeshParser.hpp"

#include "core/RegionInfo.hpp"
#include "core/renderer/BaseMapMeshBuilder.hpp"
#include "core/renderer/RegionMeshInfo.hpp"

#include <core/PathTriangulator.h>

#include <tgfx/core/Path.h>
#include <tgfx/core/Rect.h>
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

    // 解析所有子节点
    auto &children = rootNode->getChildren();
    for (const auto &child : children) {
        parseNode(child.get(), viewportLengthContext);
    }

    //    removeInvisible();

    // 第一步：解析所有路径，创建 RegionMeshInfo
    for (auto &item : shapeOrders) {
        processPath(item.second, item.first, viewportLengthContext);
    }

    // 清理
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

void SVGMeshParser::removeInvisible() {
    if (shapeOrders.empty()) {
        return;
    }

    /*
     * 在实际的业务场景中。对于大型场馆通常也不会有太多的区域。大概在 200 左右。
     * 如果完全不存在覆盖的情况遍历次数为 200 x 200
     * 一旦存在覆盖的情况，那么这里就会被剔除掉。从而避免的后面渲染执行不必要的
     */
    for (int i = static_cast<int>(shapeOrders.size()) - 1; i >= 0; --i) {
        auto &current = shapeOrders[i];
        auto *currentNode = current.first;
        // 复制路径对象，避免在删除元素时引用失效
        const tgfx::Path currentPath = current.second;

        // 1. fill 检查
        bool currentHasFill = true;
        if (const auto &fillAttr = currentNode->getFill().get(); fillAttr) {
            if (fillAttr->type() == tgfx::SVGPaint::Type::None) {
                currentHasFill = false;
            }
        }
        if (!currentHasFill) {
            continue;
        }

        auto currentBounds = currentPath.getBounds();
        if (currentBounds.isEmpty()) {
            shapeOrders.erase(shapeOrders.begin() + i);
            continue;
        }

        // 2. 向前检查「所有」元素
        for (int j = i - 1; j >= 0; --j) {
            const auto &prevPath = shapeOrders[j].second;
            auto prevBounds = prevPath.getBounds();

            if (prevBounds.isEmpty()) {
                shapeOrders.erase(shapeOrders.begin() + j);
                --i;
                continue;
            }

            // 先用 bounds 进行快速过滤（如果 bounds 不包含，肯定不包含）
            if (!currentBounds.contains(prevBounds)) {
                continue;
            }

            // 使用路径采样点进行精确判断，避免相邻异形路径被错误剔除
            // 检查前一个路径边界框的角点和中心点是否都在当前路径内
            bool isFullyContained = true;
            const float left = prevBounds.x();
            const float top = prevBounds.y();
            const float right = prevBounds.x() + prevBounds.width();
            const float bottom = prevBounds.y() + prevBounds.height();
            const float centerX = prevBounds.centerX();
            const float centerY = prevBounds.centerY();

            // 检查四个角点和中心点
            const std::array<tgfx::Point, 5> testPoints = {
                tgfx::Point::Make(left, top),        // 左上角
                tgfx::Point::Make(right, top),       // 右上角
                tgfx::Point::Make(right, bottom),    // 右下角
                tgfx::Point::Make(left, bottom),     // 左下角
                tgfx::Point::Make(centerX, centerY)  // 中心点
            };

            for (const auto &point : testPoints) {
                if (!currentPath.contains(point.x, point.y)) {
                    isFullyContained = false;
                    break;
                }
            }

            // 只有当所有采样点都在当前路径内时，才认为完全包含
            if (isFullyContained) {
                shapeOrders.erase(shapeOrders.begin() + j);
                --i;
            }
        }
    }
}

void SVGMeshParser::processPath(const tgfx::Path &path, tgfx::SVGNode *node, const tgfx::SVGLengthContext &lengthContext) {
    if (meshBuilder == nullptr) {
        return;
    }

    auto bounds = path.getBounds();

    std::string regionId;
    std::unordered_map<std::string, std::string> attributes;

    const auto &customAttributes = node->getCustomAttributes();
    for (const auto &item : customAttributes) {
        if (item.value.empty()) {
            continue;
        }

        if (item.name == "zoneId") {
            regionId = item.value;
        }
        attributes[item.name] = item.value;
    }

    if (const auto &id = node->getID().get(); id) {
        attributes["id"] = id.value();
    }

    // 创建 RegionMeshInfo
    auto regionMeshInfo = std::make_shared<kk::renderer::RegionMeshInfo>();
    regionMeshInfo->regionId = regionId;
    regionMeshInfo->attributes = attributes;
    regionMeshInfo->fillBounds = bounds;

    // 获取填充颜色
    if (const auto &attribute = node->getFill().get(); attribute && attribute->type() == tgfx::SVGPaint::Type::Color) {
        regionMeshInfo->fillColor = attribute->color().color();
    }

    // 使用智能指针保存 Path（避免拷贝）
    regionMeshInfo->path = std::make_shared<tgfx::Path>(path);

    // 三角化 fill 路径
    std::vector<float> fillTriangleVertices;
    tgfx::PathTriangulator::ToAATriangles(path, bounds, &fillTriangleVertices);
    if (!fillTriangleVertices.empty()) {
        // 转换为 BaseMapRegionVertex（暂时不设置 regionIndex，在 build 时设置）
        for (size_t idx = 0; idx < fillTriangleVertices.size(); idx += 3) {
            kk::renderer::BaseMapRegionVertex vertex{};
            vertex.x = fillTriangleVertices[idx];
            vertex.y = fillTriangleVertices[idx + 1];
            vertex.coverage = fillTriangleVertices[idx + 2];
            regionMeshInfo->fillVertices.push_back(vertex);
        }
    }

    regionMeshInfo->strokeColor = tgfx::Color::Black();

    // 获取 stroke 信息
    if (const auto &strokeAttr = node->getStroke().get(); strokeAttr && strokeAttr->type() == tgfx::SVGPaint::Type::Color) {
        tgfx::Stroke stroke{};
        regionMeshInfo->strokeColor = strokeAttr->color().color();

        // stroke-width
        if (const auto &widthAttr = node->getStrokeWidth().get(); widthAttr) {
            stroke.width = lengthContext.resolve(widthAttr.value(), tgfx::SVGLengthContext::LengthType::Horizontal);
        }

        // stroke-linecap
        if (const auto &capAttr = node->getStrokeLineCap().get(); capAttr) {
            switch (capAttr.value()) {
                case tgfx::SVGLineCap::Butt:
                    stroke.cap = tgfx::LineCap::Butt;
                    break;
                case tgfx::SVGLineCap::Round:
                    stroke.cap = tgfx::LineCap::Round;
                    break;
                case tgfx::SVGLineCap::Square:
                    stroke.cap = tgfx::LineCap::Square;
                    break;
                default:
                    break;
            }
        }

        // stroke-linejoin
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

        // stroke-miterlimit
        if (const auto &miterAttr = node->getStrokeMiterLimit().get(); miterAttr) {
            stroke.miterLimit = miterAttr.value();
        }

        regionMeshInfo->strokeWidth = stroke.width;

        // 只有 width > 0 才处理 stroke
        if (stroke.width > 0.0f) {
            // 三角化 stroke 路径
            tgfx::Path strokePath = path;  // 复制 path，因为 applyToPath 会修改它
            if (stroke.applyToPath(&strokePath)) {
                regionMeshInfo->strokeBounds = strokePath.getBounds();
                std::vector<float> strokeTriangleVertices;
                tgfx::PathTriangulator::ToAATriangles(strokePath, regionMeshInfo->strokeBounds, &strokeTriangleVertices);
                if (!strokeTriangleVertices.empty()) {
                    for (size_t idx = 0; idx < strokeTriangleVertices.size(); idx += 3) {
                        kk::renderer::BaseMapRegionVertex vertex{};
                        vertex.x = strokeTriangleVertices[idx];
                        vertex.y = strokeTriangleVertices[idx + 1];
                        vertex.coverage = strokeTriangleVertices[idx + 2];
                        regionMeshInfo->strokeVertices.push_back(vertex);
                    }
                }
            }
        }
    }

    // 添加到构建器
    meshBuilder->addRegionMeshInfo(regionMeshInfo);
}

};  // namespace kk::svg
