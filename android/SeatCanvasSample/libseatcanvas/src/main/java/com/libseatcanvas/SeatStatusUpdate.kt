package com.libseatcanvas

/**
 * 单个座位的 status 更新项。
 * @param seatId 座位 ID
 * @param status 业务自定义座位状态
 */
data class SeatStatusUpdate(
    val seatId: String,
    val status: Int
)
