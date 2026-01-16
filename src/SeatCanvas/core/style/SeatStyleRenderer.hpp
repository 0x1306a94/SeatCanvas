//
//  SeatStyleRenderer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SeatStyleRenderer_hpp
#define SeatStyleRenderer_hpp

#include "SeatStyleConfig.hpp"
#include "SeatStyleKey.hpp"

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace tgfx {
class Canvas;
class Rect;
class Size;
};  // namespace tgfx

namespace kk::renderer {

/**
 * 座位样式渲染器接口
 * 用于抽象不同场馆的座位样式渲染方式（Canvas 绘制或 SVG 渲染）
 *
 * 渲染器根据配置的类型自动选择合适的渲染方式
 */
class SeatStyleRenderer {
  public:
    virtual ~SeatStyleRenderer() = default;

    /**
     * 一次性渲染指定的座位样式到 Canvas
     * @param canvas Canvas
     * @param itemSize 每个座位的大小
     * @param columns Atlas 的列数
     * @param itemSpacing 座位之间的间距
     * @param density 设备密度
     * @param styleKeyToConfig 样式键到配置的映射，用于为不同状态+选中组合提供不同的样式配置
     * @return 返回每个状态组合对应的渲染区域（在 Canvas 坐标系中）
     */
    virtual std::unordered_map<kk::SeatStyleKey, tgfx::Rect> renderAllSeatStyles(
        tgfx::Canvas *canvas,
        const tgfx::Size &itemSize,
        int columns,
        int itemSpacing,
        float density,
        const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleKeyToConfig) = 0;
};

};  // namespace kk::renderer

#endif /* SeatStyleRenderer_hpp */
