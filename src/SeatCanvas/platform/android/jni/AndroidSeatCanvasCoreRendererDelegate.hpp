//
//  AndroidSeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2026/1/16.
//

#ifndef AndroidSeatCanvasCoreRendererDelegate_hpp
#define AndroidSeatCanvasCoreRendererDelegate_hpp

#include "JNIHelper.hpp"
#include "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace kk::renderer {
class AndroidSeatCanvasCoreRendererDelegate : public SeatCanvasCoreRendererDelegate {
  public:
    explicit AndroidSeatCanvasCoreRendererDelegate(jobject seatCanvasView);
    virtual ~AndroidSeatCanvasCoreRendererDelegate();
    virtual void didTapZone(uint32_t coreID, const std::string &zoneId) override;
    virtual bool styleIdForSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId, std::string &outStyleId) override;
    virtual bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) override;
    virtual void viewportWillBeginDragging(uint32_t coreID, const SeatCanvasViewportEvent &event) override;
    virtual void viewportDidScroll(uint32_t coreID, const SeatCanvasViewportEvent &event) override;
    virtual void viewportDidEndDragging(uint32_t coreID, const SeatCanvasViewportEvent &event, bool willDecelerate) override;
    virtual void viewportDidEndDecelerating(uint32_t coreID, const SeatCanvasViewportEvent &event) override;
    virtual void viewportWillBeginZooming(uint32_t coreID, const SeatCanvasViewportEvent &event) override;
    virtual void viewportDidZoom(uint32_t coreID, const SeatCanvasViewportEvent &event) override;
    virtual void viewportDidEndZooming(uint32_t coreID, const SeatCanvasViewportEvent &event) override;
    virtual void viewportDidEndScrollingAnimation(uint32_t coreID, const SeatCanvasViewportEvent &event) override;

  private:
    void callViewportMethod(const char *methodName, const SeatCanvasViewportEvent &event);
    void callViewportMethod(const char *methodName, const SeatCanvasViewportEvent &event, bool willDecelerate);

  private:
    kk::jni::Global<jobject> _seatCanvasView;
};

};  // namespace kk::renderer

#endif /* AndroidSeatCanvasCoreRendererDelegate_hpp */
