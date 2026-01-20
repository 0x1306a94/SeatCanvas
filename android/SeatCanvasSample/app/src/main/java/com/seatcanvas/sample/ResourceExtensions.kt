package com.seatcanvas.sample

import android.content.Context

fun Context.defaultSeatStylePath(name: String): String {
    return "SeatCanvasSampleAssets/${ResourceScope.DEFAULT.value}/seatstyle/$name"
}

fun Context.defaultBaseMaps(): List<BaseMapFileInfo> {
    val baseMapPath = "SeatCanvasSampleAssets/${ResourceScope.DEFAULT.value}/basemap"
    return try {
        assets.list(baseMapPath)?.mapNotNull { filename ->
            BaseMapFileInfo(
                scope = ResourceScope.DEFAULT,
                filename = filename,
                assetPath = "$baseMapPath/$filename"
            )
        } ?: emptyList()
    } catch (e: Exception) {
        e.printStackTrace()
        emptyList()
    }
}

fun Context.customizedBaseMaps(): List<BaseMapFileInfo> {
    val baseMapPath = "SeatCanvasSampleAssets/${ResourceScope.CUSTOMIZED.value}/basemap"
    return try {
        assets.list(baseMapPath)?.mapNotNull { filename ->
            BaseMapFileInfo(
                scope = ResourceScope.CUSTOMIZED,
                filename = filename,
                assetPath = "$baseMapPath/$filename"
            )
        } ?: emptyList()
    } catch (e: Exception) {
        e.printStackTrace()
        emptyList()
    }
}

object ResourceExtensions {
    fun zoneDataPath(context: Context, scope: ResourceScope, name: String): String {
        val jsonName = name.replace(".svg", ".json")
        return "SeatCanvasSampleAssets/${scope.value}/zonedata/$jsonName"
    }

    fun seatDataPath(context: Context, scope: ResourceScope, name: String): String {
        val jsonName = name.replace(".svg", ".json")
        return "SeatCanvasSampleAssets/${scope.value}/seatdata/$jsonName"
    }
}
