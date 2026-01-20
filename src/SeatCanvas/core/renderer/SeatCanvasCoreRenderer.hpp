//
//  SeatCanvasCoreRenderer.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/11.
//

#ifndef SeatCanvasCoreRenderer_hpp
#define SeatCanvasCoreRenderer_hpp

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <tgfx/core/Color.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/gpu/RenderPass.h>

#include "core/BaseMapColorState.h"
#include "core/SeatData.hpp"
#include "core/SeatRenderMode.hpp"
#include "core/SeatZoneData.hpp"
#include "core/ZoomLevelConfig.hpp"
#include "core/ZoomScaleConfig.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"
#include "core/style/SeatStyleConfig.hpp"
#include "core/style/SeatStyleKey.hpp"

namespace tgfx {
class Recording;
class TextShaper;
class Context;
class Canvas;
class Surface;
class Texture;
class GPU;
class Image;
};  // namespace tgfx

namespace kk {
class DisplayLink;
class BaseMapConfig;
};  // namespace kk

namespace kk::animation {
class Animator;
};

namespace kk::layer {
class BaseMapRootLayer;
};

namespace kk::drawers {
class SeatZoneNameLayerTree;
class SeatOverlayLayerTree;
};  // namespace kk::drawers

namespace kk::gesture {
class ElasticZoomPanController;
};

namespace kk::renderer {
class PlatformView;
class RenderFrameMetrics;
class CustomBaseMapPass;
class CustomSeatPass;
class SeatStyleAtlasManager;
class SeatCanvasCoreRendererState;
class BaseMapMeshBuilder;
class SeatCanvasCoreRenderer {
  public:
    explicit SeatCanvasCoreRenderer(
        std::unique_ptr<PlatformView> platformView,
        std::unique_ptr<kk::gesture::ElasticZoomPanController> zoomPanController,
        const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleKeyToConfig = {});

    ~SeatCanvasCoreRenderer();

    uint32_t coreID() const;

    const SeatCanvasCoreRendererState *state();

    void setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate);

    const kk::ZoomLevelConfig &zoomLevelConfig() const;

    void replacePlatformView(std::unique_ptr<PlatformView> platformView);

    bool updateSize();
    void setMaxWidth(float maxWidth);
    float getMaxWidth() const;

    // 内容缩放比例
    float getContentScale() const;

    float getMinimumZoomScale() const;
    float getMaximumZoomScale() const;
    float getZoomScale() const;
    void setZoomScale(float zoomScale);
    const tgfx::Point &getContentOffset() const;
    void setContentOffset(const tgfx::Point &contentOffset);
    bool isSmallVenue() const;
    float showBackZoomThreshold() const;
    const tgfx::Color &getBackgroundColor() const;
    void setBackgroundColor(const tgfx::Color &color);

    float getSeatSize() const;
    void setSeatSize(float seatSize);

    /// 高亮指定区域：高亮区域 additionalAlpha=1.0f，其余为 nonHighlightedAlpha
    /// @param zoneIds 高亮区域的 ID 集合
    /// @param nonHighlightedAlpha 非高亮区域的 additionalAlpha
    void setHighlightedZoneIds(const std::vector<std::string> &zoneIds, float nonHighlightedAlpha);

    /// 清除高亮，将所有区域 additionalAlpha 重置为 1.0f
    void clearHighlightedZones();

    /**
     * 设置样式键到配置的映射
     * 动态更新座位样式配置，会同步更新到 SeatStyleAtlasManager
     * @param styleKeyToConfig 样式键到配置的映射
     */
    void setStyleKeyToConfig(const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleKeyToConfig);

    /**
     * 从 JSON 数据设置样式键到配置的映射
     * @param bytes JSON 数据的字节数组
     * @param len JSON 数据的长度
     */
    void setStyleKeyToConfigFromJSON(const void *bytes, size_t len);

    kk::SeatRenderMode seatRenderMode() const;

    /// 获取当前帧率（每秒帧数）
    float getFPS() const;

    /// MARK: 手势处理方法，由平台层调用

    /// 处理点击手势
    /// - Parameter location: 点击位置, viewport 坐标系 (像素单位)
    void handleTap(const tgfx::Point &location);

    /// 处理滑动手势
    /// - Parameters:
    ///   - state: 手势状态
    ///   - translation: 手势当前平移量 (像素单位)
    ///   - timestampMs: 手势事件时间戳（毫秒）
    void handlePan(kk::gesture::GestureState state, const tgfx::Point &translation, double timestampMs);

