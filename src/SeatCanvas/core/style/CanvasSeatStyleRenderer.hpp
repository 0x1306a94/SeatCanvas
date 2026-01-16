//
//  CanvasSeatStyleRenderer.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef CanvasSeatStyleRenderer_hpp
#define CanvasSeatStyleRenderer_hpp

#include "SeatStyleRenderer.hpp"

namespace kk::renderer {
class CircleSeatStyleConfig;
class SVGSeatStyleConfig;
class CanvasSeatStyleRenderer : public SeatStyleRenderer {
  public:
    CanvasSeatStyleRenderer();
    ~CanvasSeatStyleRenderer() override = default;

    std::unordered_map<kk::SeatStyleKey, tgfx::Rect> renderAllSeatStyles(
        tgfx::Canvas *canvas,
        const tgfx::Size &itemSize,
        int columns,
        int itemSpacing,
        float density,
        const std::unordered_map<kk::SeatStyleKey, std::shared_ptr<SeatStyleConfig>> &styleKeyToConfig) override;

  private:
    void renderCircleStyle(tgfx::Canvas *canvas,
                           const tgfx::Size &itemSize,
                           const CircleSeatStyleConfig &config,
                           bool selected,
                           float density);

    void renderSVGStyle(tgfx::Canvas *canvas,
                        const tgfx::Size &itemSize,
                        const SVGSeatStyleConfig &config,
                        float density);
};

};  // namespace kk::renderer

#endif /* CanvasSeatStyleRenderer_hpp */
