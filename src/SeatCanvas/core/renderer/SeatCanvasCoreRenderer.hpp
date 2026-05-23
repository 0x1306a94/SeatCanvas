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
#include <unordered_set>
#include <vector>

#include <tgfx/core/Color.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

#include "core/BaseMapColorState.h"
#include "core/SeatData.hpp"
#include "core/ZoomLevelConfig.hpp"
#include "core/ZoomScaleConfig.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"
#include "core/renderer/SeatCanvasCoreRendererEvent.hpp"
#include "core/renderer/ViewportControllerCallback.hpp"
#include "core/style/SeatRenderStyleKey.hpp"
#include "core/style/SeatStyleConfig.hpp"

namespace tgfx {
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
class SeatDataManager;
class ViewportController;
class BaseMapMeshBuilder;
struct ZoneMeshInfo;
class SeatCanvasCoreRenderer : public ViewportControllerCallback {
  public:
    explicit SeatCanvasCoreRenderer(
        std::unique_ptr<PlatformView> platformView,
        std::unique_ptr<kk::gesture::ElasticZoomPanController> zoomPanController,
        const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig = {});

    ~SeatCanvasCoreRenderer();

    uint32_t coreID() const;

    const SeatCanvasCoreRendererState *state() const;

    void setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate);

    const kk::ZoomLevelConfig &zoomLevelConfig() const;

    /// 设置从彩虹图切换到绘制座位的缩放阈值
    /// 当当前缩放比例大于等于该阈值时，将绘制座位；否则显示彩虹图
    /// 如果不调用该方法，则使用内部计算得到的 ZoomLevelConfig.venue 作为默认阈值
    void setSeatRenderZoomThreshold(float zoomThreshold);

    /// 获取当前用于控制彩虹图与座位渲染切换的缩放阈值
    /// 如果未显式设置，则返回内部计算得到的 ZoomLevelConfig.venue
    float getSeatRenderZoomThreshold() const;

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

    /**
     * 设置样式键到配置的映射
     * 动态更新座位样式配置，会同步更新到 SeatStyleAtlasManager
     * @param styleIdToConfig 样式ID到配置的映射
     */
    void setStyleIdToConfig(const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig);

    /**
     * 从 JSON 数据设置样式键到配置的映射
     * @param bytes JSON 数据的字节数组
     * @param len JSON 数据的长度
     */
    void setStyleKeyToConfigFromJSON(const void *bytes, size_t len);

    /// 获取当前帧率（每秒帧数）
    float getFPS() const;

