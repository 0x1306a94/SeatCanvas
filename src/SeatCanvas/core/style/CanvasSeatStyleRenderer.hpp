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

    std::unordered_map<std::string, tgfx::Rect> renderAllSeatStyles(
        tgfx::Canvas *canvas,
        const tgfx::Size &itemSize,
        int columns,
        int itemSpacing,
        float density,
        const std::unordered_map<std::string, std::shared_ptr<SeatStyleConfig>> &styleIdToConfig) override;

  private:
    void renderCircleStyle(tgfx::Canvas *canvas,
                           const tgfx::Size &itemSize,
                           const CircleSeatStyleConfig &config,
                           float density);

    void renderSVGStyle(tgfx::Canvas *canvas,
                        const tgfx::Size &itemSize,
                        const SVGSeatStyleConfig &config,
                        float density);
};

};  // namespace kk::renderer

#endif /* CanvasSeatStyleRenderer_hpp */
