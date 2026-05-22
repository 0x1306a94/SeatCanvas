package com.libseatcanvas

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.Surface
import android.view.TextureView
import android.view.View
import androidx.annotation.ColorInt

class SeatCanvasView : TextureView, TextureView.SurfaceTextureListener {
    private var surface: Surface? = null
    private var nativePtr: Long = 0
    private var isAttachedToWindow: Boolean = false
    private var isVisible: Boolean = false
    private val gestureHandler by lazy {
        SeatCanvasGestureHandler(
            onPan = ::nativeHandlePan,
            onPinch = ::nativeHandlePinch,
            onTap = ::handleTap
        )
    }

    constructor(context: Context) : super(context) {
        setup()
    }

    constructor(context: Context, attrs: AttributeSet) : super(context, attrs) {
        setup()
    }


    constructor(context: Context, attrs: AttributeSet, defStyleAttr: Int) : super(
        context,
        attrs,
        defStyleAttr
    ) {
        setup()
    }

    private fun setup() {
        val metrics = resources.displayMetrics
        nativeInitSystemProperties(metrics.density, metrics.scaledDensity)

        setupNativePtr()
        setupSurfaceTexture()
        // 确保 View 可以接收触摸事件
        isClickable = true
        isFocusable = true
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        if (!nativeInitialized()) {
            return super.onTouchEvent(event)
        }

        if (gestureHandler.handle(event)) {
            return true
        }

        return super.onTouchEvent(event)
    }

    private var delegate: SeatCanvasRendererDelegate? = null

    /**
     * 设置座位渲染器代理（样式 ID、座位点击、区域点击等回调）。
     * @param delegate 代理，可为 null 表示不接收回调
     */
    fun setDelegate(delegate: SeatCanvasRendererDelegate?) {
        this.delegate = delegate
    }


    private fun setupNativePtr() {
        if (nativeInitialized()) {
            return
        }
        nativePtr = nativeCreate()
    }

    private fun setupSurfaceTexture() {
        surfaceTextureListener = this
    }

    override fun onSurfaceTextureAvailable(p0: SurfaceTexture, p1: Int, p2: Int) {
        surface = Surface(p0)
        nativeUpdateSurface(surface!!)

    }

