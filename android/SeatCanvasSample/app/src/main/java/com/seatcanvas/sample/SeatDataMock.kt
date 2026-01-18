package com.seatcanvas.sample

import android.graphics.Color
import com.libseatcanvas.SeatData
import com.libseatcanvas.SeatZoneData

data class RegionRect(val x: Float, val y: Float, val width: Float, val height: Float)

data class RegionInfo(val zoneId: String, val rect: RegionRect)

fun getMockRegions(): List<RegionInfo> = listOf(
    RegionInfo("20011", RegionRect(850f, 2800f, 550f, 320f)),
    RegionInfo("20012", RegionRect(850f, 3170f, 550f, 320f)),
    RegionInfo("20013", RegionRect(850f, 3540f, 550f, 320f)),
    RegionInfo("20014", RegionRect(850f, 3910f, 550f, 320f)),
    RegionInfo("20031", RegionRect(2000f, 2800f, 450f, 280f)),
    RegionInfo("20032", RegionRect(2000f, 3130f, 450f, 280f)),
    RegionInfo("20034", RegionRect(2000f, 3790f, 450f, 280f)),
    RegionInfo("10085", RegionRect(1700f, 1520f, 700f, 230f)),
    RegionInfo("10086", RegionRect(2450f, 1520f, 700f, 230f)),
    RegionInfo("30031", RegionRect(9550f, 2800f, 450f, 280f)),
    RegionInfo("30032", RegionRect(9550f, 3130f, 450f, 280f)),
    RegionInfo("30033", RegionRect(9550f, 3460f, 450f, 280f)),
    RegionInfo("10096", RegionRect(9600f, 1800f, 700f, 220f)),
    RegionInfo("10098", RegionRect(11100f, 1800f, 700f, 220f)),
    RegionInfo("40109", RegionRect(6950f, 9300f, 700f, 250f)),
    RegionInfo("40110", RegionRect(7700f, 9300f, 700f, 250f)),
    RegionInfo("40123", RegionRect(6600f, 9600f, 750f, 300f)),
    RegionInfo("40124", RegionRect(7400f, 9600f, 750f, 300f)),
    RegionInfo("37492", RegionRect(33f, 411f, 289f, 329f)),
    RegionInfo("74148", RegionRect(400f, 76f, 332f, 232f)),
)

fun generateMockZoneDatas(): Array<SeatZoneData> {
    val regions = getMockRegions()
    return regions.map { region ->
        val color = Color.WHITE
        val priceColor = Color.rgb(
            (128..255).random(),
            (128..255).random(),
            (128..255).random(),
        )
        SeatZoneData(zoneId = region.zoneId, color = color, priceColor = priceColor)
    }.toTypedArray()
}

fun generateMockSeatDatas(rect: RegionRect): Array<SeatData> {
    val itemSize = 36.0f
    val spacing = 10.0f

    val cols = ((rect.width) / (itemSize + spacing)).toInt()
    val rows = ((rect.height) / (itemSize + spacing)).toInt()

    val seats = mutableListOf<SeatData>()

    for (row in 0 until rows) {
        for (col in 0 until cols) {
            val seatX = rect.x + col * (itemSize + spacing)
            val seatY = rect.y + row * (itemSize + spacing)
            val seatId = "seat_${row}_${col}"

            val randValue = (row * cols + col) % 10
            val status = when {
                randValue < 6 -> 0u
                randValue < 8 -> 1u
                randValue < 9 -> 2u
                else -> 3u
            }

            seats.add(
                SeatData(
                    seatId = seatId,
                    status = status,
                    selected = false,
                    x = seatX,
                    y = seatY
                )
            )
        }
    }
    return seats.toTypedArray()
}
