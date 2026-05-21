//
//  SeatCanvasBaseMapLoadedEvent.swift
//  SeatCanvas
//

import CoreGraphics
import Foundation

@objc(KKSeatCanvasBaseMapLoadedEvent)
@MainActor
public final class SeatCanvasBaseMapLoadedEvent: NSObject, Sendable {
    @objc public let baseMapSize: CGSize
    @objc public let zoomLevels: ZoomLevel
    @objc public let minimumZoomScale: CGFloat
    @objc public let maximumZoomScale: CGFloat
    @objc public let zoomScale: CGFloat
    @objc public let visibleOriginalRect: CGRect

    init(
        baseMapSize: CGSize,
        zoomLevels: ZoomLevel,
        minimumZoomScale: CGFloat,
        maximumZoomScale: CGFloat,
        zoomScale: CGFloat,
        visibleOriginalRect: CGRect
    ) {
        self.baseMapSize = baseMapSize
        self.zoomLevels = zoomLevels
        self.minimumZoomScale = minimumZoomScale
        self.maximumZoomScale = maximumZoomScale
        self.zoomScale = zoomScale
        self.visibleOriginalRect = visibleOriginalRect
    }
}
