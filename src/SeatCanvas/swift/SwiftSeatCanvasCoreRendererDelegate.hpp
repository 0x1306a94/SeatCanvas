//
//  SwiftSeatCanvasCoreRendererDelegate.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

#ifndef SwiftSeatCanvasCoreRendererDelegate_hpp
#define SwiftSeatCanvasCoreRendererDelegate_hpp

#import "core/renderer/SeatCanvasCoreRendererDelegate.hpp"

namespace kk::renderer {
class SwiftSeatCanvasCoreRendererDelegate : public SeatCanvasCoreRendererDelegate {
  public:
    explicit SwiftSeatCanvasCoreRendererDelegate();
    virtual ~SwiftSeatCanvasCoreRendererDelegate();
    virtual bool shouldSelectSeat(uint32_t coreID, const std::string &seatId);
    virtual void didSelectSeat(uint32_t coreID, const std::string &seatId);
    virtual void didDeselectSeat(uint32_t coreID, const std::string &seatId);
};

};  // namespace kk::renderer

#endif /* SwiftSeatCanvasCoreRendererDelegate_hpp */
