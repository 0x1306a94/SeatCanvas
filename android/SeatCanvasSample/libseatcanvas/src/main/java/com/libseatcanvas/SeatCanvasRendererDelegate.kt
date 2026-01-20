package com.libseatcanvas

/**
 * 座位渲染器代理接口
 * 用于处理座位选择相关的回调
 */
interface SeatCanvasRendererDelegate {
    /**
     * 点击某个区域
     * @param zoneId 区域ID
     */
    fun didTapZone(zoneId: String)

    /**
     * 是否可以选中座位
     * @param zoneId 区域ID
     * @param seatId 座位ID
     * @return true 表示可以选中，false 表示不能选中
     */
    fun shouldSelectSeat(zoneId: String, seatId: String): Boolean

    /**
     * 选中某个座位
     * @param zoneId 区域ID
     * @param seatId 座位ID
     */
    fun didSelectSeat(zoneId: String, seatId: String)

    /**
     * 取消选中某个座位
     * @param zoneId 区域ID
     * @param seatId 座位ID
     */
    fun didDeselectSeat(zoneId: String, seatId: String)
}
