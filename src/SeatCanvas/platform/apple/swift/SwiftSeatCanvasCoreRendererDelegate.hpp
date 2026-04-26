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
    virtual void didTapZone(uint32_t coreID, const std::string &zoneId);
    virtual bool styleIdForSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId, std::string &outStyleId);
    virtual bool didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId);
};

};  // namespace kk::bridge

#endif /* SwiftSeatCanvasCoreRendererDelegate_hpp */
