package com.seatcanvas.sample

import com.google.gson.annotations.SerializedName

data class MockPriceData(
    @SerializedName("code")
    val code: String,
    @SerializedName("name")
    val name: String,
    @SerializedName("color")
    val color: String,
    @SerializedName("zoneIds")
    val zoneIds: List<String>
)
