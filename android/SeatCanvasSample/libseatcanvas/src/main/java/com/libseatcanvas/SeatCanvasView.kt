package com.libseatcanvas

import android.content.Context
import android.graphics.SurfaceTexture
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.Surface
import android.view.TextureView
import android.view.View

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
     * 设置座位渲染器代理
     * @param delegate 代理对象，用于处理座位选择相关的回调
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

    fun loadBaseMap(data: ByteArray?, format: BaseMapFormat) {
        val result = nativeLoadBaseMapFromFormat(data, format.name)
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

    fun updateSeatZones(zones: Array<SeatZoneData>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSeatZones(zones)
    }

    fun updateSeats(zoneId: String, seats: Array<SeatData>) {
        if (!nativeInitialized()) {
            return
        }
        nativeUpdateSeats(zoneId, seats)
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

    /**
     * 由 C++ 层调用，转发给 delegate
     * @param regionId 区域ID
     * @param seatId 座位ID
     * @return true 表示可以选中，false 表示不能选中
     */
    private fun nativeOnShouldSelectSeat(regionId: String, seatId: String): Boolean {
        return delegate?.shouldSelectSeat(regionId, seatId) ?: false
    }

    /**
     * 由 C++ 层调用，转发给 delegate
     * @param regionId 区域ID
     * @param seatId 座位ID
     */
    private fun nativeOnDidSelectSeat(regionId: String, seatId: String) {
        delegate?.didSelectSeat(regionId, seatId)
    }

    /**
     * 由 C++ 层调用，转发给 delegate
     * @param regionId 区域ID
     * @param seatId 座位ID
     */
    private fun nativeOnDidDeselectSeat(regionId: String, seatId: String) {
        delegate?.didDeselectSeat(regionId, seatId)
    }

    private external fun nativeLoadBaseMapFromFormat(data: ByteArray?, formatName: String): Long
    private external fun nativeLoadBaseMap(ptr: Long): Boolean
    private external fun nativeSetSeatStyleJSONConfig(data: ByteArray?, len: Int)
    private external fun nativeUpdateSeatZones(zones: Array<SeatZoneData>)
    private external fun nativeUpdateSeats(zoneId: String, seats: Array<SeatData>)
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