package com.seatcanvas.sample

import android.graphics.Color

object ColorExtensions {
    fun Int.rgbaHex(): String {
        val r = Color.red(this)
        val g = Color.green(this)
        val b = Color.blue(this)
        val a = Color.alpha(this)
        return String.format("#%02X%02X%02X%02X", r, g, b, a)
    }

    fun Int.argbHex(): String {
        val a = Color.alpha(this)
        val r = Color.red(this)
        val g = Color.green(this)
        val b = Color.blue(this)
        return String.format("#%02X%02X%02X%02X", a, r, g, b)
    }

    fun rgbHexFromArgb(argbHex: String): String? {
        var hexString = argbHex.trim()
        if (hexString.startsWith("#")) {
            hexString = hexString.substring(1)
        }
        if (hexString.length != 8) {
            return null
        }
        return try {
            val r = hexString.substring(2, 4)
            val g = hexString.substring(4, 6)
            val b = hexString.substring(6, 8)
            "#${r}${g}${b}".uppercase()
        } catch (e: Exception) {
            null
        }
    }

    fun colorFromArgbHex(argbHex: String): Int? {
        var hexString = argbHex.trim()
        if (hexString.startsWith("#")) {
            hexString = hexString.substring(1)
        }
        if (hexString.length != 8) {
            return null
        }
        return try {
            val hexValue = hexString.toLong(16)
            val a = ((hexValue shr 24) and 0xFF).toInt()
            val r = ((hexValue shr 16) and 0xFF).toInt()
            val g = ((hexValue shr 8) and 0xFF).toInt()
            val b = (hexValue and 0xFF).toInt()
            Color.argb(a, r, g, b)
        } catch (e: Exception) {
            null
        }
    }
}
