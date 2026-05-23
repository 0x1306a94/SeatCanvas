//
//  SeatCanvasRendererDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import CoreGraphics
import Foundation

@MainActor
protocol SeatCanvasRendererDelegate: AnyObject {
    func seatCanvasRendererDidLoadBaseMap()
    func seatCanvasRendererDidUnloadBaseMap()
    func seatCanvasRendererDidUpdateZoomLevelConfig(zoomLevels: ZoomLevel, minimumZoomScale: CGFloat, maximumZoomScale: CGFloat, zoomScale: CGFloat)

    func seatCanvasRendererDidTapZone(zoneId: String)
    func seatCanvasRendererDidTapSeat(zoneId: String, seatId: String) -> Bool

    func seatCanvasRendererWillBeginDragging(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
    func seatCanvasRendererDidScroll(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
    func seatCanvasRendererDidEndDragging(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect, decelerate: Bool)
    func seatCanvasRendererDidEndDecelerating(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
    func seatCanvasRendererWillBeginZooming(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
    func seatCanvasRendererDidZoom(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
    func seatCanvasRendererDidEndZooming(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
    func seatCanvasRendererDidEndScrollingAnimation(zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
}
