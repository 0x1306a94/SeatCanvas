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
    virtual bool shouldSelectSeat(uint32_t coreID, const std::string &regionId, const std::string &seatId);
    virtual void didSelectSeat(uint32_t coreID, const std::string &regionId, const std::string &seatId);
    virtual void didDeselectSeat(uint32_t coreID, const std::string &regionId, const std::string &seatId);

  private:
    kk::jni::Global<jobject> _seatCanvasView;
};

};  // namespace kk::renderer

#endif /* AndroidSeatCanvasCoreRendererDelegate_hpp */
