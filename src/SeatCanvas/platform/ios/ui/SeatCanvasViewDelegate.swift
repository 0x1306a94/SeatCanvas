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
    /// 底图加载完成；push 座位数据和样式配置
    @objc optional func seatCanvasViewDidLoadBaseMap(_ view: SeatCanvasView)

    /// 底图已卸载
    @objc optional func seatCanvasViewDidUnloadBaseMap(_ view: SeatCanvasView)

    /// 缩放级别配置已更新（底图加载完成或设备旋转后触发）；在此设置 seatRenderZoomThreshold 以同步旋转后的阈值
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoomLevels: 缩放等级
    ///   - minimumZoomScale: 最小缩放比例
    ///   - maximumZoomScale: 最大缩放比例
    ///   - zoomScale: 当前缩放比
    @objc optional func seatCanvasView(_ view: SeatCanvasView, didUpdateZoomLevelConfig zoomLevels: ZoomLevel, minimumZoomScale: CGFloat, maximumZoomScale: CGFloat, zoomScale: CGFloat)

    /// 点击某个座位，业务层处理状态变更
    /// - Parameters:
    ///   - view: SeatCanvasView 实例
    ///   - zoneId: 区域ID
    ///   - seatId: 座位ID
    /// - Returns: 是否发生了状态变化 true 则触发重绘
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
