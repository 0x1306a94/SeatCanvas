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
import com.libseatcanvas.ZoomLevel

class SeatCanvasSampleModel {
    var prices: List<MockPriceData> = emptyList()
    val seatsMap: MutableMap<String, MockSeatData> = mutableMapOf()
    var seatZoneMap: Map<String, List<MockSeatData>> = emptyMap()
    val availableSeats: MutableMap<String, MutableSet<String>> = mutableMapOf()
    val selectedSeatIds: MutableSet<String> = mutableSetOf()

    private val handler = Handler(Looper.getMainLooper())
    private var refreshRunnable: Runnable? = null
    private var availableSeatsTimerPaused = false
    private var isViewportInteracting = false
    private var seatCanvasView: SeatCanvasView? = null
    private var appContext: Context? = null
    private var baseMapInfo: BaseMapFileInfo? = null

    fun attach(context: Context, baseMapInfo: BaseMapFileInfo, seatCanvasView: SeatCanvasView) {
        this.appContext = context.applicationContext
        this.baseMapInfo = baseMapInfo
        this.seatCanvasView = seatCanvasView
    }

    val rendererDelegate: SeatCanvasRendererDelegate = object : SeatCanvasRendererDelegate {
        override fun didLoadBaseMap() {
            val context = appContext ?: return
            val info = baseMapInfo ?: return
            val view = seatCanvasView ?: return
            loadMockData(context, info, view)
            view.applySeatStyleJSONConfig(
                SeatStyleBuilder.buildSVGSeatStyleConfig(context, prices)
            )
            regenerateRandomAvailableSeats(fullRefresh = true)
            startAvailableSeatsTimer()
        }

        override fun didUnloadBaseMap() {
            stopAvailableSeatsTimer()
            availableSeats.clear()
            selectedSeatIds.clear()
        }

        override fun didUpdateZoomLevelConfig(
            zoomLevels: ZoomLevel,
            minimumZoomScale: Float,
            maximumZoomScale: Float,
            zoomScale: Float
        ) {
            seatCanvasView?.seatRenderZoomThreshold = zoomLevels.venue
        }

        override fun didTapSeat(zoneId: String, seatId: String): Boolean {
            val previousSelected = selectedSeatIds.toSet()
            var changed = false
            if (selectedSeatIds.contains(seatId)) {
                selectedSeatIds.remove(seatId)
                changed = true
            } else if (seatAvailable(zoneId, seatId)) {
                selectedSeatIds.add(seatId)
                changed = true
            }
            if (changed) {
                pushSelectedSeatIds(previousSelected)
            }
            Log.d(TAG, "didTapSeat: zoneId=$zoneId, seatId=$seatId")
            return changed
        }

        override fun didTapZone(zoneId: String) {
            Log.d(TAG, "didTapZone: zoneId=$zoneId")
        }

        override fun willBeginDragging(viewport: SeatCanvasViewport) {
            Log.d(TAG, "willBeginDragging: viewport=$viewport")
            beginViewportInteraction()
        }

        override fun didScroll(viewport: SeatCanvasViewport) {
//            Log.d(TAG, "didScroll: viewport=$viewport")
        }

        override fun didEndDragging(viewport: SeatCanvasViewport, decelerate: Boolean) {
            Log.d(TAG, "didEndDragging: viewport=$viewport decelerate=$decelerate")
            if (!decelerate) {
                endViewportInteraction()
            }
        }

        override fun didEndDecelerating(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didEndDecelerating: viewport=$viewport")
            endViewportInteraction()
        }

        override fun willBeginZooming(viewport: SeatCanvasViewport) {
            Log.d(TAG, "willBeginZooming: viewport=$viewport")
            beginViewportInteraction()
        }

        override fun didZoom(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didZoom: viewport=$viewport")
        }

        override fun didEndZooming(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didEndZooming: viewport=$viewport")
            endViewportInteraction()
        }

        override fun didEndScrollingAnimation(viewport: SeatCanvasViewport) {
            Log.d(TAG, "didEndScrollingAnimation: viewport=$viewport")
            endViewportInteraction()
        }


    }

