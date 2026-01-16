//
//  SwiftSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

#import "SwiftSeatCanvasCoreRendererDelegate.hpp"

#import <tgfx/platform/Print.h>

#import "SeatCanvas-Bridge-swift.h"

#import <swift/bridging>

namespace kk::renderer {
SwiftSeatCanvasCoreRendererDelegate::SwiftSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

SwiftSeatCanvasCoreRendererDelegate::~SwiftSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

bool SwiftSeatCanvasCoreRendererDelegate::shouldSelectSeat(uint32_t coreID, const std::string &seatId) {
    return SeatCanvas::switf_bridge_shouldSelectSeat(coreID, seatId);
}

void SwiftSeatCanvasCoreRendererDelegate::didSelectSeat(uint32_t coreID, const std::string &seatId) {
    SeatCanvas::switf_bridge_didSelectSeat(coreID, seatId);
}

void SwiftSeatCanvasCoreRendererDelegate::didDeselectSeat(uint32_t coreID, const std::string &seatId) {
    SeatCanvas::switf_bridge_didDeselectSeat(coreID, seatId);
}
};  // namespace kk::renderer
