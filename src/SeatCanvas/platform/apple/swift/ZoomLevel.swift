//
//  ZoomLevel.swift
//  SeatCanvas
//
//  Created by king on 2026/2/10.
//

import Foundation

@objc(KKSeatCanvasZoomLevel)
@objcMembers
public final class ZoomLevel: NSObject, Sendable {
    /// 最近视角（座位级别）
    public let seat: CGFloat
    /// 行/小块级视角
    public let row: CGFloat
    /// 区域级视角
    @objc(region)
    public let zone: CGFloat
    /// 全场/概览级视角（默认作为彩虹图与座位渲染切换的阈值）
    public let venue: CGFloat

    package init(seat: CGFloat, row: CGFloat, zone: CGFloat, venue: CGFloat) {
        self.seat = seat
        self.row = row
        self.zone = zone
        self.venue = venue
    }

    package static let defalut = ZoomLevel(seat: 1.0, row: 1.0, zone: 1.0, venue: 1.0)
}
