//
//  SwiftSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

#import "SwiftSeatCanvasCoreRendererDelegate.hpp"

#import <UIKit/UIColor.h>

#import <tgfx/platform/Print.h>

#import "SwiftBridgeCAPI.h"

namespace kk::bridge {
SwiftSeatCanvasCoreRendererDelegate::SwiftSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

SwiftSeatCanvasCoreRendererDelegate::~SwiftSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void SwiftSeatCanvasCoreRendererDelegate::didTapZone(uint32_t coreID, const std::string &zoneId) {
    ::switf_bridge_didTapZone(coreID, zoneId.c_str());
}

bool SwiftSeatCanvasCoreRendererDelegate::shouldSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    return ::switf_bridge_shouldSelectSeat(coreID, zoneId.c_str(), seatId.c_str());
}

void SwiftSeatCanvasCoreRendererDelegate::didSelectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    ::switf_bridge_didSelectSeat(coreID, zoneId.c_str(), seatId.c_str());
}

void SwiftSeatCanvasCoreRendererDelegate::didDeselectSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    ::switf_bridge_didDeselectSeat(coreID, zoneId.c_str(), seatId.c_str());
}

};  // namespace kk::bridge
