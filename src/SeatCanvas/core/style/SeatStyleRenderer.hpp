//
//  SeatStyleRenderer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef SeatStyleRenderer_hpp
#define SeatStyleRenderer_hpp

#include "SeatStyleConfig.hpp"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace tgfx {
class Canvas;
class Rect;
class Size;
};  // namespace tgfx

namespace kk::renderer {

/**
 * 座位样式渲染器接口
 * 用于抽象不同场馆的座位样式渲染方式（Canvas 绘制或 SVG 渲染）
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
     * @param styleIdToConfig 样式 ID 到配置的映射
     * @return 返回每个样式 ID 对应的渲染区域（在 Canvas 坐标系中）
     */
    virtual std::unordered_map<std::string, tgfx::Rect> renderAllSeatStyles(
        tgfx::Canvas *canvas,
        const tgfx::Size &itemSize,
        int columns,
        int itemSpacing,
        float density,
        const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig) = 0;
};

};  // namespace kk::renderer

#endif /* SeatStyleRenderer_hpp */
