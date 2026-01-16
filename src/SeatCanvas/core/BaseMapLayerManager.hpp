//
//  BaseMapLayerManager.hpp
//  SeatCanvas
//
//  Created by KK on 2025/11/15.
//

#ifndef BaseMapLayerManager_hpp
#define BaseMapLayerManager_hpp

#include "RegionInfo.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace kk::layer {
class SeatRegionLayer;
class SeatTextLayer;
};  // namespace kk::layer

namespace kk {

/// 底图区域Layer管理器
/// 用于存储和管理所有区域信息及对应的Layer，提供高效的查询接口
/// 支持通过坐标查找区域，以及通过区域ID获取对应的Layer
class BaseMapLayerManager {
  public:
    BaseMapLayerManager() = default;
    ~BaseMapLayerManager() = default;

    /// 添加区域信息
    void addRegion(const RegionInfo &region);

    /// 根据区域ID查找区域信息
    /// @param regionId 区域ID
    /// @return 区域信息，如果不存在则返回 nullptr
    const RegionInfo *findRegionById(const std::string &regionId) const;

    /// 查找与指定矩形相交的所有区域
    /// @param rect 查询矩形
    /// @return 相交的区域信息列表
    std::vector<const RegionInfo *> findRegionsIntersectingRect(const tgfx::Rect &rect) const;

    /// 查找包含指定点的区域
    /// @param point 查询点（内容坐标）
    /// @return 包含该点的区域信息，如果不存在则返回 nullptr
    const RegionInfo *findRegionContainingPoint(const tgfx::Point &point) const;

    /// 获取所有区域信息
    const std::vector<RegionInfo> &getAllRegions() const {
        return _regions;
    }

    /// 添加区域Layer
    /// - Parameters:
    ///   - regionId: 区域ID
    ///   - layer: layer
    void addRegionLayer(const std::string &regionId, std::shared_ptr<kk::layer::SeatRegionLayer> layer);

    /// 根据区域ID获取区域Layer
    /// @param regionId 区域ID
    /// @return 区域Layer，如果不存在则返回 nullptr
    std::shared_ptr<kk::layer::SeatRegionLayer> getRegionLayer(const std::string &regionId) const;

    /// 获取所有区域Layer的映射
    /// @return 区域ID到Layer的映射
    const std::unordered_map<std::string, std::shared_ptr<kk::layer::SeatRegionLayer>> &getAllRegionLayers() const {
        return _regionLayers;
    }

    /// 添加文本Layer
    /// @param layer 文本Layer
    void addTextLayer(std::shared_ptr<kk::layer::SeatTextLayer> layer);

    /// 获取所有文本Layer
    /// @return 文本Layer列表
    const std::vector<std::shared_ptr<kk::layer::SeatTextLayer>> &getAllTextLayers() const {
        return _textLayers;
    }

    /// 清空所有区域信息
    void clear();

    /// 获取区域数量
    size_t size() const {
        return _regions.size();
    }

  private:
    std::vector<RegionInfo> _regions;
    std::unordered_map<std::string, size_t> _regionIdIndex;  // regionId -> index in _regions
    std::unordered_map<std::string, std::shared_ptr<kk::layer::SeatRegionLayer>> _regionLayers;
    std::vector<std::shared_ptr<kk::layer::SeatTextLayer>> _textLayers;
};
};  // namespace kk

#endif /* BaseMapLayerManager_hpp */
