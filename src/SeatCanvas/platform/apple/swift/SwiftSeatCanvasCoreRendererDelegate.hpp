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
    virtual bool shouldSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);
    virtual void didSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);
    virtual void didDeselectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);
};

};  // namespace kk::bridge

#endif /* SwiftSeatCanvasCoreRendererDelegate_hpp */
