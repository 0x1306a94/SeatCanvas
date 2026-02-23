package com.seatcanvas.sample

import android.graphics.Color
import com.google.gson.annotations.SerializedName

data class MockZoneInfo(
    @SerializedName("zoneId")
    val zoneId: String,
    @SerializedName("x")
    val x: Float,
    @SerializedName("y")
    val y: Float,
    @SerializedName("w")
    val w: Float,
    @SerializedName("h")
    val h: Float,
    @SerializedName("color")
    val colorHex: String? = null,
    @SerializedName("alternateColor")
    val alternateColorHex: String? = null
) {

    val alternateColor: Int?
        get() = alternateColorHex?.let { ColorExtensions.colorFromArgbHex(it) }
}
