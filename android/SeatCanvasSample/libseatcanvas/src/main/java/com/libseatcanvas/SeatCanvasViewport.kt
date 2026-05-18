package com.libseatcanvas

/**
 * 当前视图状态。
 * @property zoomScale 当前缩放比例。
 * @property contentOffsetX 当前内容 X 偏移，单位为 viewport 像素。
 * @property contentOffsetY 当前内容 Y 偏移，单位为 viewport 像素。
 * @property visibleOriginalRect 当前可见区域，使用底图原始坐标系。
 */
class SeatCanvasViewport(
    val zoomScale: Float,
    val contentOffsetX: Float,
    val contentOffsetY: Float,
    val visibleOriginalRect: Rect
) {
    override fun toString(): String {
        return "SeatCanvasViewport(zoomScale=$zoomScale, contentOffsetX=$contentOffsetX, contentOffsetY=$contentOffsetY, visibleOriginalRect=$visibleOriginalRect)"
    }
}
