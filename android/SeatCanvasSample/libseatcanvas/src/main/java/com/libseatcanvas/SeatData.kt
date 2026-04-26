package com.libseatcanvas

/**
 * 单个座位的几何与标识数据；业务状态由 [SeatCanvasRendererDelegate.styleIdForSeat] 返回的样式 ID 表示。
 * @param seatId 座位 ID
 * @param x 原始坐标系中的 X
 * @param y 原始坐标系中的 Y
 * @param rotation 旋转角（度，顺时针、相对竖直向上），默认 0
 */
class SeatData(
    val seatId: String,
    val x: Float,
    val y: Float,
    val rotation: Float = 0f
)
