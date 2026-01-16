//
//  SeatLayerTree.hpp
//  SeatCanvas
//
//  Created by king on 2025/11/12.
//

#ifndef SeatLayerTree_hpp
#define SeatLayerTree_hpp

#include "Drawer.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include <tgfx/core/Point.h>
#include <tgfx/core/Size.h>

namespace tgfx {
class Layer;
class DisplayList;
class Image;
};  // namespace tgfx

namespace kk::layer {
class BaseMapRootLayer;
class SeatRootLayer;
class SeatRegionLayer;
class SeatAtlasLayer;
class SeatItemLayer;
};  // namespace kk::layer

namespace kk::drawers {
class SeatLayerTree : public kk::drawers::Drawer {
  public:
    explicit SeatLayerTree();

    virtual ~SeatLayerTree();

    void setBaseMapLayer(std::shared_ptr<kk::layer::BaseMapRootLayer> layer, const tgfx::Size &baseMapSize);

    void invalidateSeatStatusImage();

    void clearSeatAtlas();

    void hiddenSeatAtlas();

    void hiddenSeatAtlas(const std::unordered_map<std::string, bool> visibleMap);

    std::shared_ptr<kk::layer::SeatAtlasLayer> getSeatAtlasLayerOrCreate(const std::string &regionId, std::function<void(kk::layer::SeatAtlasLayer *)> onCreate = nullptr);
    void removeSeateAtlas(const std::string &regionId);

    void enableTiled(bool enable);
    void enableZoomBlur(bool enable);
    void enableLoadSeat(bool enable);

    tgfx::Point globalToLocal(const tgfx::Point &globalPoint) const;
    tgfx::Point localToGlobal(const tgfx::Point &localPoint) const;

    std::optional<std::string> getSeatRegionIdAt(float x, float y) const;

    std::shared_ptr<kk::layer::SeatAtlasLayer> getSeatAtlasAt(float x, float y) const;

    std::shared_ptr<kk::layer::SeatItemLayer> getSeatItemAt(float x, float y) const;

    bool hasContentChanged() const;

    bool hitTestPoint(const tgfx::Point &location) const;

    virtual void prepare(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state, bool force) override;

  protected:
    /// 更新内容尺寸
    /// - Parameter app: app
    bool updateContentSize(const kk::renderer::SeatCanvasCoreRendererState *state);
    bool prebuildBaseMapImage(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    bool prebuildSeatStatusImage(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    void loadSeatIfneeded(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    void buildLayerTree(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    void updateRootMatrix(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state);
    void onVisible(bool visible) override;
    void onDraw(tgfx::Canvas *canvas, const kk::renderer::SeatCanvasCoreRendererState *state) override;

  private:
    // 限制的内容尺寸，无论svg原始尺寸多大。都不能超过这个内容尺寸，如果超过了则等比例缩放
    tgfx::Size _contentSize{};
    // 底图的原始尺寸
    tgfx::Size _baseMapSize{};
    std::shared_ptr<kk::layer::BaseMapRootLayer> _baseMapLayer{nullptr};
    std::shared_ptr<tgfx::Image> _baseMapImage{nullptr};
    std::shared_ptr<tgfx::Layer> _root{nullptr};
    std::shared_ptr<kk::layer::SeatRootLayer> _seatRootLayer{nullptr};
    std::unordered_map<std::string, std::shared_ptr<kk::layer::SeatAtlasLayer>> _seatAtlasLayers = {};
    std::unique_ptr<tgfx::DisplayList> _displayList{nullptr};
    bool _rebuildLayer{true};
    bool _enableTiled{false};
    bool _enableZoomBlur{false};
    bool _enableLoadSeat{false};
};
};  // namespace kk::drawers

#endif /* SeatLayerTree_hpp */
