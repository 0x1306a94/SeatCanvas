package com.libseatcanvas

class Rect(val x: Float, val y: Float, val width: Float, val height: Float) {

    fun centerX(): Float {
        return x + width * 0.5f
    }

    fun centerY(): Float {
        return y + height * 0.5f
    }

    fun isEmpty(): Boolean {
        return x == 0.0f && y == 0.0f && width == 0.0f && height == 0.0f
    }
}