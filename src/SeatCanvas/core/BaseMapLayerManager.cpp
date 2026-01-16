//
//  BaseMapLayerManager.cpp
//  SeatCanvas
//
//  Created by KK on 2025/11/15.
//

#include "BaseMapLayerManager.hpp"

#include "core/layers/SeatRegionLayer.hpp"
#include "core/layers/SeatTextLayer.hpp"

namespace kk {

void BaseMapLayerManager::addRegion(const RegionInfo &region) {
    if (region.regionId.empty()) {
        // 如果没有 regionId，直接添加到列表
        _regions.push_back(region);
        return;
    }

    // 检查是否已存在相同的 regionId
    auto iter = _regionIdIndex.find(region.regionId);
    if (iter != _regionIdIndex.end()) {
        // 如果已存在，更新区域信息
        _regions[iter->second] = region;
    } else {
        // 如果不存在，添加新区域
        size_t index = _regions.size();
        _regions.push_back(region);
        _regionIdIndex[region.regionId] = index;
    }
}

const RegionInfo *BaseMapLayerManager::findRegionById(const std::string &regionId) const {
    if (regionId.empty()) {
        return nullptr;
    }

    auto iter = _regionIdIndex.find(regionId);
    if (iter != _regionIdIndex.end()) {
        return &_regions[iter->second];
    }

    return nullptr;
}

std::vector<const RegionInfo *> BaseMapLayerManager::findRegionsIntersectingRect(const tgfx::Rect &rect) const {
    std::vector<const RegionInfo *> result;

    for (const auto &region : _regions) {
        if (tgfx::Rect::Intersects(region.bounds, rect)) {
            result.push_back(&region);
        }
    }

    return result;
}

const RegionInfo *BaseMapLayerManager::findRegionContainingPoint(const tgfx::Point &point) const {
    // 从后往前查找，返回最上层的区域（最后添加的）
    for (auto iter = _regions.rbegin(); iter != _regions.rend(); ++iter) {
        if (iter->bounds.contains(point.x, point.y)) {
            return &(*iter);
        }
    }

    return nullptr;
}

void BaseMapLayerManager::addRegionLayer(const std::string &regionId, std::shared_ptr<kk::layer::SeatRegionLayer> layer) {
    _regionLayers.insert_or_assign(regionId, layer);
}

std::shared_ptr<kk::layer::SeatRegionLayer> BaseMapLayerManager::getRegionLayer(const std::string &regionId) const {
    if (regionId.empty()) {
        return nullptr;
    }
    auto iter = _regionLayers.find(regionId);
    if (iter != _regionLayers.end()) {
        return iter->second;
    }
    return nullptr;
}

void BaseMapLayerManager::addTextLayer(std::shared_ptr<kk::layer::SeatTextLayer> layer) {
    if (layer != nullptr) {
        _textLayers.push_back(layer);
    }
}

void BaseMapLayerManager::clear() {
    _regions.clear();
    _regionIdIndex.clear();
    _regionLayers.clear();
    _textLayers.clear();
}
};  // namespace kk
