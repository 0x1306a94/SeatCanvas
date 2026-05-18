//
//  OHOSSeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#ifndef OHOSSeatCanvasCoreRendererDelegate_hpp
#define OHOSSeatCanvasCoreRendererDelegate_hpp

#include <napi/native_api.h>
#include <string>

#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace kk::js {

class OHOSSeatCanvasCoreRendererDelegate : public kk::renderer::SeatCanvasCoreRendererDelegate {
  public:
    OHOSSeatCanvasCoreRendererDelegate();
    virtual ~OHOSSeatCanvasCoreRendererDelegate();
    void didTapZone(uint32_t coreID, const std::string &zoneId) override;
    bool styleIdForSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId,
                        std::string &outStyleId) override;
    bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) override;
    void viewportWillBeginDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidScroll(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidEndDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) override;
    void viewportDidEndDecelerating(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportWillBeginZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidZoom(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidEndZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;
    void viewportDidEndScrollingAnimation(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) override;

    void setDidTapZoneCallback(napi_env env, napi_value callback);
    void setStyleIdForSeatCallback(napi_env env, napi_value callback);
    void setDidTapSeatCallback(napi_env env, napi_value callback);
    void setViewportWillBeginDraggingCallback(napi_env env, napi_value callback);
    void setViewportDidScrollCallback(napi_env env, napi_value callback);
    void setViewportDidEndDraggingCallback(napi_env env, napi_value callback);
    void setViewportDidEndDeceleratingCallback(napi_env env, napi_value callback);
    void setViewportWillBeginZoomingCallback(napi_env env, napi_value callback);
    void setViewportDidZoomCallback(napi_env env, napi_value callback);
    void setViewportDidEndZoomingCallback(napi_env env, napi_value callback);
    void setViewportDidEndScrollingAnimationCallback(napi_env env, napi_value callback);

  private:
    void setCallback(napi_env env, napi_value callback, napi_ref &ref);
    void clearCallback(napi_env env, napi_ref &ref);
    napi_value makeViewportValue(napi_env env, const kk::renderer::SeatCanvasViewportEvent &event);
    void callViewportCallback(napi_ref ref, const kk::renderer::SeatCanvasViewportEvent &event);
    void callViewportCallback(napi_ref ref, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate);

  private:
    napi_ref _didTapZone = nullptr;
    napi_ref _styleIdForSeat = nullptr;
    napi_ref _didTapSeat = nullptr;
    napi_ref _viewportWillBeginDragging = nullptr;
    napi_ref _viewportDidScroll = nullptr;
    napi_ref _viewportDidEndDragging = nullptr;
    napi_ref _viewportDidEndDecelerating = nullptr;
    napi_ref _viewportWillBeginZooming = nullptr;
    napi_ref _viewportDidZoom = nullptr;
    napi_ref _viewportDidEndZooming = nullptr;
    napi_ref _viewportDidEndScrollingAnimation = nullptr;
};

};  // namespace kk::js

#endif /* OHOSSeatCanvasCoreRendererDelegate_hpp */
