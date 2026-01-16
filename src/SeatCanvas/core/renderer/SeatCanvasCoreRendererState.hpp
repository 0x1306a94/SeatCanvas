//
//  SeatCanvasCoreRendererState.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SeatCanvasCoreRendererState_hpp
#define SeatCanvasCoreRendererState_hpp

#include <tgfx/core/Matrix.h>
#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Size.h>

namespace kk::renderer {
class SeatCanvasCoreRendererState {
  public:
    explicit SeatCanvasCoreRendererState(int width = 1280, int height = 720, float density = 1.0f);
    ~SeatCanvasCoreRendererState();

    /// canvas 尺寸
    tgfx::Size getBoundsSize() const;
    /// 规范化后的内容尺寸（按固定宽度基准缩放，避免过大缩放级别）
    tgfx::Size getNormalizedContentSize() const;

    /// 原始尺寸（底图数据的原始尺寸）
    tgfx::Size getOriginSize() const;

    float getDensity() const;

    /// 内容缩放比例
    float getContentScale() const;

    /// 当前缩放比例
    float getZoomScale() const;

    /// 当前内容偏移
    const tgfx::Point &getContentOffset() const;

    /// 获取可见的原始矩形区域（在原始坐标系中）
    /// 根据当前的视口、缩放和偏移计算可见区域
    /// @return 原始坐标系中的可见矩形，如果无效则返回空矩形
    tgfx::Rect getVisibleOriginalRect() const;

    /// 计算并返回 MVP 矩阵（Model-View-Projection）
    /// 将原始坐标转换为 NDC 坐标 [-1, 1]
    /// 变换链：原始坐标 → 规范化内容坐标 → 屏幕坐标 → NDC坐标
    /// @return MVP 矩阵，如果状态无效则返回单位矩阵
    tgfx::Matrix getMVPMatrix() const;

    bool updateNormalizedContentSize(const tgfx::Size &normalizedContentSize);
    bool updateOriginSize(const tgfx::Size &originSize);
    bool updateContentScale(float scale);
    bool updateScreen(int width, int height, float density);
    bool updateZoomAndOffset(float zoomScale, const tgfx::Point &contentOffset);

  private:
    /// canvas 尺寸
    int width = 1280;
    int height = 720;
    float contentScale = 1.0f;
    float density = 1.0f;
    float zoomScale = 1.0f;
    /// 规范化后的内容尺寸（按固定宽度基准缩放）
    tgfx::Size normalizedContentSize = {};
    /// 原始尺寸（底图数据的原始尺寸）
    tgfx::Size originSize = {};
    tgfx::Point contentOffset = {};
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRendererState_hpp */
