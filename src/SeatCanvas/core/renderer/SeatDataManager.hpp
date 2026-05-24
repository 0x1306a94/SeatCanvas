//
//  SeatDataManager.hpp
//  SeatCanvas
//
//  Created by king on 2026/05/23.
//

#ifndef SeatDataManager_hpp
#define SeatDataManager_hpp

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <tgfx/core/Rect.h>

#include "core/SeatData.hpp"
#include "core/renderer/SeatInstanceData.hpp"
#include "core/style/SeatRenderStyleKey.hpp"

namespace kk::renderer {
class SeatStyleConfig;

class SeatStyleAtlasManager;
class BaseMapMeshBuilder;

class SeatDataManager {
  public:
    explicit SeatDataManager(SeatStyleAtlasManager *atlasManager);

    /// 设置某个区域的座位数据
    bool setSeatData(const std::string &zoneId, const std::vector<kk::SeatData> &seats);

    /// 清除所有区域和座位数据
    bool clearSeatData();

    /// 注册价档表
    void registerPricecodes(const std::vector<std::string> &pricecodes);

    /// 将价档字符串解析为 pricecodeIndex
    uint16_t pricecodeIndexForCode(const std::string &pricecode) const;

    /// 批量更新单个座位 status
    bool updateSeatStatuses(const std::vector<kk::SeatStatusUpdate> &updates);

    /// 批量更新某个 zone 内全部座位 status
    bool updateSeatStatusesForZone(const std::string &zoneId, const uint32_t *statuses, size_t count);

    /// 全量替换选中座位
    void setSelectedSeatIds(const std::vector<std::string> &seatIds);

    /// 增量更新选中座位
    bool updateSelectedSeatIds(const std::vector<std::string> &added, const std::vector<std::string> &removed);

    /// 设置样式键到配置的映射，返回 atlas 是否发生变化
    bool setStyleIdToConfig(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig);

    /// 从 JSON 数据设置样式键到配置的映射，返回 atlas 是否发生变化
    bool setStyleKeyToConfigFromJSON(const void *bytes, size_t len);

    /// atlas 生成后重建 {pricecodeIndex, status, selected} → uvIndex 查表
    void rebuildStyleKeyLookup();

    /// 收集可见区域的座位实例数据
    struct CollectResult {
        std::vector<SeatInstanceData> instances = {};
        size_t zoneCount = 0;
    };
    CollectResult collectVisibleSeatInstances(const tgfx::Rect &visibleOriginalRect, BaseMapMeshBuilder *meshBuilder, float seatSize) const;

    /// 查询指定区域的座位数据，不存在则返回 nullptr
    const std::vector<kk::SeatData> *getSeatDataForZone(const std::string &zoneId) const;

    size_t getZoneCount() const;
    size_t getTotalSeatCount() const;

  private:
    struct SeatLocation {
        std::string zoneId = {};
        size_t index = 0;
    };

    struct ZoneSeatRuntimeState {
        std::vector<uint32_t> statuses = {};
    };

    SeatStyleAtlasManager *_atlasManager;  // 非拥有指针

    std::unordered_map<std::string, std::vector<kk::SeatData>> _seatDataMap = {};
    std::unordered_map<std::string, ZoneSeatRuntimeState> _seatStateByZone = {};
    std::unordered_map<std::string, SeatLocation> _seatIndexById = {};
    std::unordered_set<std::string> _selectedSeatIds = {};
    std::vector<std::string> _pricecodes = {};
    std::unordered_map<std::string, uint16_t> _pricecodeToIndex = {};
    std::unordered_map<SeatRenderStyleKey, int32_t, SeatRenderStyleKeyHash> _uvIndexByStyleKey = {};
    std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> _registeredStyleIdToConfig = {};
};

};  // namespace kk::renderer

#endif /* SeatDataManager_hpp */
