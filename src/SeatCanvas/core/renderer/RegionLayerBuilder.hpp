//
//  RegionLayerBuilder.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef RegionLayerBuilder_hpp
#define RegionLayerBuilder_hpp

#include <tgfx/core/Path.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace kk::renderer {

/// 区域层级构建器
/// 用于构建区域的遮挡层级关系，将区域分组以便合并绘制
class RegionLayerBuilder {
  public:
    /// 区域信息
    struct RegionInfo {
        int32_t index;         // 区域索引（对应颜色索引）
        std::string regionId;  // 区域ID（可选）
        tgfx::Path path;       // 区域路径（用于精确检测遮挡）
        tgfx::Rect bounds;     // 区域边界（用于快速过滤）

        RegionInfo(int32_t idx, const std::string &id, const tgfx::Path &p, const tgfx::Rect &b)
            : index(idx)
            , regionId(id)
            , path(p)
            , bounds(b) {
        }
    };

    /// 添加区域信息
    /// @param index 区域索引
    /// @param regionId 区域ID
    /// @param path 区域路径
    /// @param bounds 区域边界
    void addRegion(int32_t index, const std::string &regionId, const tgfx::Path &path, const tgfx::Rect &bounds);

    /// 构建层级结构
    /// @return 按层级分组的区域索引列表，外层 vector 表示层级（从底到顶），内层 vector 表示同一层的区域索引
    /// 例如：[[0], [1, 2], [3]] 表示区域 0 在最底层，区域 1 和 2 在中间层，区域 3 在最顶层
    std::vector<std::vector<int32_t>> buildLayerHierarchy();

    /// 清空所有数据
    void clear();

  private:
    /// 检测区域 A 是否完全包含区域 B
    /// @param a 区域 A
    /// @param b 区域 B
    /// @return true 如果 A 完全包含 B
    bool containsRegion(const RegionInfo &a, const RegionInfo &b) const;

    /// 检测两个区域是否重叠
    /// @param a 区域 A
    /// @param b 区域 B
    /// @return true 如果 A 和 B 重叠
    bool regionsOverlap(const RegionInfo &a, const RegionInfo &b) const;

    std::vector<RegionInfo> _regions;
};

}  // namespace kk::renderer

#endif /* RegionLayerBuilder_hpp */
