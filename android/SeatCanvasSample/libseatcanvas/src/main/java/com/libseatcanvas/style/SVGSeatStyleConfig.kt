package com.libseatcanvas.style

data class SVGSeatStyleConfig(
    val content: String
) : SeatStyleConfig {
    override val type: SeatStyleType = SeatStyleType.SVG
}
