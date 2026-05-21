package com.libseatcanvas

/**
 * 座位渲染器代理。点击与视口事件由核心渲染器驱动；座位样式通过推送 API 更新。
 */
interface SeatCanvasRendererDelegate {
     /**
     * 底图加载完成；此时 ZoomLevelConfig 与初始缩放已就绪，可设置 seatRenderZoomThreshold 并 push 座位数据。
     */
    fun didLoadBaseMap(event: SeatCanvasBaseMapLoadedEvent) {}

    /**
     * 底图已卸载（loadBaseMap(null) 或切换底图前）。
     */
    fun didUnloadBaseMap() {}
    
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
