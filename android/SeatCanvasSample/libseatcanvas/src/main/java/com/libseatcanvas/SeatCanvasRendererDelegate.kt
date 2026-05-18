package com.libseatcanvas

/**
 * 座位渲染器代理。样式 ID 解析与点击处理由核心渲染器驱动。
 */
interface SeatCanvasRendererDelegate {
    /**
     * 根据区域与座位返回样式 ID；返回 null 表示不绘制该座位。
     */
    fun styleIdForSeat(zoneId: String, seatId: String): String?

    /**
     * 点击座位；若业务状态发生变化且需要刷新视图，返回 true。
     */
    fun didTapSeat(zoneId: String, seatId: String): Boolean

    /**
     * 点击区域（非座位区域）。
     */
    fun didTapZone(zoneId: String)

    /**
     * 即将开始拖动视图。
     * @param viewport 当前视图状态。
     */
    fun willBeginDragging(viewport: SeatCanvasViewport) {}

    /**
     * 视图发生滚动。
     * @param viewport 当前视图状态。
     */
    fun didScroll(viewport: SeatCanvasViewport) {}

    /**
     * 拖动手势结束。
     * @param viewport 当前视图状态。
     * @param decelerate 是否会继续惯性滚动或回弹动画。
     */
    fun didEndDragging(viewport: SeatCanvasViewport, decelerate: Boolean) {}

    /**
     * 惯性滚动或回弹动画结束。
     * @param viewport 当前视图状态。
     */
    fun didEndDecelerating(viewport: SeatCanvasViewport) {}

    /**
     * 即将开始缩放视图。
     * @param viewport 当前视图状态。
     */
    fun willBeginZooming(viewport: SeatCanvasViewport) {}

    /**
     * 视图发生缩放。
     * @param viewport 当前视图状态。
     */
    fun didZoom(viewport: SeatCanvasViewport) {}

    /**
     * 缩放手势结束。
     * @param viewport 当前视图状态。
     */
    fun didEndZooming(viewport: SeatCanvasViewport) {}

    /**
     * 程序触发的滚动或缩放动画结束。
     * @param viewport 当前视图状态。
     */
    fun didEndScrollingAnimation(viewport: SeatCanvasViewport) {}
}
