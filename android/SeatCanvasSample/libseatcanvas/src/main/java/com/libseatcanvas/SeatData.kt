package com.libseatcanvas

/**
 * 单个座位的几何与标识数据；可变状态通过 status / selected API 推送。
 * @param seatId 座位 ID
 * @param x 原始坐标系中的 X
 * @param y 原始坐标系中的 Y
 * @param rotation 旋转角（度，顺时针、相对竖直向上），默认 0
 * @param pricecode 价档 code，null 或空表示无价档（保留座等）
 */
class SeatData(
    val seatId: String,
    val x: Float,
    val y: Float,
    val rotation: Float = 0f,
    val pricecode: String? = null
)
