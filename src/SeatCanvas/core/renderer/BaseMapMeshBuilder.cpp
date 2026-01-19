//
//  BaseMapMeshBuilder.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "BaseMapMeshBuilder.hpp"

namespace kk::renderer {

void BaseMapMeshBuilder::addZoneMeshInfo(std::shared_ptr<ZoneMeshInfo> regionInfo) {
    if (!regionInfo || !regionInfo->isValid()) {
        return;
    }

    ZoneMeshInfos.push_back(regionInfo);
    auto vertexCount = static_cast<uint32_t>(regionInfo->fillVertices.size() + regionInfo->strokeVertices.size());
    drawRanges.push_back({vertexOffset, vertexCount});

    assert(ZoneMeshInfos.size() == drawRanges.size());

    totalVertexCount += vertexCount;
    vertexOffset = totalVertexCount;

    if (!regionInfo->zoneId.empty()) {
        zoneIdToMeshInfo[regionInfo->zoneId] = regionInfo;
        zoneIdToIndex[regionInfo->zoneId] = ZoneMeshInfos.size() - 1;
    }
}

std::optional<BaseMapMeshBuilder::RegionDrawRange> BaseMapMeshBuilder::findRegionDrawRangeById(const std::string &zoneId) const {
    if (zoneId.empty()) {
        return std::nullopt;
    }
    auto iter = zoneIdToIndex.find(zoneId);
    if (iter == zoneIdToIndex.end()) {
        return std::nullopt;
    }
    return findRegionDrawRangeByIndex(iter->second);
}

std::optional<BaseMapMeshBuilder::RegionDrawRange> BaseMapMeshBuilder::findRegionDrawRangeByIndex(size_t index) const {
    if (drawRanges.empty() || index >= drawRanges.size()) {
        return std::nullopt;
    }
    return {drawRanges[index]};
}

std::shared_ptr<ZoneMeshInfo> BaseMapMeshBuilder::findRegionById(const std::string &zoneId) const {
    if (zoneId.empty()) {
        return nullptr;
    }

    auto iter = zoneIdToMeshInfo.find(zoneId);
    if (iter != zoneIdToMeshInfo.end()) {
        return iter->second;
    }

    return nullptr;
}

std::vector<std::shared_ptr<ZoneMeshInfo>> BaseMapMeshBuilder::findRegionsIntersectingRect(const tgfx::Rect &rect, std::vector<size_t> *visibleIndices) const {
    std::vector<std::shared_ptr<ZoneMeshInfo>> result;

    if (visibleIndices != nullptr) {
        visibleIndices->clear();
    }

    int32_t index = -1;
    for (const auto &regionInfo : ZoneMeshInfos) {
        ++index;

        if (!regionInfo) {
            continue;
        }
        if (tgfx::Rect::Intersects(rect, regionInfo->fillBounds) || tgfx::Rect::Intersects(rect, regionInfo->strokeBounds)) {
            result.push_back(regionInfo);
            if (visibleIndices != nullptr) {
                visibleIndices->push_back(static_cast<size_t>(index));
            }
        }
    }

    return result;
}

std::shared_ptr<ZoneMeshInfo> BaseMapMeshBuilder::findRegionContainingPoint(const tgfx::Point &point) const {
    // 从后往前查找，返回最上层的区域（最后添加的）
    for (auto iter = ZoneMeshInfos.rbegin(); iter != ZoneMeshInfos.rend(); ++iter) {
        const auto &regionInfo = *iter;
        if (!regionInfo || !regionInfo->path) {
            continue;
        }

        // 1. 先用 bounds 进行粗略过滤（快速剔除）
        if (!regionInfo->fillBounds.contains(point.x, point.y)) {
            continue;
        }

        // 2. 使用 Path 进行精确 hit-test
        if (regionInfo->path->contains(point.x, point.y)) {
            return regionInfo;
        }
    }

    return nullptr;
}

const tgfx::Path *BaseMapMeshBuilder::getRegionPath(const std::string &zoneId) const {
    auto regionInfo = findRegionById(zoneId);
    if (regionInfo && regionInfo->path) {
        return regionInfo->path.get();
    }
    return nullptr;
}

}  // namespace kk::renderer
