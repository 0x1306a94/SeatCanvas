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

#include <tgfx/core/Point.h>
#include <tgfx/core/Rect.h>
#include <tgfx/core/Size.h>

#include "core/ZoomLevelConfig.hpp"

namespace kk::renderer {
struct SeatCanvasViewportEvent {
    float zoomScale = 1.0f;
    tgfx::Point contentOffset = {};
    tgfx::Rect visibleOriginalRect = {};
};

struct SeatCanvasBaseMapLoadedEvent {
    tgfx::Size baseMapSize = {};
    kk::ZoomLevelConfig zoomLevels = {};
    float minimumZoomScale = 1.0f;
    float maximumZoomScale = 1.0f;
    float zoomScale = 1.0f;
    tgfx::Rect visibleOriginalRect = {};
};

class SeatCanvasCoreRendererDelegate {
  public:
    virtual ~SeatCanvasCoreRendererDelegate() = default;
    virtual void didLoadBaseMap(uint32_t, const SeatCanvasBaseMapLoadedEvent &) = 0;
    virtual void didUnloadBaseMap(uint32_t) = 0;
    virtual void didTapZone(uint32_t coreID, const std::string &zoneId) = 0;
    virtual bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) = 0;

    virtual void viewportWillBeginDragging(uint32_t, const SeatCanvasViewportEvent &) {
    }
    virtual void viewportDidScroll(uint32_t, const SeatCanvasViewportEvent &) {
    }
    virtual void viewportDidEndDragging(uint32_t, const SeatCanvasViewportEvent &, bool) {
    }
    virtual void viewportDidEndDecelerating(uint32_t, const SeatCanvasViewportEvent &) {
    }

    virtual void viewportWillBeginZooming(uint32_t, const SeatCanvasViewportEvent &) {
    }
    virtual void viewportDidZoom(uint32_t, const SeatCanvasViewportEvent &) {
    }
    virtual void viewportDidEndZooming(uint32_t, const SeatCanvasViewportEvent &) {
    }

    virtual void viewportDidEndScrollingAnimation(uint32_t, const SeatCanvasViewportEvent &) {
    }
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRendererDelegate_hpp */
