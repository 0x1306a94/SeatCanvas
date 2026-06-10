//
//  SwiftSeatCanvasCoreRendererDelegate.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

#import "SwiftSeatCanvasCoreRendererDelegate.hpp"

#import <tgfx/platform/Print.h>

#import "SwiftBridgeCAPI.h"

namespace kk::bridge {
SwiftSeatCanvasCoreRendererDelegate::SwiftSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

SwiftSeatCanvasCoreRendererDelegate::~SwiftSeatCanvasCoreRendererDelegate() {
    tgfx::PrintLog("%s", __PRETTY_FUNCTION__);
}

void SwiftSeatCanvasCoreRendererDelegate::didLoadBaseMap(uint32_t coreID) {
    ::switf_bridge_didLoadBaseMap(coreID);
}

void SwiftSeatCanvasCoreRendererDelegate::didUnloadBaseMap(uint32_t coreID) {
    ::switf_bridge_didUnloadBaseMap(coreID);
}

void SwiftSeatCanvasCoreRendererDelegate::didUpdateZoomLevelConfig(uint32_t coreID, const kk::renderer::SeatCanvasZoomLevelConfigEvent &event) {
    ::switf_bridge_didUpdateZoomLevelConfig(coreID,
                                            event.zoomLevels.seat,
                                            event.zoomLevels.row,
                                            event.zoomLevels.zone,
                                            event.zoomLevels.venue,
                                            event.minimumZoomScale,
                                            event.maximumZoomScale,
                                            event.zoomScale);
}

void SwiftSeatCanvasCoreRendererDelegate::didTapZone(uint32_t coreID, const std::string &zoneId) {
    ::switf_bridge_didTapZone(coreID, zoneId.c_str());
}

bool SwiftSeatCanvasCoreRendererDelegate::didTapSeat(uint32_t coreID, const std::string &zoneId, const std::string &seatId) {
    return ::switf_bridge_didTapSeat(coreID, zoneId.c_str(), seatId.c_str());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportWillBeginDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportWillBeginDragging(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportDidScroll(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportDidScroll(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportDidEndDragging(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event, bool willDecelerate) {
    ::switf_bridge_viewportDidEndDragging(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height(), willDecelerate);
}

void SwiftSeatCanvasCoreRendererDelegate::viewportDidEndDecelerating(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportDidEndDecelerating(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportWillBeginZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportWillBeginZooming(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportDidZoom(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportDidZoom(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportDidEndZooming(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportDidEndZooming(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

void SwiftSeatCanvasCoreRendererDelegate::viewportDidEndScrollingAnimation(uint32_t coreID, const kk::renderer::SeatCanvasViewportEvent &event) {
    ::switf_bridge_viewportDidEndScrollingAnimation(coreID, event.zoomScale, event.contentOffset.x, event.contentOffset.y, event.visibleOriginalRect.x(), event.visibleOriginalRect.y(), event.visibleOriginalRect.width(), event.visibleOriginalRect.height());
}

};  // namespace kk::bridge
