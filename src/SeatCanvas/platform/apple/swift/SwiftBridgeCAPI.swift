//
//  SwiftBridgeCAPI.swift
//  SeatCanvas
//
//  Created by king on 2025/12/11.
//

import CoreGraphics
import Foundation

// MARK: - C++ 到 Swift 的桥接函数

private func bridgeString(_ value: UnsafePointer<CChar>?) -> String {
    guard let value else {
        return ""
    }
    return String(cString: value)
}

private func writeCString(_ string: String, to buffer: UnsafeMutablePointer<CChar>?, maxLength: Int) -> Bool {
    guard let buffer, maxLength > 0 else {
        return false
    }
    let utf8 = Array(string.utf8)
    if utf8.count + 1 > maxLength {
        return false
    }
    for (index, byte) in utf8.enumerated() {
        buffer[index] = CChar(bitPattern: byte)
    }
    buffer[utf8.count] = 0
    return true
}

private func makeViewportValues(
    zoomScale: Float,
    contentOffsetX: Float,
    contentOffsetY: Float,
    visibleOriginalRectX: Float,
    visibleOriginalRectY: Float,
    visibleOriginalRectWidth: Float,
    visibleOriginalRectHeight: Float
) -> (CGFloat, CGPoint, CGRect) {
    let contentOffset = CGPoint(x: CGFloat(contentOffsetX), y: CGFloat(contentOffsetY))
    let visibleOriginalRect = CGRect(
        x: CGFloat(visibleOriginalRectX),
        y: CGFloat(visibleOriginalRectY),
        width: CGFloat(visibleOriginalRectWidth),
        height: CGFloat(visibleOriginalRectHeight)
    )
    return (CGFloat(zoomScale), contentOffset, visibleOriginalRect)
}

/// 点击区域回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
@c(switf_bridge_didTapZone)
func switf_bridge_didTapZone(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?) {
    let zoneIdString = bridgeString(zoneId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }

        delegate.seatCanvasRendererDidTapZone(zoneId: zoneIdString)
    }
}

/// 点击座位回调，由 C++ 层调用(仅内部调用)
/// - Parameters:
///   - coreID: 座位渲染器实例ID
///   - zoneId: 区域ID
///   - seatId: 座位ID
/// - Returns: 业务层状态是否发生变化
@c(switf_bridge_didTapSeat)
func switf_bridge_didTapSeat(_ coreID: UInt32, _ zoneId: UnsafePointer<CChar>?, _ seatId: UnsafePointer<CChar>?) -> Bool {
    let zoneIdString = bridgeString(zoneId)
    let seatIdString = bridgeString(seatId)
    return MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return false
        }

        return delegate.seatCanvasRendererDidTapSeat(zoneId: zoneIdString, seatId: seatIdString)
    }
}

@c(switf_bridge_viewportWillBeginDragging)
func switf_bridge_viewportWillBeginDragging(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }

        delegate.seatCanvasRendererWillBeginDragging(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_viewportDidScroll)
func switf_bridge_viewportDidScroll(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidScroll(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_viewportDidEndDragging)
func switf_bridge_viewportDidEndDragging(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float, _ willDecelerate: Bool) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidEndDragging(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2, decelerate: willDecelerate)
    }
}

@c(switf_bridge_viewportDidEndDecelerating)
func switf_bridge_viewportDidEndDecelerating(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidEndDecelerating(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_viewportWillBeginZooming)
func switf_bridge_viewportWillBeginZooming(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererWillBeginZooming(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_viewportDidZoom)
func switf_bridge_viewportDidZoom(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidZoom(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_viewportDidEndZooming)
func switf_bridge_viewportDidEndZooming(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidEndZooming(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_viewportDidEndScrollingAnimation)
func switf_bridge_viewportDidEndScrollingAnimation(_ coreID: UInt32, _ zoomScale: Float, _ contentOffsetX: Float, _ contentOffsetY: Float, _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float) {
    let viewport = makeViewportValues(zoomScale: zoomScale, contentOffsetX: contentOffsetX, contentOffsetY: contentOffsetY, visibleOriginalRectX: visibleOriginalRectX, visibleOriginalRectY: visibleOriginalRectY, visibleOriginalRectWidth: visibleOriginalRectWidth, visibleOriginalRectHeight: visibleOriginalRectHeight)
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidEndScrollingAnimation(zoomScale: viewport.0, contentOffset: viewport.1, visibleOriginalRect: viewport.2)
    }
}

@c(switf_bridge_didLoadBaseMap)
func switf_bridge_didLoadBaseMap(_ coreID: UInt32, _ baseMapWidth: Float, _ baseMapHeight: Float,
                                 _ seatZoom: Float, _ rowZoom: Float, _ zoneZoom: Float, _ venueZoom: Float,
                                 _ minimumZoomScale: Float, _ maximumZoomScale: Float, _ zoomScale: Float,
                                 _ visibleOriginalRectX: Float, _ visibleOriginalRectY: Float, _ visibleOriginalRectWidth: Float, _ visibleOriginalRectHeight: Float)
{
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }

        let event = SeatCanvasBaseMapLoadedEvent(
            baseMapSize: CGSize(width: CGFloat(baseMapWidth), height: CGFloat(baseMapHeight)),
            zoomLevels: ZoomLevel(seat: CGFloat(seatZoom), row: CGFloat(rowZoom), zone: CGFloat(zoneZoom), venue: CGFloat(venueZoom)),
            minimumZoomScale: CGFloat(minimumZoomScale),
            maximumZoomScale: CGFloat(maximumZoomScale),
            zoomScale: CGFloat(zoomScale),
            visibleOriginalRect: CGRect(x: CGFloat(visibleOriginalRectX), y: CGFloat(visibleOriginalRectY), width: CGFloat(visibleOriginalRectWidth), height: CGFloat(visibleOriginalRectHeight))
        )

        delegate.seatCanvasRendererDidLoadBaseMap(event: event)
    }
}

@c(switf_bridge_didUnloadBaseMap)
func switf_bridge_didUnloadBaseMap(_ coreID: UInt32) {
    MainActor.assumeIsolated {
        guard let delegate = SeatCanvasRendererDelegateRegistry.shared.delegate(for: coreID) else {
            #if DEBUG
                print("[SwiftBridge] 警告: 未找到 coreID \(coreID) 对应的 SeatCanvasRendererDelegate")
            #endif
            return
        }
        delegate.seatCanvasRendererDidUnloadBaseMap()
    }
}
