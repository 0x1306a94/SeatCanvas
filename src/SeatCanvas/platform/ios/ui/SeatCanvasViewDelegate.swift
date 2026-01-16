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
    ///   - seatId: 座位ID
    func seatCanvasView(_ view: SeatCanvasView, shouldSelectSeat seatId: String) -> Bool

    /// 选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - seatId: 座位ID
    func seatCanvasView(_ view: SeatCanvasView, didSelectSeat seatId: String)

    /// 取消选中某个座位
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - seatId: 座位ID
    func seatCanvasView(_ view: SeatCanvasView, didDeselectSeat seatId: String)
}
