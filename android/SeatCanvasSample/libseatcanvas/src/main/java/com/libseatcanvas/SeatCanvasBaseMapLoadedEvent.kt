package com.libseatcanvas

/**
 * 底图加载完成事件，对应 C++ 层的 SeatCanvasBaseMapLoadedEvent。
 */
data class SeatCanvasBaseMapLoadedEvent(
    val baseMapWidth: Float,
    val baseMapHeight: Float,
    val zoomLevels: ZoomLevel,
    val minimumZoomScale: Float,
    val maximumZoomScale: Float,
    val zoomScale: Float,
    val visibleOriginalRect: Rect,
)
