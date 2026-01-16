//
//  RegionLayerBuilder.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "RegionLayerBuilder.hpp"

#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

#include <algorithm>
#include <array>
#include <queue>
#include <set>

namespace kk::renderer {

void RegionLayerBuilder::addRegion(int32_t index, const std::string &regionId, const tgfx::Path &path, const tgfx::Rect &bounds) {
    _regions.emplace_back(index, regionId, path, bounds);
}

bool RegionLayerBuilder::containsRegion(const RegionInfo &a, const RegionInfo &b) const {
    // 1. 先检查中心点（这对于圆形等形状很重要）
    // 如果区域 B 的中心点不在区域 A 内，则 A 不包含 B
    const float centerX = b.bounds.centerX();
    const float centerY = b.bounds.centerY();

    if (!a.path.contains(centerX, centerY)) {
        return false;
    }

    // 2. 检查边界框的角点
    // 如果区域 B 的边界框的角点都在区域 A 内，则认为 A 包含 B
    const float left = b.bounds.x();
    const float top = b.bounds.y();
    const float right = b.bounds.x() + b.bounds.width();
    const float bottom = b.bounds.y() + b.bounds.height();

    // 检查四个角点
    const std::array<tgfx::Point, 4> cornerPoints = {
        tgfx::Point::Make(left, top),      // 左上角
        tgfx::Point::Make(right, top),     // 右上角
        tgfx::Point::Make(right, bottom),  // 右下角
        tgfx::Point::Make(left, bottom)    // 左下角
    };

    for (const auto &point : cornerPoints) {
        if (!a.path.contains(point.x, point.y)) {
            return false;
        }
    }

    return true;
}

bool RegionLayerBuilder::regionsOverlap(const RegionInfo &a, const RegionInfo &b) const {
    // 1. 先用 bounds 进行快速过滤
    if (!tgfx::Rect::Intersects(a.bounds, b.bounds)) {
        return false;
    }

    // 2. 使用采样点检测重叠
    // 检查区域 A 的边界框的角点和中心点是否在区域 B 内，或者反之
    const float aLeft = a.bounds.x();
    const float aTop = a.bounds.y();
    const float aRight = a.bounds.x() + a.bounds.width();
    const float aBottom = a.bounds.y() + a.bounds.height();
    const float aCenterX = a.bounds.centerX();
    const float aCenterY = a.bounds.centerY();

    const float bLeft = b.bounds.x();
    const float bTop = b.bounds.y();
    const float bRight = b.bounds.x() + b.bounds.width();
    const float bBottom = b.bounds.y() + b.bounds.height();
    const float bCenterX = b.bounds.centerX();
    const float bCenterY = b.bounds.centerY();

    // 检查区域 A 的采样点是否在区域 B 内
    const std::array<tgfx::Point, 5> aTestPoints = {
        tgfx::Point::Make(aLeft, aTop),
        tgfx::Point::Make(aRight, aTop),
        tgfx::Point::Make(aRight, aBottom),
        tgfx::Point::Make(aLeft, aBottom),
        tgfx::Point::Make(aCenterX, aCenterY)};

    for (const auto &point : aTestPoints) {
        if (b.path.contains(point.x, point.y)) {
            return true;
        }
    }

    // 检查区域 B 的采样点是否在区域 A 内
    const std::array<tgfx::Point, 5> bTestPoints = {
        tgfx::Point::Make(bLeft, bTop),
        tgfx::Point::Make(bRight, bTop),
        tgfx::Point::Make(bRight, bBottom),
        tgfx::Point::Make(bLeft, bBottom),
        tgfx::Point::Make(bCenterX, bCenterY)};

    for (const auto &point : bTestPoints) {
        if (a.path.contains(point.x, point.y)) {
            return true;
        }
    }

    return false;
}

std::vector<std::vector<int32_t>> RegionLayerBuilder::buildLayerHierarchy() {
    if (_regions.empty()) {
        return {};
    }

    const size_t n = _regions.size();

    // 跳过背景区域（假设第一个区域是背景，index 为 0）
    // 从索引 1 开始处理实际的图形区域
    if (n <= 1) {
        return {};
    }

    const size_t startIndex = 1;  // 跳过背景
    const size_t actualRegionCount = n - startIndex;

    // 1. 构建遮挡关系图（无向图）
    // adjacencyList[i] 包含所有与区域 i 重叠的区域索引
    // 如果区域 i 和 j 重叠，则它们不能在同一层级
    std::vector<std::vector<size_t>> adjacencyList(actualRegionCount);

    // 检测所有区域对的重叠关系（跳过背景）
    for (size_t i = startIndex; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            // 如果区域 i 和 j 重叠，则它们不能在同一层级
            if (regionsOverlap(_regions[i], _regions[j])) {
                size_t localI = i - startIndex;
                size_t localJ = j - startIndex;
                adjacencyList[localI].push_back(localJ);
                adjacencyList[localJ].push_back(localI);
            }
        }
    }

    // 2. 使用贪心图着色算法分配层级
    // color[i] 表示区域 i 的层级（从 0 开始）
    std::vector<int32_t> color(actualRegionCount, -1);
    std::vector<std::vector<int32_t>> layers;

    // 为每个区域分配层级
    for (size_t regionIdx = 0; regionIdx < actualRegionCount; regionIdx++) {
        // 找到邻接区域已使用的颜色（层级）
        std::set<int32_t> usedColors;
        for (size_t neighbor : adjacencyList[regionIdx]) {
            if (color[neighbor] != -1) {
                usedColors.insert(color[neighbor]);
            }
        }

        // 找到最小的未使用的颜色（层级）
        int32_t assignedColor = 0;
        while (usedColors.count(assignedColor) > 0) {
            assignedColor++;
        }

        color[regionIdx] = assignedColor;

        // 确保 layers 有足够的容量
        if (static_cast<size_t>(assignedColor) >= layers.size()) {
            layers.resize(assignedColor + 1);
        }

        // 将区域添加到对应的层级
        layers[assignedColor].push_back(_regions[regionIdx + startIndex].index);
    }

    // 移除空的层级（虽然理论上不应该有空层级）
    layers.erase(std::remove_if(layers.begin(), layers.end(),
                                [](const std::vector<int32_t> &layer) {
                                    return layer.empty();
                                }),
                 layers.end());

    return layers;
}

void RegionLayerBuilder::clear() {
    _regions.clear();
}

}  // namespace kk::renderer