    fun loadMockPrice(context: Context, baseMapInfo: BaseMapFileInfo): List<MockPriceData> {
        val path =
            ResourceExtensions.priceDataPath(context, baseMapInfo.scope, baseMapInfo.filename)
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

    fun loadMockData(
        context: Context,
        baseMapInfo: BaseMapFileInfo,
        seatCanvasView: SeatCanvasView
    ) {
        this.seatCanvasView = seatCanvasView
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

        seatCanvasView.registerPricecodes(prices.map { it.code }.toTypedArray())

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
                    rotation = mockSeat.rotation,
                    pricecode = mockSeat.pricecode.takeIf { it.isNotEmpty() }
                )
            }.toTypedArray()
            seatCanvasView.updateSeats(zoneId, seatDataArray)
            seatCanvasView.updateSeatStatusesForZone(zoneId, buildStatusesForZone(zoneId, seats))
        }
        seatCanvasView.setSelectedSeatIds(selectedSeatIds.toTypedArray())
    }

    private fun loadSeatDatas(
        context: Context,
        baseMapInfo: BaseMapFileInfo
    ): Map<String, List<MockSeatData>> {
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

    fun regenerateRandomAvailableSeats(fullRefresh: Boolean = false) {
        val view = seatCanvasView ?: return

        val zoneIds = if (fullRefresh) {
            seatZoneMap.keys.toList()
        } else {
            if (view.zoomScale < view.seatRenderZoomThreshold) {
                return
            }
            val zoneIdsInView = view.zoneIdsInOriginalRect(view.visibleOriginalRect())
            if (zoneIdsInView.isEmpty()) {
                return
            }
            zoneIdsInView
        }

        if (fullRefresh) {
            availableSeats.clear()
        }

        for (zoneId in zoneIds) {
            val seats = seatZoneMap[zoneId] ?: continue
            if (seats.isEmpty()) {
                continue
            }
            val ratio = 0.2 + Math.random() * 0.7
            val availableCount = maxOf(1, (seats.size * ratio).toInt())
            val availableIds =
                seats.shuffled().take(availableCount).map { it.seatId }.toMutableSet()
            availableSeats[zoneId] = availableIds
            view.updateSeatStatusesForZone(zoneId, buildStatusesForZone(zoneId, seats))
        }
        pruneSelectedSeatsForAvailability()
        view.setSelectedSeatIds(selectedSeatIds.toTypedArray())

        if (!fullRefresh) {
            Log.d(TAG, "refreshAvailableSeatsForVisibleZones: zoneIds=$zoneIds")
        }
    }

    private fun beginViewportInteraction() {
        if (isViewportInteracting) {
            return
        }
        isViewportInteracting = true
        pauseAvailableSeatsTimer()
    }

    private fun endViewportInteraction() {
        if (!isViewportInteracting) {
            return
        }
        isViewportInteracting = false
        regenerateRandomAvailableSeats(fullRefresh = false)
        resumeAvailableSeatsTimer()
    }

    private fun buildStatusesForZone(zoneId: String, seats: List<MockSeatData>): IntArray {
        val statuses = IntArray(seats.size)
        val availableIds = availableSeats[zoneId]
        for (index in seats.indices) {
            statuses[index] = if (availableIds?.contains(seats[index].seatId) == true) {
                SeatStatus.AVAILABLE
            } else {
                SeatStatus.UNAVAILABLE
            }
        }
        return statuses
    }

    private fun pushSelectedSeatIds(previousSelected: Set<String>) {
        val view = seatCanvasView ?: return
        val added = selectedSeatIds.filter { !previousSelected.contains(it) }.toTypedArray()
        val removed = previousSelected.filter { !selectedSeatIds.contains(it) }.toTypedArray()
        view.updateSelectedSeatIds(added, removed)
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
        availableSeatsTimerPaused = false
        val runnable = object : Runnable {
            override fun run() {
                if (availableSeatsTimerPaused) {
                    return
                }
                regenerateRandomAvailableSeats(fullRefresh = false)
                handler.postDelayed(this, REFRESH_INTERVAL_MS)
            }
        }
        refreshRunnable = runnable
        handler.postDelayed(runnable, REFRESH_INTERVAL_MS)
    }

    fun pauseAvailableSeatsTimer() {
        availableSeatsTimerPaused = true
        refreshRunnable?.let { handler.removeCallbacks(it) }
    }

    fun resumeAvailableSeatsTimer() {
        if (!availableSeatsTimerPaused) {
            return
        }
        availableSeatsTimerPaused = false
        refreshRunnable?.let { handler.postDelayed(it, REFRESH_INTERVAL_MS) }
    }

    fun stopAvailableSeatsTimer() {
        availableSeatsTimerPaused = false
        isViewportInteracting = false
        refreshRunnable?.let { handler.removeCallbacks(it) }
        refreshRunnable = null
    }

    fun detachSeatCanvasView() {
        seatCanvasView = null
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
