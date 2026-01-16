package com.libseatcanvas

import android.view.MotionEvent
import kotlin.math.sqrt

internal class SeatCanvasGestureHandler(
    private val onPan: (Int, Float, Float, Double) -> Unit,
    private val onPinch: (Int, Float, Float, Float) -> Unit,
    private val onTap: (Float, Float) -> Unit
) {
    private var isPinching = false
    private var isPanning = false
    private var accumulatedTranslationX = 0f
    private var accumulatedTranslationY = 0f
    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var pinchInitialDistance = 0f
    private var pinchFocusX = 0f
    private var pinchFocusY = 0f

    // 点击检测相关
    private var tapStartX = 0f
    private var tapStartY = 0f
    private var tapStartTime = 0L
    private var panStarted = false // 是否已经开始拖拽手势
    private val tapSlop = 10f // 点击的最大移动距离（像素）
    private val tapTimeout = 300L // 点击的最大时间（毫秒）

    fun handle(event: MotionEvent): Boolean {
        return when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                // 记录点击开始位置和时间
                tapStartX = event.x
                tapStartY = event.y
                tapStartTime = event.eventTime
                panStarted = false
                isPanning = false
                true
            }

            MotionEvent.ACTION_POINTER_DOWN -> {
                if (event.pointerCount == 2) {
                    beginPinch(event)
                }
                true
            }

            MotionEvent.ACTION_MOVE -> {
                if (isPinching && event.pointerCount >= 2) {
                    updatePinch(event)
                } else if (!isPinching) {
                    // 检查移动距离是否超过阈值，决定是点击还是拖拽
                    val dx = event.x - tapStartX
                    val dy = event.y - tapStartY
                    val distance = kotlin.math.sqrt(dx * dx + dy * dy)

                    if (!panStarted && distance > tapSlop) {
                        // 移动距离超过阈值，开始拖拽
                        panStarted = true
                        beginPan(event)
                    } else if (isPanning) {
                        // 已经在拖拽，继续更新
                        updatePan(event)
                    }
                }
                true
            }

            MotionEvent.ACTION_POINTER_UP -> {
                if (isPinching && event.pointerCount - 1 < 2) {
                    endPinch(GestureState.ENDED, event.eventTime.toDouble())
                }
                true
            }

            MotionEvent.ACTION_UP -> {
                if (isPanning) {
                    endPan(GestureState.ENDED, event.eventTime.toDouble())
                } else if (!isPinching && !panStarted && onTap != null) {
                    // 检查是否是点击：移动距离小且时间短，且没有开始拖拽
                    val dx = event.x - tapStartX
                    val dy = event.y - tapStartY
                    val distance = kotlin.math.sqrt(dx * dx + dy * dy)
                    val duration = event.eventTime - tapStartTime

                    if (distance < tapSlop && duration < tapTimeout) {
                        onTap(event.x, event.y)
                    }
                }
                panStarted = false
                true
            }

            MotionEvent.ACTION_CANCEL -> {
                if (isPinching) {
                    endPinch(GestureState.CANCELLED, event.eventTime.toDouble())
                }
                if (isPanning) {
                    endPan(GestureState.CANCELLED, event.eventTime.toDouble())
                }
                panStarted = false
                true
            }

            else -> false
        }
    }

    private fun beginPan(event: MotionEvent) {
        isPanning = true
        accumulatedTranslationX = 0f
        accumulatedTranslationY = 0f
        lastTouchX = event.x
        lastTouchY = event.y
        onPan(GestureState.BEGAN, 0f, 0f, event.eventTime.toDouble())
    }

    private fun updatePan(event: MotionEvent) {
        val dx = event.x - lastTouchX
        val dy = event.y - lastTouchY
        accumulatedTranslationX += -dx
        accumulatedTranslationY += -dy
        lastTouchX = event.x
        lastTouchY = event.y
        onPan(
            GestureState.CHANGED,
            accumulatedTranslationX,
            accumulatedTranslationY,
            event.eventTime.toDouble()
        )
    }

    private fun endPan(state: Int, timestamp: Double) {
        onPan(state, accumulatedTranslationX, accumulatedTranslationY, timestamp)
        isPanning = false
    }

    private fun beginPinch(event: MotionEvent) {
        isPinching = true
        isPanning = false
        pinchInitialDistance = distance(event)
        pinchFocusX = focus(event, axisX = true)
        pinchFocusY = focus(event, axisX = false)
        onPinch(GestureState.BEGAN, 1f, pinchFocusX, pinchFocusY)
    }

    private fun updatePinch(event: MotionEvent) {
        val currentDistance = distance(event)
        if (pinchInitialDistance == 0f) {
            pinchInitialDistance = currentDistance
        }
        if (currentDistance == 0f) {
            return
        }
        val scale = currentDistance / pinchInitialDistance
        pinchFocusX = focus(event, axisX = true)
        pinchFocusY = focus(event, axisX = false)
        onPinch(GestureState.CHANGED, scale, pinchFocusX, pinchFocusY)
    }

    private fun endPinch(state: Int, timestamp: Double) {
        onPinch(state, 1f, pinchFocusX, pinchFocusY)
        isPinching = false
    }

    private fun distance(event: MotionEvent): Float {
        if (event.pointerCount < 2) return 0f
        val dx = event.getX(0) - event.getX(1)
        val dy = event.getY(0) - event.getY(1)
        return sqrt(dx * dx + dy * dy)
    }

    private fun focus(event: MotionEvent, axisX: Boolean): Float {
        if (event.pointerCount < 2) return if (axisX) event.x else event.y
        val first = if (axisX) event.getX(0) else event.getY(0)
        val second = if (axisX) event.getX(1) else event.getY(1)
        return (first + second) / 2f
    }
}

internal object GestureState {
    const val POSSIBLE = 0
    const val BEGAN = 1
    const val CHANGED = 2
    const val ENDED = 3
    const val CANCELLED = 4
}

