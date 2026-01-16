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
        val metrics = resources.displayMetrics
        surface = Surface(p0)
        nativeUpdateSurface(surface!!, metrics.density)

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
        val metrics = resources.displayMetrics
        nativeUpdateSurface(null, metrics.density)
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

    fun loadBaseMap(data: ByteArray?) {
        val result = nativeLoadBaseMapFromSVG(data)
        nativeLoadBaseMap(result)
    }

    fun enableTiled(enable: Boolean) {
        nativeEnableTiled(enable)
    }

    fun enableZoomBlur(enable: Boolean) {
        nativeEnableZoomBlur(enable)
    }

    private fun zoomToRect(bounds: Rect, animated: Boolean = true, padding: Float = 0f, durationMs: Double = 300.0) {
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

    private external fun nativeLoadBaseMapFromSVG(data: ByteArray?): Long
    private external fun nativeLoadBaseMap(ptr: Long): Boolean
    private external fun nativeHandleTap(x: Float, y: Float)
    private external fun nativeHandlePan(state: Int, tx: Float, ty: Float, timestampMs: Double)
    private external fun nativeHandlePinch(state: Int, scale: Float, cx: Float, cy: Float)

    private external fun nativeEnableTiled(enable: Boolean)
    private external fun nativeEnableZoomBlur(enable: Boolean)
    private external fun nativeSeatRegionByPoint(x: Float, y: Float): HitTestSeatRegionResult?
    private external fun nativeZoomToRect(bounds: Rect, animated: Boolean, padding: Float, durationMs: Double)
    private external fun nativeGetZoomScale(): Float
    private external fun nativeGetContentOffset(): FloatArray
    private external fun nativeStartDrawLoop()
    private external fun nativeStopDrawLoop()
    private external fun nativeInvalidateContent()
    private external fun nativeUpdateSize()
    private external fun nativeUpdateSurface(surface: Surface?, density: Float)
    private external fun nativeCreate(): Long
    private external fun nativeRelease()

    companion object {
        init {
            System.loadLibrary("SeatCanvas")
        }
    }
}