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

#include <tgfx/core/Color.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

#include "core/RegionInfo.hpp"
#include "core/SeatRenderMode.hpp"
#include "core/ZoomLevelConfig.hpp"
#include "core/ZoomScaleConfig.hpp"
#include "core/gesture/GestureState.hpp"
#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace tgfx {
class Recording;
class TextShaper;
class Canvas;
};  // namespace tgfx

namespace kk {
class DisplayLink;
class BaseMapConfig;
class BaseMapLayerManager;
};  // namespace kk

namespace kk::animation {
class Animator;
};

namespace kk::layer {
class BaseMapRootLayer;
};

namespace kk::drawers {
class SeatLayerTree;
class SeatOverlayLayerTree;
};  // namespace kk::drawers

namespace kk::gesture {
class ElasticZoomPanController;
};

namespace kk::renderer {
class RendererBackend;
class RenderFrameMetrics;
class SeatItemImageProvider;
class SeatCanvasCoreRendererState;
class SeatCanvasCoreRenderer {
  private:
    enum class BaseMapColorState {
        Original,
        Rainbow,
    };

  public:
    explicit SeatCanvasCoreRenderer(std::unique_ptr<RendererBackend> backend, std::unique_ptr<kk::gesture::ElasticZoomPanController> zoomPanController);

    ~SeatCanvasCoreRenderer();

    uint32_t coreID() const;

    const SeatCanvasCoreRendererState *state();

    void setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate);

    const kk::ZoomLevelConfig &zoomLevelConfig() const;

    void replaceBackend(std::unique_ptr<RendererBackend> backend);

    bool updateSize();
    void setMaxWidth(float maxWidth);
    float getMaxWidth() const;

    float getSvgScale() const;

    float getMinimumZoomScale() const;
    float getMaximumZoomScale() const;
    float getZoomScale() const;
    void setZoomScale(float zoomScale);
    const tgfx::Point &getContentOffset() const;
    void setContentOffset(const tgfx::Point &contentOffset);
    bool isSmallVenue() const;
    float showBackZoomThreshold() const;
    void setBackgroundColor(const tgfx::Color &color);

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

    void enableTiled(bool enable);
    void enableZoomBlur(bool enable);

    void invalidateSeatStatusImage();

    void invalidateContent();

    void start();

    void stop();

    void draw(bool force = false);

    /// 获取当前显示范围（在内容坐标系中的可见区域矩形）
    /// 考虑了当前的缩放和偏移
    /// @return 在内容坐标系中的可见区域矩形，如果内容为空则返回空矩形
    tgfx::Rect getVisibleContentRect() const;

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

    /// 获取区域数据
    /// @param x x 坐标
    /// @param y y 坐标
    /// @return 区域数据
    const kk::RegionInfo *getSeatRegionDataByPoint(float x, float y) const;

    /// 缩放到指定区域并居中显示
    /// @param rect 目标区域（在内容坐标系中）
    /// @param animated 是否使用动画
    /// @param padding 区域周围的边距（在内容坐标系中），默认为 0
    /// @param durationMs 动画持续时间（毫秒），仅在 animated 为 true 时有效，默认 300ms
    void zoomToRect(const tgfx::Rect &rect, bool animated = true, float padding = 0.0f, double durationMs = 300.0);

    /// 设置选中的区域ID（仅对 ClickToEnter 模式有效）
    /// @param regionId 区域ID，为空表示取消选择，恢复到全区域视图
    void setSelectedRegionId(const std::string &regionId);

  private:
    /// 设置当前的底图配置
    /// @param config 底图配置
    void updateUseBaseMapConfig(std::shared_ptr<kk::BaseMapConfig> config);

    /// 底图发生变化
    void handleBaseMapChanged();

    /// 设置底图
    /// @param layer 底图
    /// @param baseMapSize 底图原始大小
    void setBaseMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &baseMapSize);

    /// 设置minimap
    /// @param layer minimap
    void setMiniMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer);

    /// 为区域创建虚拟的 BaseMapLayer
    /// @param regionId 区域ID
    std::shared_ptr<kk::layer::BaseMapRootLayer> buildVirtualBaseMapLayerForRegion(const std::string &regionId);

    /// 内部方法，更新SVG缩放比例
    void updateSvgScale();

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

    void drawSeatIfNeeded(tgfx::Canvas *canvas);

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

    void scrollViewWithRegion(const std::string &regionId);

    void zoomToPoint(const tgfx::Point &location, float scale, bool animated = true, float padding = 0.0f, double durationMs = 300.0);

    void applyBaseMapColorState(const kk::BaseMapLayerManager *layerManager, BaseMapColorState toState);

    void drawFPS(tgfx::Canvas *canvas);

  private:
    uint32_t _coreID;
    std::shared_ptr<SeatCanvasCoreRendererDelegate> _delegate;
    std::unique_ptr<RendererBackend> _backend;
    std::unique_ptr<kk::gesture::ElasticZoomPanController> _zoomPanController;
    std::unique_ptr<SeatCanvasCoreRendererState> _state;
    std::unique_ptr<kk::drawers::SeatLayerTree> _seatLayer;
    std::unique_ptr<kk::drawers::SeatOverlayLayerTree> _overlayLayer;
    std::unique_ptr<kk::animation::Animator> _animator;
    std::unique_ptr<RenderFrameMetrics> _frameMetrics;
    std::unique_ptr<SeatItemImageProvider> _seatImageProvider = {nullptr};
    std::shared_ptr<tgfx::TextShaper> _textShaper = {nullptr};
    std::unique_ptr<tgfx::Recording> _lastRecording = {nullptr};
    std::shared_ptr<kk::DisplayLink> _displayLink = {nullptr};
    std::shared_ptr<kk::BaseMapConfig> _baseMapConfig = {nullptr};
    std::unordered_map<std::string, std::shared_ptr<kk::BaseMapConfig>> _virtualBaseMapConfig = {};
    std::weak_ptr<kk::BaseMapConfig> _useBaseMapConfig;

    tgfx::Color _backgroundColor = {tgfx::Color::White()};
    BaseMapColorState _baseMapColorState = {BaseMapColorState::Original};
    bool _autoChangeBaseMapColorState = {true};
    bool _autoDrawSeat = {true};
    bool _firstFrameSubmitted = {false};
    bool _invalidate = {true};
    float _maxWidth = {1000.f};
    float _svgScale = {1.0f};
    float _svgModelScale = {1.0f};
    kk::SeatRenderMode _renderMode = {kk::SeatRenderMode::ZoomBased};
    kk::ZoomLevelConfig _zoomLevelConfig = {};
    uint32_t _minimapAnimationId = {0};
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRenderer_hpp */