    override fun onSurfaceTextureSizeChanged(p0: SurfaceTexture, p1: Int, p2: Int) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSize()
    }

    override fun onSurfaceTextureUpdated(p0: SurfaceTexture) {
    }

    override fun onSurfaceTextureDestroyed(p0: SurfaceTexture): Boolean {
        nativeUpdateSurface(null)
        post {
            surface?.release()
            surface = null
        }
        return true
    }

    private fun nativeInitialized(): Boolean {
        return nativePtr != 0L
    }

    private fun release() {
        delegate = null
        surface?.release()
        surface = null
        if (nativeInitialized()) {
            nativeRelease()
        }
    }

    override fun onAttachedToWindow() {
        isAttachedToWindow = true;
        super.onAttachedToWindow()

        checkVisible()
    }

    override fun onDetachedFromWindow() {
        isAttachedToWindow = false
        super.onDetachedFromWindow()

        surface?.release()
        surface = null

        checkVisible()
    }

    override fun setVisibility(visibility: Int) {
        super.setVisibility(visibility)
        checkVisible()

    }

    override fun onVisibilityAggregated(isVisible: Boolean) {
        super.onVisibilityAggregated(isVisible)
        checkVisible()
    }

    private fun checkVisible() {
        val visible = isAttachedToWindow && isShown
        if (isVisible == visible) {
            return
        }

        isVisible = visible
        if (isVisible) {
            nativeStartDrawLoop()
        } else {
            nativeStopDrawLoop()
        }
    }


    fun onResume() {
        // When the device is locked and then unlocked, the SeatCanvasView's content may disappear,
        // use the following way to make the content appear.
        if (isVisible) {
            visibility = View.INVISIBLE
            visibility = View.VISIBLE
        }
    }


    fun onDestroy() {
        release()
    }

    fun loadBaseMap(data: ByteArray?, format: BaseMapFormat, parseConfig: BaseMapParseConfig? = null) {
        val result = nativeLoadBaseMapFromFormat(data, format.name, parseConfig?.serializeToByteArray())
        nativeLoadBaseMap(result)
    }

    /**
     * 应用样式配置（使用 SeatStyleConfigBuilder 构建）
     * @param data JSON 样式数据
     */
    fun applySeatStyleJSONConfig(data: ByteArray?) {
        if (!nativeInitialized()) {
            return
        }
        if (data == null || data.isEmpty()) {
            nativeSetSeatStyleJSONConfig(null, 0)
            return
        }
        nativeSetSeatStyleJSONConfig(data, data.size)
    }

    /**
     * 画布背景色（ColorInt，AARRGGBB）。如 Color.WHITE、Color.parseColor("#RRGGBB") 等。
     */
    var canvasColor: Int
        @ColorInt get() {
            if (nativeInitialized()) {
                return nativeGetCanvasColor()
            } else {
                return android.graphics.Color.WHITE
            }
        }
        set(@ColorInt value) {
            if (nativeInitialized()) {
                nativeSetCanvasColor(value)
            }
        }


    fun updateSeatZoneAlternateColors(zones: Array<SeatZoneColor>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSeatZoneAlternateColors(zones)
    }

    fun updateMiniMapZoneAlternateColors(zones: Array<SeatZoneColor>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateMiniMapZoneAlternateColors(zones)
    }

    /**
     * 更新指定 zone 的座位几何数据。
     * 会重置该 zone 的 status 为 0，并清除旧 seat 的 selected；随后需 re-push status/selected。
     */
    fun updateSeats(zoneId: String, seats: Array<SeatData>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSeats(zoneId, seats)
    }

    /** 注册价档表（load 前调用一次） */
    fun registerPricecodes(pricecodes: Array<String>) {
        if (!nativeInitialized()) {
            return
        }
        nativeRegisterPricecodes(pricecodes)
    }

    /** 批量更新单个座位 status */
    fun updateSeatStatuses(updates: Array<SeatStatusUpdate>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSeatStatuses(updates)
    }

    /** 批量更新某个 zone 内全部座位 status（数组下标与 updateSeats 顺序一致） */
    fun updateSeatStatusesForZone(zoneId: String, statuses: IntArray) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSeatStatusesForZone(zoneId, statuses)
    }

    /** 全量替换选中座位 */
    fun setSelectedSeatIds(seatIds: Array<String>) {
        if (!nativeInitialized()) {
            return
        }
        nativeSetSelectedSeatIds(seatIds)
    }

    /** 增量更新选中座位 */
    fun updateSelectedSeatIds(added: Array<String>, removed: Array<String>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSelectedSeatIds(added, removed)
    }

    /**
     * 清除全部座位数据
     */
    fun clearSeatData() {
        if (!nativeInitialized()) {
            return
        }
        nativeClearSeatData()
    }

    /**
     * 座位大小
     */
    var seatSize: Float
        get() {
            if (nativeInitialized()) {
                return nativeGetSeatSize()
            } else {
                return 36.0f
            }
        }
        set(value) {
            if (nativeInitialized()) {
                nativeSetSeatSize(value)
            }
        }

    /**
     * 座位渲染阈值（控制从彩虹图切换到绘制座位的缩放级别）
     * 默认等同于内部计算得到的 ZoomLevelConfig.venue
     */
    var seatRenderZoomThreshold: Float
        get() {
            if (nativeInitialized()) {
                return nativeGetSeatRenderZoomThreshold()
            } else {
                return 0f
            }
        }
        set(value) {
            if (nativeInitialized()) {
                nativeSetSeatRenderZoomThreshold(value)
            }
        }

    /**
     * 是否绘制调试 HUD（FPS、缩放级别、座位统计等），默认关闭
     */
    var debugHUDEnabled: Boolean
        get() {
            if (nativeInitialized()) {
                return nativeIsDebugHUDEnabled()
            }
            return false
        }
        set(value) {
            if (nativeInitialized()) {
                nativeSetDebugHUDEnabled(value)
            }
        }

    /**
     * 获取当前缩放级别配置（seat/row/zone/venue）
     */
    fun zoomLevel(): ZoomLevel {
        if (!nativeInitialized()) {
            return ZoomLevel(1f, 1f, 1f, 1f)
        }
        return nativeGetZoomLevel()
    }

    /**
     * 当前内容允许的最小缩放比例
     */
    val minimumZoomScale: Float
        get() {
            if (nativeInitialized()) {
                return nativeGetMinimumZoomScale()
            }
            return 1f
        }

    /**
     * 当前缩放级别
     */
    val zoomScale: Float
        get() {
            if (nativeInitialized()) {
                return nativeGetZoomScale()
            }
            return 1f
        }

    /**
     * 当前内容允许的最大缩放比例
     */
    val maximumZoomScale: Float
        get() {
            if (nativeInitialized()) {
                return nativeGetMaximumZoomScale()
            }
            return 1f
        }

    /**
     * 获取当前显示范围（原始坐标系）
     */
    fun visibleOriginalRect(): Rect {
        if (!nativeInitialized()) {
            return Rect(0f, 0f, 0f, 0f)
        }
        return nativeGetVisibleOriginalRect()
    }

    /**
     * 查找与指定矩形相交的区域 ID 列表（原始坐标系）
     * @param rect 查询矩形，通常配合 visibleOriginalRect() 使用
     * @return 相交区域的 zoneId 列表
     */
    fun zoneIdsInOriginalRect(rect: Rect): List<String> {
        if (!nativeInitialized()) {
            return emptyList()
        }
        return nativeGetZoneIdsInOriginalRect(rect)?.toList() ?: emptyList()
    }

    private fun zoomToRect(
        bounds: Rect,
        animated: Boolean = true,
        padding: Float = 0f,
        durationMs: Double = 300.0
    ) {
        nativeZoomToRect(bounds, animated, padding, durationMs)
    }


    /**
     * 处理点击事件
     */
    private fun handleTap(x: Float, y: Float) {
        if (!nativeInitialized()) {
            return
        }

        nativeHandleTap(x, y)
    }

    private fun nativeOnDidLoadBaseMap(
        baseMapWidth: Float,
        baseMapHeight: Float,
        seatZoom: Float,
        rowZoom: Float,
        zoneZoom: Float,
        venueZoom: Float,
        minimumZoomScale: Float,
        maximumZoomScale: Float,
        zoomScale: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.didLoadBaseMap(
            SeatCanvasBaseMapLoadedEvent(
                baseMapWidth = baseMapWidth,
                baseMapHeight = baseMapHeight,
                zoomLevels = ZoomLevel(seatZoom, rowZoom, zoneZoom, venueZoom),
                minimumZoomScale = minimumZoomScale,
                maximumZoomScale = maximumZoomScale,
                zoomScale = zoomScale,
                visibleOriginalRect = Rect(
                    visibleOriginalRectX,
                    visibleOriginalRectY,
                    visibleOriginalRectWidth,
                    visibleOriginalRectHeight
                )
            )
        )
    }

    private fun nativeOnDidUnloadBaseMap() {
        delegate?.didUnloadBaseMap()
    }

    /**
     * 由 C++ 层调用，转发给 delegate
     * @param zoneId 区域ID
     */
    private fun nativeOnDidTapZone(zoneId: String) {
        delegate?.didTapZone(zoneId)
    }

    /**
     * 由 native 层调用：处理座位点击；返回 true 表示状态已变且需要重绘。
     */
    private fun nativeOnDidTapSeat(zoneId: String, seatId: String): Boolean {
        return delegate?.didTapSeat(zoneId, seatId) ?: false
    }

    private fun makeViewport(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ): SeatCanvasViewport {
        return SeatCanvasViewport(
            zoomScale,
            contentOffsetX,
            contentOffsetY,
            Rect(
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportWillBeginDragging(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.willBeginDragging(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportDidScroll(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.didScroll(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportDidEndDragging(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float,
        decelerate: Boolean
    ) {
        delegate?.didEndDragging(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            ),
            decelerate
        )
    }

    private fun nativeOnViewportDidEndDecelerating(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.didEndDecelerating(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportWillBeginZooming(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.willBeginZooming(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportDidZoom(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.didZoom(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportDidEndZooming(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.didEndZooming(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }

    private fun nativeOnViewportDidEndScrollingAnimation(
        zoomScale: Float,
        contentOffsetX: Float,
        contentOffsetY: Float,
        visibleOriginalRectX: Float,
        visibleOriginalRectY: Float,
        visibleOriginalRectWidth: Float,
        visibleOriginalRectHeight: Float
    ) {
        delegate?.didEndScrollingAnimation(
            makeViewport(
                zoomScale,
                contentOffsetX,
                contentOffsetY,
                visibleOriginalRectX,
                visibleOriginalRectY,
                visibleOriginalRectWidth,
                visibleOriginalRectHeight
            )
        )
    }



    private external fun nativeLoadBaseMapFromFormat(data: ByteArray?, formatName: String, parseConfigJSON: ByteArray?): Long
    private external fun nativeLoadBaseMap(ptr: Long): Boolean
    private external fun nativeSetCanvasColor(color: Int)
    private external fun nativeGetCanvasColor(): Int
    private external fun nativeSetSeatStyleJSONConfig(data: ByteArray?, len: Int)
    private external fun nativeUpdateSeatZoneAlternateColors(zones: Array<SeatZoneColor>)
    private external fun nativeUpdateMiniMapZoneAlternateColors(zones: Array<SeatZoneColor>)
    private external fun nativeUpdateSeats(zoneId: String, seats: Array<SeatData>)
    private external fun nativeRegisterPricecodes(pricecodes: Array<String>)
    private external fun nativeUpdateSeatStatuses(updates: Array<SeatStatusUpdate>)
    private external fun nativeUpdateSeatStatusesForZone(zoneId: String, statuses: IntArray)
    private external fun nativeSetSelectedSeatIds(seatIds: Array<String>)
    private external fun nativeUpdateSelectedSeatIds(added: Array<String>, removed: Array<String>)
    private external fun nativeClearSeatData()
    private external fun nativeGetSeatSize(): Float
    private external fun nativeSetSeatSize(seatSize: Float)
    private external fun nativeGetSeatRenderZoomThreshold(): Float
    private external fun nativeSetSeatRenderZoomThreshold(threshold: Float)
    private external fun nativeIsDebugHUDEnabled(): Boolean
    private external fun nativeSetDebugHUDEnabled(enabled: Boolean)
    private external fun nativeGetZoomLevel(): ZoomLevel
    private external fun nativeHandleTap(x: Float, y: Float)
    private external fun nativeHandlePan(state: Int, tx: Float, ty: Float, timestampMs: Double)
    private external fun nativeHandlePinch(state: Int, scale: Float, cx: Float, cy: Float)

    private external fun nativeZoomToRect(
        bounds: Rect,
        animated: Boolean,
        padding: Float,
        durationMs: Double
    )

    private external fun nativeGetZoomScale(): Float
    private external fun nativeGetMinimumZoomScale(): Float
    private external fun nativeGetMaximumZoomScale(): Float
    private external fun nativeGetVisibleOriginalRect(): Rect
    private external fun nativeGetZoneIdsInOriginalRect(rect: Rect): Array<String>?
    private external fun nativeGetContentOffset(): FloatArray
    private external fun nativeStartDrawLoop()
    private external fun nativeStopDrawLoop()
    private external fun nativeInvalidateContent()
    private external fun nativeUpdateSize()
    private external fun nativeUpdateSurface(surface: Surface?)
    private external fun nativeCreate(): Long
    private external fun nativeRelease()

    companion object {
        private external fun nativeInitSystemProperties(density: Float, fontScale: Float)

        init {
            System.loadLibrary("SeatCanvas")
        }
    }
}
