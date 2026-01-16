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
class SeatCanvasCoreRendererDelegate {
  public:
    virtual ~SeatCanvasCoreRendererDelegate() = default;
    virtual bool shouldSelectSeat(uint32_t coreID, const std::string &regionId, const std::string &seatId) = 0;
    virtual void didSelectSeat(uint32_t coreID, const std::string &regionId, const std::string &seatId) = 0;
    virtual void didDeselectSeat(uint32_t coreID, const std::string &regionId, const std::string &seatId) = 0;
};
};  // namespace kk::renderer

#endif /* SeatCanvasCoreRendererDelegate_hpp */
