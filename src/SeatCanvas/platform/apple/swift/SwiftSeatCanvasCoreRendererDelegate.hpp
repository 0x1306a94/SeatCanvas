//
//  SwiftSeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

#ifndef SwiftSeatCanvasCoreRendererDelegate_hpp
#define SwiftSeatCanvasCoreRendererDelegate_hpp

#import "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace kk::bridge {
class SwiftSeatCanvasCoreRendererDelegate : public kk::renderer::SeatCanvasCoreRendererDelegate {
  public:
    explicit SwiftSeatCanvasCoreRendererDelegate();
    virtual ~SwiftSeatCanvasCoreRendererDelegate();
    virtual void didLoadBaseMap(uint32_t coreID, const kk::renderer::SeatCanvasBaseMapLoadedEvent &event) override;
    virtual void didUnloadBaseMap(uint32_t coreID) override;
    virtual void didTapZone(uint32_t coreID, const std::string &zoneId) override;
    virtual bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) override;
    virtual void viewportWillBeginDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    virtual void viewportDidScroll(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    virtual void viewportDidEndDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) override;
    virtual void viewportDidEndDecelerating(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    virtual void viewportWillBeginZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    virtual void viewportDidZoom(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    virtual void viewportDidEndZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    virtual void viewportDidEndScrollingAnimation(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
};

};  // namespace kk::bridge

#endif /* SwiftSeatCanvasCoreRendererDelegate_hpp */