    /// 处理捏合手势
    /// - Parameters:
    ///   - state: 手势状态
    ///   - scale: 当前缩放比例
    ///   - center: 缩放中心（捏合手势发送时的中心位置，始终是在viewport的坐标系中）
    void handlePinch(kk::gesture::GestureState state, float scale, const tgfx::Point &center);

    /// 设置底图配置
    /// - Parameter baseMapConfig: 底图配置
    /// - Parameter renderMode: 渲染模式
    void setBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> baseMapConfig, kk::SeatRenderMode renderMode);

    void invalidateSeatStatusImage();

    void invalidateContent();

    void start();

    void stop();

    void draw(bool force = false);

    /// 获取当前显示范围（在原始坐标系中的可见区域矩形）
    /// 考虑了当前的缩放和偏移
    /// @return 在原始坐标系中的可见区域矩形，如果内容为空则返回空矩形
    tgfx::Rect getVisibleOriginalRect() const;

    /// 将屏幕坐标转为内容坐标系中的坐标
    /// @param location viewport 坐标系
    /// @param contentOffset 位移
    /// @param scale 缩放
    tgfx::Point convertScreenToContent(const tgfx::Point &location, const tgfx::Point &contentOffset, float scale) const;

    /// 将内容坐标转为屏幕坐标系中的坐标
    /// @param location 内容坐标系
    /// @param contentOffset 位移
    /// @param scale 缩放
    tgfx::Point convertContentToScreen(const tgfx::Point &location, const tgfx::Point &contentOffset, float scale) const;

    /// 将规范化内容坐标转换为原始坐标
    /// @param normalizedContentLocation 规范化内容坐标
    tgfx::Point convertNormalizedContentToOriginal(const tgfx::Point &normalizedContentLocation) const;

    /// 将原始坐标转换为规范化内容坐标
    /// @param originalLocation 原始坐标
    tgfx::Point convertOriginalToNormalizedContent(const tgfx::Point &originalLocation) const;

    /// 将屏幕坐标转换为原始坐标
    /// @param screenLocation 屏幕坐标
    tgfx::Point convertScreenToOriginal(const tgfx::Point &screenLocation) const;

    /// 将原始坐标转换为屏幕坐标
    /// @param originalLocation 原始坐标
    tgfx::Point convertOriginalToScreen(const tgfx::Point &originalLocation) const;

    /// 判断屏幕坐标是否在内容区域内
    /// @param screenLocation 屏幕坐标
    /// @return 如果在内容区域内返回 true，否则返回 false（例如点击到留白区域）
    bool isPointInContentArea(const tgfx::Point &screenLocation) const;

    /// 缩放到指定区域并居中显示
    /// @param rect 目标区域（在内容坐标系中）
    /// @param animated 是否使用动画
    /// @param padding 区域周围的边距（在内容坐标系中），默认为 0
    /// @param durationMs 动画持续时间（毫秒），仅在 animated 为 true 时有效，默认 300ms
    void zoomToRect(const tgfx::Rect &rect, bool animated = true, float padding = 0.0f, double durationMs = 300.0);

    /// 设置选中的区域ID（仅对 ClickToEnter 模式有效）
    /// @param zoneId 区域ID，为空表示取消选择，恢复到全区域视图
    void setSelectedzoneId(const std::string &zoneId);

    /// 设置区域数据
    /// 用于设置区域的显示信息（区域颜色和价格颜色）
    /// @param zoneData 区域数据，包含 zoneId, zoneColor, priceColor
    void setZoneData(const kk::SeatZoneData &zoneData);

    /// 设置某个区域的座位数据
    /// @param zoneId 区域ID
    /// @param seats 座位数据列表，每个座位包含 seatId, status, x, y
    void setSeatData(const std::string &zoneId, const std::vector<kk::SeatData> &seats);

    /// 更新座位状态
    /// @param zoneId 区域ID
    /// @param seatId 座位ID
    /// @param status 新的座位状态
    void updateSeatStatus(const std::string &zoneId, const std::string &seatId, uint32_t status);

    /// 清除所有区域和座位数据
    void clearSeatData();

  private:
    /// 设置当前的底图配置
    /// @param config 底图配置
    void updateUseBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> config);

    /// 底图发生变化
    void handleBaseMapChanged();

    /// 设置底图
    /// @param layer 底图
    /// @param baseMapSize 底图原始大小
    void setBaseMapLayer(const tgfx::Size &baseMapSize);

    /// 设置minimap
    /// @param layer minimap
    void setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer);

    /// 为区域创建虚拟的 BaseMapLayer
    /// @param zoneId 区域ID
    std::shared_ptr<kk::layer::BaseMapRootLayer> buildVirtualBaseMapLayerForZone(const std::string &zoneId);

    /// 内部方法，根据原始尺寸更新内容缩放比例（用于规范化内容尺寸）
    void updateContentScale();

    /// 内部方法，更新内容尺寸
    void updateContentSize();

    /// 内部方法，更新最大最小缩放比例
    void updateMaxMinZoomScalesForCurrentBounds();

    /// 内部方法，更新ZoomPanController状态并通知App
    void updateZoomPanControllerState();

    bool scheduleAnimator();
    void showMinimapWithoutAnimation();
    void hideMinimapWithAnimation();
    void hideMinimapWithoutAnimation();
    void animateMinimapToAlpha(float targetAlpha, double durationMs);

    void prepareSeatIfNeeded();

    void handleZoomBack();

    /// 处理座位选择（点击座位时触发）
    /// @param location 点击位置（viewport 坐标系，像素单位）
    void handleSeatSelectionAtLocation(const tgfx::Point &location);

    /// 处理点击时的自动缩放（智能缩放到点击区域）
    /// @param location 点击位置（viewport 坐标系，像素单位）
    void handleAutoZoomOnTap(const tgfx::Point &location);

    /// 根据位置执行渐进式缩放
    /// @param location 缩放目标位置（viewport 坐标系，像素单位）
    void scrollViewWithLocation(const tgfx::Point &location);

    void scrollViewWithZone(const std::string &zoneId);

    void zoomToPoint(const tgfx::Point &location, float scale, bool animated = true, float padding = 0.0f, double durationMs = 300.0);

    void applyBaseMapColorState(kk::BaseMapColorState toState);

    void applySavedZoneDataColors(std::shared_ptr<BaseMapMeshBuilder> meshBuilder);

    void drawFPS(tgfx::Canvas *canvas);

    bool executeCustomRenderPass(tgfx::Context *context, const SeatCanvasCoreRendererState *state);

    /// 判断是否应该自动绘制座位
    /// @return 如果当前缩放级别大于 zoomScale50 返回 true，否则返回 false
    bool shouldAutoDrawSeat() const;

  private:
    uint32_t _coreID;
    std::shared_ptr<SeatCanvasCoreRendererDelegate> _delegate;
    std::unique_ptr<PlatformView> _platformView;
    std::unique_ptr<kk::gesture::ElasticZoomPanController> _zoomPanController;
    std::unique_ptr<SeatCanvasCoreRendererState> _state;
    std::unique_ptr<CustomBaseMapPass> _customBaseMapPass;
    std::unique_ptr<CustomSeatPass> _customSeatPass;
    std::unique_ptr<SeatStyleAtlasManager> _seatAtlasManager;
    std::unique_ptr<kk::drawers::SeatZoneNameLayerTree> _seatZoneNameLayer;
    std::unique_ptr<kk::drawers::SeatOverlayLayerTree> _overlayLayer;
    std::unique_ptr<kk::animation::Animator> _animator;
    std::unique_ptr<RenderFrameMetrics> _frameMetrics;
    std::shared_ptr<tgfx::TextShaper> _textShaper = {nullptr};
    std::unique_ptr<tgfx::Recording> _lastRecording = {nullptr};
    std::shared_ptr<kk::DisplayLink> _displayLink = {nullptr};
    std::shared_ptr<kk::BaseMapConfig> _baseMapConfig = {nullptr};
    std::unordered_map<std::string, std::shared_ptr<kk::BaseMapConfig>> _virtualBaseMapConfig = {};
    std::weak_ptr<kk::BaseMapConfig> _useBaseMapConfig;

    tgfx::Color _backgroundColor = {tgfx::Color::White()};
    BaseMapColorState _baseMapColorState = {BaseMapColorState::Original};
    bool _autoChangeBaseMapColorState = {true};
    bool _disableAutoDrawSeat;
    bool _firstFrameSubmitted = {false};
    bool _invalidate = {true};
    float _maxWidth = {1000.f};
    float _svgModelScale = {1.0f};
    kk::SeatRenderMode _renderMode = {kk::SeatRenderMode::ZoomBased};
    kk::ZoomLevelConfig _zoomLevelConfig = {};
    uint32_t _minimapAnimationId = {0};

    std::unordered_map<std::string, kk::SeatZoneData> _zoneDataMap = {};
    std::unordered_map<std::string, std::vector<kk::SeatData>> _seatDataMap = {};
    float _seatSize = {36.0f};
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRenderer_hpp */
