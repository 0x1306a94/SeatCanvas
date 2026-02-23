package com.libseatcanvas

class SeatData(
    val seatId: String,
    val status: UInt,
    val selected: Boolean,
    val x: Float,
    val y: Float,
    val rotation: Float = 0f
) {
}
