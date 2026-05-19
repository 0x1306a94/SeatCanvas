package com.seatcanvas.sample

import com.google.gson.annotations.SerializedName

data class MockSeatData(
    @SerializedName("seatId")
    val seatId: String,
    @SerializedName("pricecode")
    val pricecode: String,
    @SerializedName("x")
    val x: Float,
    @SerializedName("y")
    val y: Float,
    @SerializedName("rotation")
    val rotation: Float = 0f,
    var selected: Boolean = false
)
