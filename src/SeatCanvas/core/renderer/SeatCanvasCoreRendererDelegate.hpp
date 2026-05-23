//
//  SeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

#ifndef SeatCanvasCoreRendererDelegate_hpp
#define SeatCanvasCoreRendererDelegate_hpp

#include <cstdint>
#include <string>

namespace kk::renderer {
struct SeatCanvasViewportEvent;
struct SeatCanvasZoomLevelConfigEvent;

class SeatCanvasCoreRendererDelegate {
  public:
    virtual ~SeatCanvasCoreRendererDelegate() = default;
    virtual void didLoadBaseMap(uint32_t) = 0;
    virtual void didUnloadBaseMap(uint32_t) = 0;
    virtual void didUpdateZoomLevelConfig(uint32_t, const SeatCanvasZoomLevelConfigEvent &) = 0;

    virtual void didTapZone(uint32_t coreID, const std::string &zoneId) = 0;
    virtual bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) = 0;

    virtual void viewportWillBeginDragging(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidScroll(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidEndDragging(uint32_t, const SeatCanvasViewportEvent &, bool) = 0;
    virtual void viewportDidEndDecelerating(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportWillBeginZooming(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidZoom(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidEndZooming(uint32_t, const SeatCanvasViewportEvent &) = 0;
    virtual void viewportDidEndScrollingAnimation(uint32_t, const SeatCanvasViewportEvent &) = 0;
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRendererDelegate_hpp */
