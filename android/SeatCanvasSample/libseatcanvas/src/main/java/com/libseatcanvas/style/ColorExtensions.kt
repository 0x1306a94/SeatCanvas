package com.libseatcanvas.style

import android.graphics.Color

fun Int.toRGBAHex(): String {
    val r = Color.red(this)
    val g = Color.green(this)
    val b = Color.blue(this)
    val a = Color.alpha(this)
    return String.format("#%02X%02X%02X%02X", r, g, b, a)
}

fun Int.toARGBHex(): String {
    val r = Color.red(this)
    val g = Color.green(this)
    val b = Color.blue(this)
    val a = Color.alpha(this)
    return String.format("#%02X%02X%02X%02X", a, r, g, b)
}