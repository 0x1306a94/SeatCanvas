package com.libseatcanvas.style

import androidx.annotation.ColorInt

/**
 * 圆形座位样式。选中态可使用 [overlay]、[checkmark]；未选中一般只需 [fill]。
 * @param fill 填充色（ARGB）
 * @param overlay 遮罩色，选中时覆盖在座位上；未使用则为 null
 * @param checkmark 对勾颜色；未使用则为 null
 */
data class CircleSeatStyleConfig(
    @ColorInt val fill: Int,
    @ColorInt val overlay: Int? = null,
    @ColorInt val checkmark: Int? = null
) : SeatStyleConfig {
    override val type: SeatStyleType = SeatStyleType.CIRCLE
}
