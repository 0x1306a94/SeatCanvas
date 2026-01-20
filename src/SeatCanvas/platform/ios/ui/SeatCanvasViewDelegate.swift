//
//  SeatCanvasViewDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import Foundation

/// 座位选中事件回调
@MainActor
@objc(KKSeatCanvasViewDelegate)
public protocol SeatCanvasViewDelegate {
    /// 是否可以选中座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_ view: SeatCanvasView, shouldSelectSeat zoneId: String, seatId: String) -> Bool

    /// 选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_ view: SeatCanvasView, didSelectSeat zoneId: String, seatId: String)

    /// 取消选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    func seatCanvasView(_ view: SeatCanvasView, didDeselectSeat zoneId: String, seatId: String)

    /// 点击某个区域
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    func seatCanvasView(_ view: SeatCanvasView, didTapZone zoneId: String)
}
