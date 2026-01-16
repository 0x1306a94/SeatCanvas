//
//  ElasticZoomPanController.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef ElasticZoomPanController_hpp
#define ElasticZoomPanController_hpp

#include <algorithm>
#include <optional>

#include <tgfx/core/Matrix.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Size.h>

#include "core/EdgeInsets.h"
#include "core/gesture/GestureState.hpp"

namespace kk::gesture {
class PanProxy;
class ScrollProperties;
class ElasticZoomPanController {
  public:
    ElasticZoomPanController();
    ~ElasticZoomPanController();

    /// 设置视窗范围
    /// - Parameter bounds: 视窗范围
    void setBounds(const tgfx::Size &bounds);

    /// 获取视窗范围
    const tgfx::Size &getBounds() const;

    /// 设置内容大小
    /// - Parameter contentSize: 内容大小
    void setContentSize(const tgfx::Size &contentSize);

    /// 获取内容大小
    const tgfx::Size &getContentSize() const;

    /// 设置内容边界
    /// - Parameter contentInset: 内容边界
    void setContentInset(const EdgeInsets &contentInset);

    /// 获取内容边界
    const EdgeInsets &getContentInset() const;

    void setMinimumZoomScale(float minimumZoomScale);

    float getMinimumZoomScale() const;

    void setMaximumZoomScale(float maximumZoomScale);

    float getMaximumZoomScale() const;

    void setZoomScale(float zoomScale, bool revalidate = true);

    float getZoomScale() const;

    void setContentOffset(const tgfx::Point &contentOffset, bool revalidate = true);

    const tgfx::Point &getContentOffset() const;

    // --- Transformation Matrix ---
    tgfx::Matrix getMatrix() const;

    // --- Gesture Handlers (Simulating User Input) ---

    /// 处理滑动手势
    /// - Parameters:
    ///   - state: 手势状态
    ///   - translation: 手势当前平移量
    ///   - timestampMs: 手势事件时间戳（毫秒）
    void handlePan(kk::gesture::GestureState state, const tgfx::Point &translation, double timestampMs);

    /// 处理捏合手势
    /// - Parameters:
    ///   - state: 手势状态
    ///   - scale: 当前缩放比例
    ///   - center: 缩放中心（捏合手势发送时的中心位置，始终是在viewport的坐标系中）
    void handlePinch(kk::gesture::GestureState state, float scale, const tgfx::Point &center);

    /// 处理显示链接的每一帧更新
    /// 用于驱动 Pan 的惯性滑动/回弹动画和 Pinch 的回弹动画
    /// - Returns: 如果任何动画状态发生变化（需要继续更新），返回 true；否则返回 false
    /// - Note: 这个方法应该在显示链接的每一帧中被调用，通常由外部渲染循环调用
    bool handleDisplayLinkFire();

    /// 清除所有动画状态（Pan 的惯性/回弹、Pinch 的回弹）
    /// 在某些业务逻辑下需要直接停止动画时调用
    void stopAllAnimations();

    /// 检查是否有待处理的动画（即将开始或正在进行的动画）
    /// - Returns: 如果有动画即将开始或正在进行，返回 true；否则返回 false
    bool hasPendingAnimation() const;

  protected:
    // --- Internal Logic ---
    void handlePanProxy();
    bool handleDisplayLinkFireForAxis(ScrollProperties *properties, bool horizontal);
    bool handleDisplayLinkFireForZoomScale();
    void handleEndPanWithVelocity(const tgfx::Point &velocity);
    void prepareBouncingForZoomScale(float targetScale);
    bool handleBouncingWithIntervalForZoomScale(ScrollProperties *properties, double interval);
    void handleEndPanWithVelocityForAxis(ScrollProperties *properties, float velocity, bool horizontal);
    void prepareBouncingWithVelocityForAxis(ScrollProperties *properties, float velocity, bool overflowVelocity, bool horizontal);
    bool handleDeceleratingWithIntervalForAxis(ScrollProperties *properties, double interval, float &velocity, bool horizontal);
    bool handleBouncingWithIntervalForAxis(ScrollProperties *properties, double interval, bool horizontal);
    void setContentOffsetValueForAxis(float offset, bool horizontal);
    float rubberBandForOffset(float offset, float minOffset, float maxOffset, float range, bool inverse) const;
    float zoomScaleForRubberBandScale(float scale) const;
    bool canHorizontalScroll() const;
    bool canVerticalScroll() const;
    bool canScrollForAxis(bool horizontal) const;
    float overflowOffsetForAxis(bool horizontal) const;
    float offsetForAxis(bool horizontal) const;
    float minimumOffsetForAxis(bool horizontal) const;
    float maximumOffsetForAxis(bool horizontal) const;

    const tgfx::Rect &getOffsetBounds() const;

    void updateOffsetBounds();

    void revalidateContentOffset();

  protected:
    // --- Private Members ---
    float _minimumZoomScale{1.0f};
    float _maximumZoomScale{1.0f};
    float _zoomScale{1.0f};
    tgfx::Size _bounds{0.0f, 0.0f};
    tgfx::Size _contentSize{0.0f, 0.0f};
    tgfx::Point _contentOffset{0.0f, 0.0f};
    tgfx::Point _lastContentOffset{0.0f, 0.0f};
    kk::EdgeInsets _contentInset{40.0f, 40.0f, 40.0f, 40.0f};
    tgfx::Rect _cachedOffsetBounds{0.0f, 0.0f, 0.0f, 0.0f};
    bool _isTracking{false};
    bool _isDragging{false};
    bool _alwaysBounceHorizontal{true};
    bool _alwaysBounceVertical{true};
    float _pinchStartZoomScale{1.0f};
    tgfx::Point _pinchEndContentPoint{0.0f, 0.0f};
    tgfx::Point _pinchEndCenter{0.0f, 0.0f};
    float _zoomBounceStartScale{1.0f};
    float _zoomBounceTargetScale{1.0f};

    std::optional<tgfx::Point> _touchBeganTranslation{std::nullopt};
    std::unique_ptr<PanProxy> _panProxy{nullptr};
    std::unique_ptr<ScrollProperties> _scrollPropertiesX{nullptr};
    std::unique_ptr<ScrollProperties> _scrollPropertiesY{nullptr};
    std::unique_ptr<ScrollProperties> _zoomScaleProperties{nullptr};
};

};  // namespace kk::gesture

#endif /* ElasticZoomPanController_hpp */
