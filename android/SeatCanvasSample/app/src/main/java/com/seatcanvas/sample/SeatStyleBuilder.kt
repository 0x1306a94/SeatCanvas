package com.seatcanvas.sample

import android.content.Context
import android.graphics.Color
import androidx.core.content.ContextCompat
import com.libseatcanvas.SeatRenderStyleId
import com.libseatcanvas.style.SeatStyleConfigBuilder

object SeatStatus {
    const val UNAVAILABLE: Int = 0
    const val AVAILABLE: Int = 1
}

object SeatStyleBuilder {
    fun buildCircleSeatStyleConfig(context: Context, prices: List<MockPriceData>): ByteArray? {
        if (prices.isEmpty()) {
            return null
        }
        val builder = SeatStyleConfigBuilder()
        val disabled = ContextCompat.getColor(context, R.color.seat_disabled)
        val overlay = Color.argb((0.7 * 255).toInt(), 0, 0, 0)
        val checkmark = Color.WHITE
        for (price in prices) {
            val fill = ColorExtensions.colorFromArgbHex(price.color) ?: continue
            builder.addCircleStyle(SeatRenderStyleId.compose(price.code, SeatStatus.UNAVAILABLE, selected = false), disabled)
            builder.addCircleStyle(SeatRenderStyleId.compose(price.code, SeatStatus.AVAILABLE, selected = false), fill)
            builder.addCircleStyle(
                styleId = SeatRenderStyleId.compose(price.code, SeatStatus.AVAILABLE, selected = true),
                fill = fill,
                overlay = overlay,
                checkmark = checkmark
            )
        }
        return builder.toJSONData()
    }

    fun buildSVGSeatStyleConfig(context: Context, prices: List<MockPriceData>): ByteArray? {
        if (prices.isEmpty()) {
            return null
        }
        val available = loadSVGContent(context, "icon_seat_selectable.svg") ?: return null
        val selected = loadSVGContent(context, "icon_seat_selected.svg") ?: return null
        val disabled = loadSVGContent(context, "icon_seat_nonselectable.svg") ?: return null

        val builder = SeatStyleConfigBuilder()
        for (price in prices) {
            val rgbHex = ColorExtensions.rgbHexFromArgb(price.color) ?: continue
            builder.addSVGStyle(SeatRenderStyleId.compose(price.code, SeatStatus.UNAVAILABLE, selected = false), disabled)
            val modifiedAvailable = available.replace(Regex("#EB484A", RegexOption.IGNORE_CASE), rgbHex)
            val modifiedSelected = selected.replace(Regex("#5BC64D", RegexOption.IGNORE_CASE), rgbHex)
            builder.addSVGStyle(SeatRenderStyleId.compose(price.code, SeatStatus.AVAILABLE, selected = false), modifiedAvailable)
            builder.addSVGStyle(SeatRenderStyleId.compose(price.code, SeatStatus.AVAILABLE, selected = true), modifiedSelected)
        }
        return builder.toJSONData()
    }

    private fun loadSVGContent(context: Context, name: String): String? {
        return try {
            val fileName = context.defaultSeatStylePath(name)
            context.assets.open(fileName).use { inputStream ->
                inputStream.bufferedReader().use { reader -> reader.readText() }
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }
}
