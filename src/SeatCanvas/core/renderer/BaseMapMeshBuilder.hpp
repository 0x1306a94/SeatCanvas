//
//  BaseMapMeshBuilder.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef BaseMapMeshBuilder_hpp
#define BaseMapMeshBuilder_hpp

#include "BaseMapZoneVertex.hpp"
#include "ZoneMeshInfo.hpp"

#include <tgfx/core/Color.h>
#include <tgfx/core/Path.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Stroke.h>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace kk::renderer {

/*
 * 底图网格构建器
 *
 * 不变量：
 * 所有区域网格都严格按照顺序附加到单个顶点缓冲区中。
 * 每个区域都占据连续的顶点范围。
 *
 */
class BaseMapMeshBuilder {
  public:
    struct RegionDrawRange {
        uint32_t vertexOffset = 0;
        uint32_t vertexCount = 0;
    };

    BaseMapMeshBuilder() = default;
    ~BaseMapMeshBuilder() = default;

    /// 添加区域网格信息
    /// @param regionInfo 区域网格信息（包含原始数据、三角化后的顶点等）
    void addZoneMeshInfo(std::shared_ptr<ZoneMeshInfo> regionInfo);

    /// 获取所有区域网格信息（构建前）
    const std::vector<std::shared_ptr<ZoneMeshInfo>> &getZoneMeshInfos() const {
        return ZoneMeshInfos;
    }

    size_t getTotalVertexCount() const {
        return totalVertexCount;
    }

    std::optional<RegionDrawRange> findRegionDrawRangeById(const std::string &zoneId) const;
    std::optional<RegionDrawRange> findRegionDrawRangeByIndex(size_t index) const;

    /// Hit-test 相关方法（基于 ZoneMeshInfo）

    /// 根据区域ID查找区域网格信息
    /// @param zoneId 区域ID
    /// @return 区域网格信息，如果不存在则返回 nullptr
    std::shared_ptr<ZoneMeshInfo> findRegionById(const std::string &zoneId) const;

    /// 查找与指定矩形相交的所有区域
    /// @param rect 查询矩形
    /// @param visibleIndices 索引
    /// @return 相交的区域网格信息列表
    std::vector<std::shared_ptr<ZoneMeshInfo>> findRegionsIntersectingRect(const tgfx::Rect &rect, std::vector<size_t> *visibleIndices = nullptr) const;

    /// 查找包含指定点的区域
    /// @param point 查询点（SVG 坐标）
    /// @return 包含该点的区域网格信息，如果不存在则返回 nullptr（返回最上层的区域）
    std::shared_ptr<ZoneMeshInfo> findRegionContainingPoint(const tgfx::Point &point) const;

    /// 根据区域ID获取区域 Path
    /// @param zoneId 区域ID
    /// @return 区域 Path，如果不存在则返回 nullptr
    const tgfx::Path *getRegionPath(const std::string &zoneId) const;

  private:
    std::vector<std::shared_ptr<ZoneMeshInfo>> ZoneMeshInfos = {};
    std::vector<RegionDrawRange> drawRanges = {};
    std::unordered_map<std::string, std::shared_ptr<ZoneMeshInfo>> zoneIdToMeshInfo = {};
    std::unordered_map<std::string, size_t> zoneIdToIndex = {};
    uint32_t totalVertexCount = 0;
    uint32_t vertexOffset = 0;
};

};  // namespace kk::renderer

#endif /* BaseMapMeshBuilder_hpp */
