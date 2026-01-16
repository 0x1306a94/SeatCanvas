package com.libseatcanvas.style

import androidx.annotation.ColorInt

data class CircleSeatStyleConfig(
    @ColorInt val fill: Int,
    @ColorInt val overlay: Int,
    @ColorInt val checkmark: Int
) : SeatStyleConfig {
    override val type: SeatStyleType = SeatStyleType.CIRCLE
}
