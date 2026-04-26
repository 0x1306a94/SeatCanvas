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

bool SwiftSeatCanvasCoreRendererDelegate::styleIdForSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId, std::string &outStyleId) {
    constexpr size_t kMaxStyleIdLength = 1024;
    char buffer[kMaxStyleIdLength] = {0};
    auto found = ::switf_bridge_styleIdForSeat(coreID, zoneId.c_str(), seatId.c_str(), buffer, kMaxStyleIdLength);
    if (!found || buffer[0] == '\0') {
        return false;
    }
    outStyleId = buffer;
    return true;
}

bool SwiftSeatCanvasCoreRendererDelegate::didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    return ::switf_bridge_didTapSeat(coreID, zoneId.c_str(), seatId.c_str());
}

};  // namespace kk::bridge
