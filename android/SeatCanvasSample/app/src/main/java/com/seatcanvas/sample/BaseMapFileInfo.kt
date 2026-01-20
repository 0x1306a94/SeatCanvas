package com.seatcanvas.sample

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

enum class ResourceScope(val value: String) {
    DEFAULT("default"),
    CUSTOMIZED("customized")
}

@Parcelize
data class BaseMapFileInfo(
    val scopeValue: String,
    val filename: String,
    val assetPath: String
) : Parcelable {
    val scope: ResourceScope
        get() = when (scopeValue) {
            ResourceScope.DEFAULT.value -> ResourceScope.DEFAULT
            ResourceScope.CUSTOMIZED.value -> ResourceScope.CUSTOMIZED
            else -> ResourceScope.DEFAULT
        }

    constructor(scope: ResourceScope, filename: String, assetPath: String) : this(
        scope.value,
        filename,
        assetPath
    )
}
