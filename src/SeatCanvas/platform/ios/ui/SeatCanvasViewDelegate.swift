//
//  SeatCanvasViewDelegate.swift
//  SeatCanvas
//
//  Created by king on 2025/12/12.
//

import CoreGraphics
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

    /// 即将开始拖动视图
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewWillBeginDragging(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)

    /// 视图发生滚动
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewDidScroll(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)

    /// 拖动手势结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    ///   - decelerate: 是否会继续惯性滚动或回弹动画
    @objc optional func seatCanvasViewDidEndDragging(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect, decelerate: Bool)

    /// 惯性滚动或回弹动画结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewDidEndDecelerating(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)

    /// 即将开始缩放视图
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewWillBeginZooming(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)

    /// 视图发生缩放
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewDidZoom(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)

    /// 缩放手势结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewDidEndZooming(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)

    /// 程序触发的滚动或缩放动画结束
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomScale: 当前缩放比例
    ///   - contentOffset: 当前内容偏移，单位为 viewport 像素
    ///   - visibleOriginalRect: 当前可见区域，使用底图原始坐标系
    @objc optional func seatCanvasViewDidEndScrollingAnimation(_ view: SeatCanvasView, zoomScale: CGFloat, contentOffset: CGPoint, visibleOriginalRect: CGRect)
}
