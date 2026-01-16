//
//  SeatOverlayLayerTree.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SeatOverlayLayerTree_hpp
#define SeatOverlayLayerTree_hpp

#include "Drawer.hpp"

#include <memory>

#include <tgfx/core/Size.h>
#include <tgfx/layers/DisplayList.h>

#include "core/EdgeInsets.h"

namespace tgfx {
class SVGDOM;
class Layer;
class ShapeLayer;
class ImageLayer;
class DisplayList;
class Image;
};  // namespace tgfx

namespace kk::layer {
class BaseMapRootLayer;
};  // namespace kk::layer

namespace kk::drawers {
class SeatOverlayLayerTree : public kk::drawers::Drawer {
  public:
    explicit SeatOverlayLayerTree();

    ~SeatOverlayLayerTree() override;

    void setBaseMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &baseMapSize);

    void invalidateAreaCacheImage();
    bool hasContentChanged() const;
    void prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) override;
    void setBackVisible(bool visible);
    bool backVisible() const {
        return _backVisible;
    }
    void setBackAlpha(float alpha);
    float backAlpha() const {
        return _backAlpha;
    }

    void setMinimapVisible(bool visible);
    bool minimapVisible() const {
        return _minimapVisible;
    }
    void setMinimapAlpha(float alpha);
    float minimapAlpha() const {
        return _minimapAlpha;
    }

    bool hitTestInBack(const tgfx::Point &location) const;
    bool hitTestInMinimap(const tgfx::Point &location) const;

  protected:
    void updateLineBox(const kk::renderer::SeatCanvasCoreRendererState *state);
    bool updateContainerSize(const kk::renderer::SeatCanvasCoreRendererState *state);
    bool prebuildAreaImage(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    std::shared_ptr<tgfx::Layer> buildLayerTree(const kk::renderer::SeatCanvasCoreRendererState *state);
    std::shared_ptr<tgfx::ShapeLayer> buildBackLayer(const kk::renderer::SeatCanvasCoreRendererState *state);
    void onVisible(bool visible) override;
    void onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) override;

  private:
    tgfx::Size _viewSize = {};
    tgfx::Size _containerSize = {};
    tgfx::Size _baseMapSize = {};
    kk::EdgeInsets _inset = {};
    bool _rebuildAreaCacheImage = {true};
    std::shared_ptr<tgfx::Layer> _root = {nullptr};
    std::shared_ptr<kk::layer::BaseMapRootLayer> _baseMapLayer = {nullptr};
    std::shared_ptr<tgfx::ShapeLayer> _backLayer = {nullptr};
    std::shared_ptr<tgfx::ShapeLayer> _minimapContainer = {nullptr};
    std::shared_ptr<tgfx::ImageLayer> _imageLayer = {nullptr};
    std::shared_ptr<tgfx::ShapeLayer> _lineBox = {nullptr};
    tgfx::DisplayList _displayList = {};
    std::shared_ptr<tgfx::Image> _areaCacheImage = {nullptr};
    bool _backVisible = {false};
    float _backAlpha = {1.0f};
    bool _minimapVisible = {false};
    float _minimapAlpha = {1.0f};
};
};  // namespace kk::drawers

#endif /* SeatOverlayLayerTree_hpp */
