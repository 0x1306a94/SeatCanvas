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
}
