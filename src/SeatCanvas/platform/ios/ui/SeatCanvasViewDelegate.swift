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
public protocol SeatCanvasViewDelegate: AnyObject {
    /// 根据区域和座位ID返回样式ID。
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID（由业务保证全局唯一）
    /// - Returns: 样式ID。返回 nil 或空字符串表示该座位不渲染。
    func seatCanvasView(_ view: SeatCanvasView, styleIdForSeat zoneId: String, seatId: String) -> String?

    /// 点击某个座位，业务层处理状态变更。
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    /// - Returns: 是否发生了状态变化。true 则触发重绘。
    func seatCanvasView(_ view: SeatCanvasView, didTapSeat zoneId: String, seatId: String) -> Bool

    /// 点击某个区域
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    func seatCanvasView(_ view: SeatCanvasView, didTapZone zoneId: String)
}
