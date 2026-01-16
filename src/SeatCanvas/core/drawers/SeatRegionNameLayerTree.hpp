//
//  SeatRegionNameLayerTree.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SeatRegionNameLayerTree_hpp
#define SeatRegionNameLayerTree_hpp

#include "Drawer.hpp"

#include <memory>

#include <tgfx/core/Point.h>
#include <tgfx/core/Size.h>

namespace tgfx {
class Layer;
class DisplayList;
};  // namespace tgfx

namespace kk::drawers {
class SeatRegionNameLayerTree : public kk::drawers::Drawer {
  public:
    explicit SeatRegionNameLayerTree();

    virtual ~SeatRegionNameLayerTree();

    void setTextRootLayer(std::shared_ptr<tgfx::Layer> layer, const tgfx::Size &baseMapSize);

    tgfx::Point globalToLocal(const tgfx::Point &globalPoint) const;
    tgfx::Point localToGlobal(const tgfx::Point &localPoint) const;

    bool hasContentChanged() const;

    virtual void prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) override;

  protected:
    /// 更新内容尺寸
    /// - Parameter app: app
    bool updateContentSize(const kk::renderer::SeatCanvasCoreRendererState *state);
    void buildLayerTree(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    void updateRootMatrix(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    void onVisible(bool visible) override;
    void onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) override;

  private:
    // 限制的内容尺寸，无论svg原始尺寸多大。都不能超过这个内容尺寸，如果超过了则等比例缩放
    tgfx::Size _contentSize{};
    // 底图的原始尺寸
    tgfx::Size _baseMapSize{};
    std::shared_ptr<tgfx::Layer> _root = {nullptr};
    std::shared_ptr<tgfx::Layer> _textRootLayer = {nullptr};
    std::unique_ptr<tgfx::DisplayList> _displayList = {nullptr};
    bool _rebuildLayer = {false};
};
};  // namespace kk::drawers

#endif /* SeatRegionNameLayerTree_hpp */