    /// 是否绘制调试 HUD（FPS、缩放级别、座位统计等）
    bool isDebugHUDEnabled() const;
    void setDebugHUDEnabled(bool enabled);

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
    void setBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> baseMapConfig);

    void invalidateContent();

    void start();

    void stop();

    void draw(bool force = false);

    /// 获取当前显示范围（在原始坐标系中的可见区域矩形）
    /// 考虑了当前的缩放和偏移
    /// @return 在原始坐标系中的可见区域矩形，如果内容为空则返回空矩形
    tgfx::Rect getVisibleOriginalRect() const;

    /// 查找与指定矩形相交的区域 ID 列表（原始坐标系）
    /// 遍历底图网格，判断 fillBounds 或 strokeBounds 与入参 rect 是否相交
    /// @param rect 查询矩形（原始坐标系，通常配合 getVisibleOriginalRect 使用）
    /// @return 相交区域的 zoneId 列表，已过滤空 zoneId
    std::vector<std::string> getZoneIdsInOriginalRect(const tgfx::Rect &rect) const;

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

    /// 更新区域颜色
    /// @param colors 颜色表
    void updateSeatZoneAlternateColors(const std::unordered_map<std::string, tgfx::Color> &colors);

    /// 更新小地图区域颜色
    /// @param colors 颜色表
    void updateMiniMapZoneAlternateColors(const std::unordered_map<std::string, tgfx::Color> &colors);

    /// 设置某个区域的座位数据
    /// @param zoneId 区域ID
    /// @param seats 座位几何数据列表，每个座位包含 seatId, x, y, rotation, pricecodeIndex
    /// @note 会重置该 zone 的 status 为 0，并清除旧 seat 的 selected；随后需 re-push status/selected
    void setSeatData(const std::string &zoneId, const std::vector<kk::SeatData> &seats);

    /// 清除所有区域和座位数据
    void clearSeatData();

    /// 注册价档表（load 前调用一次）
    void registerPricecodes(const std::vector<std::string> &pricecodes);

    /// 将价档字符串解析为 pricecodeIndex
    uint16_t pricecodeIndexForCode(const std::string &pricecode) const;

    /// 批量更新单个座位 status
    void updateSeatStatuses(const std::vector<kk::SeatStatusUpdate> &updates);

    /// 批量更新某个 zone 内全部座位 status（数组下标与 setSeatData 顺序一致）
    void updateSeatStatusesForZone(const std::string &zoneId, const std::vector<uint32_t> &statuses);

    /// 全量替换选中座位
    void setSelectedSeatIds(const std::vector<std::string> &seatIds);

    /// 增量更新选中座位
    void updateSelectedSeatIds(const std::vector<std::string> &added, const std::vector<std::string> &removed);

  private:
    /// 设置当前的底图配置
    /// @param config 底图配置
    void updateUseBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> config);

    /// 底图发生变化
    void handleBaseMapChanged();

    /// 从 PlatformView 同步 viewport 尺寸到 state（不触发 content size 重算）
    bool syncBoundsFromPlatformView();

    /// 设置底图
    /// @param layer 底图
    /// @param baseMapSize 底图原始大小
    void setBaseMapLayer(const tgfx::Size &baseMapSize);

    /// 设置minimap
    /// @param layer minimap
    void setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer);

    /// 内部方法，更新ZoomPanController状态并通知App
    void updateZoomPanControllerState(bool notifyViewport = true);
    SeatCanvasViewportEvent makeViewportEvent() const;

    void dispatchZoomLevelConfigUpdate();

    bool scheduleAnimator();
    void showMinimapWithoutAnimation();
    void hideMinimapWithAnimation();
    void hideMinimapWithoutAnimation();

    void prepareSeatIfNeeded();

    // ---- ViewportControllerCallback 实现 ----
    void onInvalidateContent() override;
    void onApplyBaseMapColorState(kk::BaseMapColorState state) override;
    void onHideMinimapWithoutAnimation() override;
    void onSetOverlayBackVisible(bool visible) override;
    void onSetOverlayBackAlpha(float alpha) override;
    bool onIsOverlayBackVisible() const override;
    void onUpdateZoomPanControllerState(bool notify) override;

    /// 处理座位选择（点击座位时触发）
    /// @param location 点击位置（viewport 坐标系，像素单位）
    void handleSeatSelectionAtLocation(const tgfx::Point &location);

    /// 处理点击时的自动缩放（智能缩放到点击区域）
    /// @param location 点击位置（viewport 坐标系，像素单位）
    void handleAutoZoomOnTap(const tgfx::Point &location);

    void applyBaseMapColorState(kk::BaseMapColorState toState);

    void applySavedSeatZoneAlternateColors(std::shared_ptr<BaseMapMeshBuilder> meshBuilder);
    void applySavedMiniMapZoneAlternateColors(std::shared_ptr<BaseMapMeshBuilder> meshBuilder);

    void drawDebugHUD(tgfx::Canvas *canvas);

    bool executeCustomRenderPass(tgfx::Context *context, const SeatCanvasCoreRendererState *state);

    /// 判断是否应该自动绘制座位
    /// @return 如果当前缩放级别大于 ZoomLevelConfig.venue 返回 true，否则返回 false
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
    std::unique_ptr<ViewportController> _viewportController = {nullptr};
    std::shared_ptr<tgfx::TextShaper> _textShaper = {nullptr};
    std::shared_ptr<kk::DisplayLink> _displayLink = {nullptr};
    std::shared_ptr<kk::BaseMapConfig> _baseMapConfig = {nullptr};
    std::weak_ptr<kk::BaseMapConfig> _useBaseMapConfig;

    tgfx::Color _backgroundColor = {tgfx::Color::White()};
    BaseMapColorState _baseMapColorState = {BaseMapColorState::Original};
    bool _debugHUDEnabled = {false};
    bool _invalidate = {true};
    kk::ZoomLevelConfig _zoomLevelConfig = {};
    float _seatRenderZoomThreshold = {0.0f};
    uint32_t _minimapAnimationId = {0};
    std::unordered_map<std::string, tgfx::Color> _zoneColorMap = {};
    std::unordered_map<std::string, tgfx::Color> _minimapZoneColorMap = {};
    std::unique_ptr<SeatDataManager> _seatDataManager = {nullptr};
    float _seatSize = {36.0f};
    size_t _renderedSeatZoneCount = {0};
    size_t _renderedSeatCount = {0};
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRenderer_hpp */
