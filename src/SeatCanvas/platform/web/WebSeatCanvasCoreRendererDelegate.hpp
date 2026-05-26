//
//  WebSeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

#ifndef WebSeatCanvasCoreRendererDelegate_hpp
#define WebSeatCanvasCoreRendererDelegate_hpp

#include <cstdint>
#include <string>

#include <emscripten/val.h>

#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"
#include "core/renderer/SeatCanvasCoreRendererEvent.hpp"

namespace kk::web {
class WebSeatCanvasCoreRendererDelegate : public kk::renderer::SeatCanvasCoreRendererDelegate {
  public:
    WebSeatCanvasCoreRendererDelegate();
    ~WebSeatCanvasCoreRendererDelegate() override;

    void didLoadBaseMap(uint32_t coreID) override;
    void didUnloadBaseMap(uint32_t coreID) override;
    void didUpdateZoomLevelConfig(uint32_t coreID, const kk::renderer::SeatCanvasZoomLevelConfigEvent &event) override;
    void didTapZone(uint32_t coreID, const std::string &zoneId) override;
    bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) override;

    void viewportWillBeginDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidScroll(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidEndDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) override;
    void viewportDidEndDecelerating(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportWillBeginZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidZoom(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidEndZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidEndScrollingAnimation(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;

    void setDidLoadBaseMapCallback(emscripten::val callback);
    void setDidUnloadBaseMapCallback(emscripten::val callback);
    void setDidUpdateZoomLevelConfigCallback(emscripten::val callback);
    void setDidTapZoneCallback(emscripten::val callback);
    void setDidTapSeatCallback(emscripten::val callback);
    void setViewportWillBeginDraggingCallback(emscripten::val callback);
    void setViewportDidScrollCallback(emscripten::val callback);
    void setViewportDidEndDraggingCallback(emscripten::val callback);
    void setViewportDidEndDeceleratingCallback(emscripten::val callback);
    void setViewportWillBeginZoomingCallback(emscripten::val callback);
    void setViewportDidZoomCallback(emscripten::val callback);
    void setViewportDidEndZoomingCallback(emscripten::val callback);
    void setViewportDidEndScrollingAnimationCallback(emscripten::val callback);

  private:
    emscripten::val makeZoomLevelConfigEventValue(const kk::renderer::SeatCanvasZoomLevelConfigEvent &event);
    emscripten::val makeViewportValue(const kk::renderer::SeatCanvasViewportEvent &event);
    void callViewportCallback(emscripten::val &ref, const kk::renderer::SeatCanvasViewportEvent &event);
    void callViewportCallback(emscripten::val &ref, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate);

    bool isFunction(const emscripten::val &v) const;

    emscripten::val _didTapZone = {emscripten::val::undefined()};
    emscripten::val _didTapSeat = {emscripten::val::undefined()};
    emscripten::val _viewportWillBeginDragging = {emscripten::val::undefined()};
    emscripten::val _viewportDidScroll = {emscripten::val::undefined()};
    emscripten::val _viewportDidEndDragging = {emscripten::val::undefined()};
    emscripten::val _viewportDidEndDecelerating = {emscripten::val::undefined()};
    emscripten::val _viewportWillBeginZooming = {emscripten::val::undefined()};
    emscripten::val _viewportDidZoom = {emscripten::val::undefined()};
    emscripten::val _viewportDidEndZooming = {emscripten::val::undefined()};
    emscripten::val _viewportDidEndScrollingAnimation = {emscripten::val::undefined()};
    emscripten::val _didLoadBaseMap = {emscripten::val::undefined()};
    emscripten::val _didUnloadBaseMap = {emscripten::val::undefined()};
    emscripten::val _didUpdateZoomLevelConfig = {emscripten::val::undefined()};
};
};  // namespace kk::web

#endif /* WebSeatCanvasCoreRendererDelegate_hpp */
