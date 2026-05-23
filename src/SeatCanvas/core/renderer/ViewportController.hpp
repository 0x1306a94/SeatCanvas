//
//  ViewportController.hpp
//  SeatCanvas
//
//  Created by king on 2026/05/23.
//

#ifndef ViewportController_hpp
#define ViewportController_hpp

#include <memory>

#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>

#include "core/BaseMapColorState.h"
#include "core/ZoomLevelConfig.hpp"
#include "core/renderer/SeatCanvasCoreRendererEvent.hpp"
#include "core/renderer/ViewportControllerCallback.hpp"

namespace kk::animation {
class Animator;
};

namespace kk::gesture {
class ElasticZoomPanController;
};

namespace kk::renderer {

class SeatCanvasCoreRendererState;
class SeatCanvasCoreRendererDelegate;
struct ZoneMeshInfo;

class ViewportController {
  public:
    ViewportController(kk::gesture::ElasticZoomPanController *zoomPanController,
                       SeatCanvasCoreRendererState *state,
                       kk::animation::Animator *animator,
                       std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate,
                       kk::ZoomLevelConfig *zoomLevelConfig,
                       ViewportControllerCallback *callback);

    /// 更新 delegate（由 renderer 在 setDelegate 时同步调用）
    void setDelegate(std::shared_ptr<SeatCanvasCoreRendererDelegate> delegate);
    /// 更新 coreID（由 renderer 构造后同步调用）
    void setCoreID(uint32_t coreID);

    /// 缩放到指定区域并居中显示
    void zoomToRect(const tgfx::Rect &rect, bool animated, float padding, double durationMs);

    /// 点击空白/已有区域时的渐进式缩放
    void scrollViewWithLocation(const tgfx::Point &location, float seatRenderZoomThreshold);

    /// 缩放到指定区域
    void scrollViewWithZone(const std::shared_ptr<ZoneMeshInfo> &zoneInfo, float seatRenderZoomThreshold);

    /// 返回全局视图
    void handleZoomBack();

    // ---- 底图生命周期调用的配置方法 ----
    void updateContentScale(float maxWidth);
    void updateContentSize();

    // ---- 渲染循环调用的方法 ----
    void notifyViewportDidEndDeceleratingIfNeeded();

    // ---- 访问器 ----
    float getMaxWidth() const;
    void setMaxWidth(float maxWidth);
    kk::ZoomLevelConfig &zoomLevelConfig();

    bool isAutoDrawSeatDisabled() const;

    /// 动画进行中是否自动切换底图颜色，动画完成时恢复
    bool autoChangeBaseMapColorState = {true};

    bool isPanAnimationActive() const;
    void setPanAnimationActive(bool active);
    void resetAnimationState();
    bool isScrollingAnimationActive() const;

  private:
    float showBackZoomThreshold() const;
    bool isSmallVenue() const;

    void updateMaxMinZoomScalesForCurrentBounds();
    void notifyViewportDidEndScrollingAnimation();
    void beginViewportScrollingAnimation();
    SeatCanvasViewportEvent makeViewportEvent() const;
    void zoomToPoint(const tgfx::Point &location, float scale, bool animated, float padding, double durationMs, float seatRenderZoomThreshold);

    void stopAllViewportAnimations();
    void prepareScrollAnimationState(float currentZoomScale, float seatRenderZoomThreshold);
    bool setupShowBackAndOverlay(float targetZoomScale);

    kk::gesture::ElasticZoomPanController *_zoomPanController;
    SeatCanvasCoreRendererState *_state;
    kk::animation::Animator *_animator;
    std::shared_ptr<SeatCanvasCoreRendererDelegate> _delegate;
    kk::ZoomLevelConfig *_zoomLevelConfig;
    ViewportControllerCallback *_callback;

    float _maxWidth = {1000.f};
    float _svgModelScale = {1.0f};
    uint32_t _coreID = {0};
    bool _panAnimationActive = {false};
    bool _scrollingAnimationActive = {false};
    bool _disableAutoDrawSeat = {false};
};

};  // namespace kk::renderer

#endif /* ViewportController_hpp */
