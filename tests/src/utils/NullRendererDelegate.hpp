#ifndef NullRendererDelegate_hpp
#define NullRendererDelegate_hpp

#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace kk::test {

class NullRendererDelegate : public kk::renderer::SeatCanvasCoreRendererDelegate {
  public:
    void didLoadBaseMap(uint32_t) override {
    }

    void didUnloadBaseMap(uint32_t) override {
    }

    void didUpdateZoomLevelConfig(uint32_t, const kk::renderer::SeatCanvasZoomLevelConfigEvent &) override {
    }

    void didTapZone(uint32_t, const std::string &) override {
    }

    bool didTapSeat(uint32_t, const std::string &, const std::string &) override {
        return false;
    }

    void viewportWillBeginDragging(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }

    void viewportDidScroll(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }

    void viewportDidEndDragging(uint32_t, const kk::renderer::SeatCanvasViewportEvent &, bool) override {
    }

    void viewportDidEndDecelerating(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }

    void viewportWillBeginZooming(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }

    void viewportDidZoom(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }

    void viewportDidEndZooming(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }

    void viewportDidEndScrollingAnimation(uint32_t, const kk::renderer::SeatCanvasViewportEvent &) override {
    }
};

};  // namespace kk::test

#endif /* NullRendererDelegate_hpp */
