package com.seatcanvas.sample

import com.google.gson.annotations.SerializedName

data class MockSeatData(
    @SerializedName("seatId")
    val seatId: String,
    @SerializedName("status")
    val status: UInt,
    @SerializedName("x")
    val x: Float,
    @SerializedName("y")
    val y: Float,
    var selected: Boolean = false
)
