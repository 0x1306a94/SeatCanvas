//
//  BaseMapMeshBuilder.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "BaseMapMeshBuilder.hpp"

#include "RegionLayerBuilder.hpp"

namespace kk::renderer {

void BaseMapMeshBuilder::addRegionMeshInfo(std::shared_ptr<RegionMeshInfo> regionInfo) {
    if (!regionInfo || !regionInfo->isValid()) {
        return;
    }

    regionMeshInfos.push_back(regionInfo);
    auto vertexCount = static_cast<uint32_t>(regionInfo->fillVertices.size() + regionInfo->strokeVertices.size());
    drawRanges.push_back({vertexOffset, vertexCount});

    assert(regionMeshInfos.size() == drawRanges.size());

    totalVertexCount += vertexCount;
    vertexOffset = totalVertexCount;

    // 建立 regionId 到 RegionMeshInfo 的映射
    if (!regionInfo->regionId.empty()) {
        regionIdToMeshInfo[regionInfo->regionId] = regionInfo;
        regionIdToIndex[regionInfo->regionId] = regionMeshInfos.size() - 1;
    }
}

std::optional<BaseMapMeshBuilder::RegionDrawRange> BaseMapMeshBuilder::findRegionDrawRangeById(const std::string &regionId) const {
    if (regionId.empty()) {
        return std::nullopt;
    }
    auto iter = regionIdToIndex.find(regionId);
    if (iter == regionIdToIndex.end()) {
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

std::shared_ptr<RegionMeshInfo> BaseMapMeshBuilder::findRegionById(const std::string &regionId) const {
    if (regionId.empty()) {
        return nullptr;
    }

    auto iter = regionIdToMeshInfo.find(regionId);
    if (iter != regionIdToMeshInfo.end()) {
        return iter->second;
    }

    return nullptr;
}

std::vector<std::shared_ptr<RegionMeshInfo>> BaseMapMeshBuilder::findRegionsIntersectingRect(const tgfx::Rect &rect, std::vector<size_t> *visibleIndices) const {
    std::vector<std::shared_ptr<RegionMeshInfo>> result;

    if (visibleIndices != nullptr) {
        visibleIndices->clear();
    }

    int32_t index = -1;
    for (const auto &regionInfo : regionMeshInfos) {
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

std::shared_ptr<RegionMeshInfo> BaseMapMeshBuilder::findRegionContainingPoint(const tgfx::Point &point) const {
    // 从后往前查找，返回最上层的区域（最后添加的）
    for (auto iter = regionMeshInfos.rbegin(); iter != regionMeshInfos.rend(); ++iter) {
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

const tgfx::Path *BaseMapMeshBuilder::getRegionPath(const std::string &regionId) const {
    auto regionInfo = findRegionById(regionId);
    if (regionInfo && regionInfo->path) {
        return regionInfo->path.get();
    }
    return nullptr;
}

}  // namespace kk::renderer
