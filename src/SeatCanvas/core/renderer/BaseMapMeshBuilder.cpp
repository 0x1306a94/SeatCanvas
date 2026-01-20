//
//  BaseMapMeshBuilder.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "BaseMapMeshBuilder.hpp"

namespace kk::renderer {

void BaseMapMeshBuilder::addZoneMeshInfo(std::shared_ptr<ZoneMeshInfo> zoneInfo) {
    if (!zoneInfo || !zoneInfo->isValid()) {
        return;
    }

    zoneMeshInfos.push_back(zoneInfo);
    auto vertexCount = static_cast<uint32_t>(zoneInfo->fillVertices.size() + zoneInfo->strokeVertices.size());
    drawRanges.push_back({vertexOffset, vertexCount});

    assert(zoneMeshInfos.size() == drawRanges.size());

    totalVertexCount += vertexCount;
    vertexOffset = totalVertexCount;

    if (!zoneInfo->zoneId.empty()) {
        zoneIdToMeshInfo[zoneInfo->zoneId] = zoneInfo;
        zoneIdToIndex[zoneInfo->zoneId] = zoneMeshInfos.size() - 1;
    }
}

std::optional<BaseMapMeshBuilder::ZoneDrawRange> BaseMapMeshBuilder::findZoneDrawRangeById(const std::string &zoneId) const {
    if (zoneId.empty()) {
        return std::nullopt;
    }
    auto iter = zoneIdToIndex.find(zoneId);
    if (iter == zoneIdToIndex.end()) {
        return std::nullopt;
    }
    return findZoneDrawRangeByIndex(iter->second);
}

std::optional<BaseMapMeshBuilder::ZoneDrawRange> BaseMapMeshBuilder::findZoneDrawRangeByIndex(size_t index) const {
    if (drawRanges.empty() || index >= drawRanges.size()) {
        return std::nullopt;
    }
    return {drawRanges[index]};
}

std::shared_ptr<ZoneMeshInfo> BaseMapMeshBuilder::findZoneById(const std::string &zoneId) const {
    if (zoneId.empty()) {
        return nullptr;
    }

    auto iter = zoneIdToMeshInfo.find(zoneId);
    if (iter != zoneIdToMeshInfo.end()) {
        return iter->second;
    }

    return nullptr;
}

std::vector<std::shared_ptr<ZoneMeshInfo>> BaseMapMeshBuilder::findZoneIntersectingRect(const tgfx::Rect &rect, std::vector<size_t> *visibleIndices) const {
    std::vector<std::shared_ptr<ZoneMeshInfo>> result;

    if (visibleIndices != nullptr) {
        visibleIndices->clear();
    }

    int32_t index = -1;
    for (const auto &zoneInfo : zoneMeshInfos) {
        ++index;

        if (!zoneInfo) {
            continue;
        }
        if (tgfx::Rect::Intersects(rect, zoneInfo->fillBounds) || tgfx::Rect::Intersects(rect, zoneInfo->strokeBounds)) {
            result.push_back(zoneInfo);
            if (visibleIndices != nullptr) {
                visibleIndices->push_back(static_cast<size_t>(index));
            }
        }
    }

    return result;
}

std::shared_ptr<ZoneMeshInfo> BaseMapMeshBuilder::findZoneContainingPoint(const tgfx::Point &point) const {
    // 从后往前查找，返回最上层的区域（最后添加的）
    for (auto iter = zoneMeshInfos.rbegin(); iter != zoneMeshInfos.rend(); ++iter) {
        const auto &zoneInfo = *iter;
        if (!zoneInfo || !zoneInfo->path) {
            continue;
        }

        // 1. 先用 bounds 进行粗略过滤（快速剔除）
        if (!zoneInfo->fillBounds.contains(point.x, point.y)) {
            continue;
        }

        // 2. 使用 Path 进行精确 hit-test
        if (zoneInfo->path->contains(point.x, point.y)) {
            return zoneInfo;
        }
    }

    return nullptr;
}

}  // namespace kk::renderer
