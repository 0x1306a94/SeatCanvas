//
//  SeatCanvasCoreRendererState.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SeatCanvasCoreRendererState_hpp
#define SeatCanvasCoreRendererState_hpp

#include <tgfx/core/Point.h>
#include <tgfx/core/Size.h>

namespace kk::renderer {
class SeatCanvasCoreRendererState {
  public:
    explicit SeatCanvasCoreRendererState(int width = 1280, int height = 720, float density = 1.0f);
    ~SeatCanvasCoreRendererState();

    /// canvas 尺寸
    tgfx::Size getBoundsSize() const;
    /// 内容尺寸
    tgfx::Size getContentSize() const;

    /// svg原始尺寸
    tgfx::Size getOriginSize() const;

    float density() const;

    /// 当前缩放比例
    float zoomScale() const;

    /// 当前滑动偏移
    const tgfx::Point &contentOffset() const;

    bool updateContentSize(const tgfx::Size &contentSize);
    bool updateOriginSize(const tgfx::Size &originSize);
    bool updateScreen(int width, int height, float density);
    bool updateZoomAndOffset(float zoomScale, const tgfx::Point &contentOffset);

  private:
    /// canvas 尺寸
    int _width = 1280;
    int _height = 720;
    float _density = 1.0f;
    float _zoomScale = 1.0f;
    /// 内容尺寸
    tgfx::Size _contentSize = {};
    /// 原始尺寸
    tgfx::Size _originSize = {};
    tgfx::Point _contentOffset = {};
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRendererState_hpp */
