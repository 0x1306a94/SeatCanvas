package com.libseatcanvas

class HitTestSeatRegionResult(val regionId: String, val bounds: Rect) {

    fun isValid(): Boolean {
        return regionId.isNotEmpty()
    }
}