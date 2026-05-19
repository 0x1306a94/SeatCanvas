package com.seatcanvas.sample

import android.content.Context
import android.os.Handler
import android.os.Looper
import android.util.Log
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import com.libseatcanvas.SeatCanvasRendererDelegate
import com.libseatcanvas.SeatCanvasView
import com.libseatcanvas.SeatCanvasViewport
import com.libseatcanvas.SeatData
import com.libseatcanvas.SeatZoneColor

class SeatCanvasSampleModel {
    var prices: List<MockPriceData> = emptyList()
    val seatsMap: MutableMap<String, MockSeatData> = mutableMapOf()
    var seatZoneMap: Map<String, List<MockSeatData>> = emptyMap()
    val availableSeats: MutableMap<String, MutableSet<String>> = mutableMapOf()
    val selectedSeatIds: MutableSet<String> = mutableSetOf()

    private val handler = Handler(Looper.getMainLooper())
    private var refreshRunnable: Runnable? = null

    val rendererDelegate: SeatCanvasRendererDelegate = object : SeatCanvasRendererDelegate {
        override fun styleIdForSeat(zoneId: String, seatId: String): String? {
            val seat = seatsMap[seatId] ?: return null
            val available = seatAvailable(zoneId, seatId)
            return styleId(seat.pricecode, available, selectedSeatIds.contains(seatId))
        }

        override fun didTapSeat(zoneId: String, seatId: String): Boolean {
            var changed = false
            if (selectedSeatIds.contains(seatId)) {
                selectedSeatIds.remove(seatId)
                changed = true
            } else if (seatAvailable(zoneId, seatId)) {
                selectedSeatIds.add(seatId)
                changed = true
            }
            Log.d(TAG, "didTapSeat: zoneId=$zoneId, seatId=$seatId")
            return changed
        }

        override fun didTapZone(zoneId: String) {
            Log.d(TAG, "didTapZone: zoneId=$zoneId")
        }

        override fun willBeginDragging(viewport: SeatCanvasViewport) {
            Log.d(TAG, "willBeginDragging: viewport=$viewport")
        }

        override fun didScroll(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didScroll: viewport=$viewport")
        }

        override fun didEndDragging(viewport: SeatCanvasViewport, decelerate: Boolean) {
            Log.d(TAG, "didEndDragging: viewport=$viewport decelerate=$decelerate")
        }

        override fun didEndDecelerating(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didEndDecelerating: viewport=$viewport")
        }

        override fun willBeginZooming(viewport: SeatCanvasViewport) {
            Log.d(TAG, "willBeginZooming: viewport=$viewport")
        }

        override fun didZoom(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didZoom: viewport=$viewport")
        }

        override fun didEndZooming(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didEndZooming: viewport=$viewport")
        }

        override fun didEndScrollingAnimation(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didEndScrollingAnimation: viewport=$viewport")
        }
    }

    fun styleId(pricecode: String, available: Boolean, selected: Boolean): String {
        return SeatStyleBuilder.styleId(pricecode, available, selected)
    }

    fun loadMockPrice(context: Context, baseMapInfo: BaseMapFileInfo): List<MockPriceData> {
        val path = ResourceExtensions.priceDataPath(context, baseMapInfo.scope, baseMapInfo.filename)
        val jsonString = readAssetFileToString(context, path) ?: return emptyList()
        return try {
            val gson = Gson()
            val type = object : TypeToken<List<MockPriceData>>() {}.type
            gson.fromJson(jsonString, type)
        } catch (e: Exception) {
            e.printStackTrace()
            emptyList()
        }
    }

    fun loadMockData(context: Context, baseMapInfo: BaseMapFileInfo, seatCanvasView: SeatCanvasView) {
        prices = loadMockPrice(context, baseMapInfo)
        val zoneColors = mutableListOf<SeatZoneColor>()
        for (price in prices) {
            val alternateColor = ColorExtensions.colorFromArgbHex(price.color) ?: continue
            for (zoneId in price.zoneIds) {
                zoneColors.add(SeatZoneColor(zoneId = zoneId, alternateColor = alternateColor))
            }
        }
        seatCanvasView.updateSeatZoneAlternateColors(zoneColors.toTypedArray())
        seatCanvasView.updateMiniMapZoneAlternateColors(zoneColors.toTypedArray())

        seatsMap.clear()
        seatZoneMap = loadSeatDatas(context, baseMapInfo)
        selectedSeatIds.clear()
        for ((zoneId, seats) in seatZoneMap) {
            for (mockSeat in seats) {
                seatsMap[mockSeat.seatId] = mockSeat
                if (mockSeat.selected) {
                    selectedSeatIds.add(mockSeat.seatId)
                }
            }
            val seatDataArray = seats.map { mockSeat ->
                SeatData(
                    seatId = mockSeat.seatId,
                    x = mockSeat.x,
                    y = mockSeat.y,
                    rotation = mockSeat.rotation
                )
            }.toTypedArray()
            seatCanvasView.updateSeats(zoneId, seatDataArray)
        }
    }

    private fun loadSeatDatas(context: Context, baseMapInfo: BaseMapFileInfo): Map<String, List<MockSeatData>> {
        val path = ResourceExtensions.seatDataPath(context, baseMapInfo.scope, baseMapInfo.filename)
        val jsonString = readAssetFileToString(context, path) ?: return emptyMap()
        return try {
            val gson = Gson()
            val type = object : TypeToken<Map<String, List<MockSeatData>>>() {}.type
            gson.fromJson(jsonString, type)
        } catch (e: Exception) {
            e.printStackTrace()
            emptyMap()
        }
    }

    fun seatAvailable(zoneId: String, seatId: String): Boolean {
        return availableSeats[zoneId]?.contains(seatId) == true
    }

    fun regenerateRandomAvailableSeats() {
        availableSeats.clear()
        for ((zoneId, seats) in seatZoneMap) {
            if (seats.isEmpty()) {
                continue
            }
            val ratio = 0.2 + Math.random() * 0.7
            val availableCount = maxOf(1, (seats.size * ratio).toInt())
            val availableIds = seats.shuffled().take(availableCount).map { it.seatId }.toMutableSet()
            availableSeats[zoneId] = availableIds
        }
        pruneSelectedSeatsForAvailability()
    }

    private fun pruneSelectedSeatsForAvailability() {
        val kept = selectedSeatIds.filter { seatId ->
            seatZoneMap.any { (zoneId, seats) ->
                seats.any { it.seatId == seatId } && seatAvailable(zoneId, seatId)
            }
        }.toMutableSet()
        selectedSeatIds.clear()
        selectedSeatIds.addAll(kept)
    }

    fun startAvailableSeatsTimer() {
        stopAvailableSeatsTimer()
        val runnable = object : Runnable {
            override fun run() {
                regenerateRandomAvailableSeats()
                handler.postDelayed(this, REFRESH_INTERVAL_MS)
            }
        }
        refreshRunnable = runnable
        handler.post(runnable)
    }

    fun stopAvailableSeatsTimer() {
        refreshRunnable?.let { handler.removeCallbacks(it) }
        refreshRunnable = null
    }

    private fun readAssetFileToString(context: Context, fileName: String): String? {
        return try {
            context.assets.open(fileName).use { inputStream ->
                inputStream.bufferedReader().use { reader -> reader.readText() }
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    companion object {
        private const val TAG = "SeatCanvasSampleModel"
        private const val REFRESH_INTERVAL_MS = 10_000L
    }
}
