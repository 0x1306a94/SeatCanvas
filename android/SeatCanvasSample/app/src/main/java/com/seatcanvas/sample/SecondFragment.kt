package com.seatcanvas.sample

import android.content.Context
import android.graphics.Color
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.gson.Gson
import com.google.gson.reflect.TypeToken
import com.libseatcanvas.BaseMapFormat
import com.libseatcanvas.SeatCanvasRendererDelegate
import com.libseatcanvas.SeatData
import com.libseatcanvas.SeatZoneColor
import com.libseatcanvas.SVGBaseMapParseConfig
import com.libseatcanvas.SeatCanvasViewport
import com.libseatcanvas.style.SeatStyleConfigBuilder
import com.seatcanvas.sample.databinding.FragmentSecondBinding

class SecondFragment : Fragment() {

    private var _binding: FragmentSecondBinding? = null
    private val binding get() = _binding!!

    private val seatStatusMap: MutableMap<String, UInt> = mutableMapOf()
    private val selectedSeatIds: MutableSet<String> = mutableSetOf()

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSecondBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val baseMapInfo = arguments?.getParcelable<BaseMapFileInfo>(ARG_BASE_MAP_INFO) ?: return
        val data = readAssetFileToByteArray(requireContext(), baseMapInfo.assetPath) ?: return
        val parseConfig = SVGBaseMapParseConfig(listOf("zoneId", "regioncode"))
        binding.seatCanvasView.loadBaseMap(data, BaseMapFormat.SVG, parseConfig)
        binding.seatCanvasView.canvasColor = ContextCompat.getColor(requireContext(), R.color.canvas_bg)
        binding.seatCanvasView.applySeatStyleJSONConfig(buildSVGSeatStyleConfig())

        setupStyleSwitches()

        binding.seatCanvasView.setDelegate(object : SeatCanvasRendererDelegate {
            /**
             * 根据区域与座位返回样式 ID；返回 null 表示不绘制该座位。
             */
            override fun styleIdForSeat(zoneId: String, seatId: String): String? {
                val status = seatStatusMap[seatId] ?: return null
                return styleId(status, selectedSeatIds.contains(seatId))
            }

            /**
             * 点击座位；若业务状态发生变化且需要刷新视图，返回 true。
             */
            override fun didTapSeat(zoneId: String, seatId: String): Boolean {
                if (selectedSeatIds.contains(seatId)) {
                    selectedSeatIds.remove(seatId)
                } else {
                    selectedSeatIds.add(seatId)
                }
                Log.d("SecondFragment", "didTapSeat: zoneId=$zoneId, seatId=$seatId")
                return true
            }

            /**
             * 点击区域（非座位区域）。
             */
            override fun didTapZone(zoneId: String) {
                Log.d("SecondFragment", "didTapZone: zoneId=$zoneId")
            }

            /**
             * 即将开始拖动视图。
             * @param viewport 当前视图状态。
             */
            override fun willBeginDragging(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "willBeginDragging: viewport=$viewport")
            }

            /**
             * 视图发生滚动。
             * @param viewport 当前视图状态。
             */
            override fun didScroll(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "didScroll: viewport=$viewport")
            }

            /**
             * 拖动手势结束。
             * @param viewport 当前视图状态。
             * @param decelerate 是否会继续惯性滚动或回弹动画。
             */
            override fun didEndDragging(viewport: SeatCanvasViewport, decelerate: Boolean) {
                Log.d("SecondFragment", "didEndDragging: viewport=$viewport decelerate=$decelerate")
            }

            /**
             * 惯性滚动或回弹动画结束。
             * @param viewport 当前视图状态。
             */
            override fun didEndDecelerating(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "didEndDecelerating: viewport=$viewport")
            }

            /**
             * 即将开始缩放视图。
             * @param viewport 当前视图状态。
             */
            override fun willBeginZooming(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "willBeginZooming: viewport=$viewport")
            }

            /**
             * 视图发生缩放。
             * @param viewport 当前视图状态。
             */
            override fun didZoom(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "didZoom: viewport=$viewport")
            }

            /**
             * 缩放手势结束。
             * @param viewport 当前视图状态。
             */
            override fun didEndZooming(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "didEndZooming: viewport=$viewport")
            }

            /**
             * 程序触发的滚动或缩放动画结束。
             * @param viewport 当前视图状态。
             */
            override fun didEndScrollingAnimation(viewport: SeatCanvasViewport) {
                Log.d("SecondFragment", "didEndScrollingAnimation: viewport=$viewport")
            }
        })

        loadMockData(baseMapInfo)
    }

    private fun styleId(status: UInt, selected: Boolean): String {
        return "status_${status}_selected_${if (selected) 1 else 0}"
    }

    private fun loadMockData(baseMapInfo: BaseMapFileInfo) {
        seatStatusMap.clear()
        selectedSeatIds.clear()

        val colors = loadZoneColors(baseMapInfo)
        binding.seatCanvasView.updateSeatZoneAlternateColors(colors)
        binding.seatCanvasView.updateMiniMapZoneAlternateColors(colors)

        val seatDatas = loadSeatDatas(baseMapInfo)
        for ((zoneId, seats) in seatDatas) {
            for (mockSeat in seats) {
                seatStatusMap[mockSeat.seatId] = mockSeat.status
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
            binding.seatCanvasView.updateSeats(zoneId, seatDataArray)
        }
    }

    private fun loadZoneColors(baseMapInfo: BaseMapFileInfo): Array<SeatZoneColor> {
        val path = ResourceExtensions.zoneDataPath(
            requireContext(),
            baseMapInfo.scope,
            baseMapInfo.filename
        )
        val jsonString = readAssetFileToString(requireContext(), path) ?: return emptyArray()
        return try {
            val gson = Gson()
            val type = object : TypeToken<List<MockZoneInfo>>() {}.type
            val zones: List<MockZoneInfo> = gson.fromJson(jsonString, type)
            zones.map { zone ->
                SeatZoneColor(
                    zoneId = zone.zoneId,
                    alternateColor = zone.alternateColor
                )
            }.toTypedArray()
        } catch (e: Exception) {
            e.printStackTrace()
            emptyArray()
        }
    }

    private fun loadSeatDatas(baseMapInfo: BaseMapFileInfo): Map<String, List<MockSeatData>> {
        val path = ResourceExtensions.seatDataPath(
            requireContext(),
            baseMapInfo.scope,
            baseMapInfo.filename
        )
        val jsonString = readAssetFileToString(requireContext(), path) ?: return emptyMap()
        return try {
            val gson = Gson()
            val type = object : TypeToken<Map<String, List<MockSeatData>>>() {}.type
            gson.fromJson(jsonString, type)
        } catch (e: Exception) {
            e.printStackTrace()
            emptyMap()
        }
    }

    override fun onDestroyView() {
        binding.seatCanvasView.onDestroy()
        super.onDestroyView()
        _binding = null
    }

    private fun setupStyleSwitches() {
        binding.circleSeatStyleSwitch.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                binding.svgSeatStyleSwitch.isChecked = false
                binding.seatCanvasView.applySeatStyleJSONConfig(buildCircleSeatStyleConfig())
            }
        }

        binding.svgSeatStyleSwitch.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                binding.circleSeatStyleSwitch.isChecked = false
                binding.seatCanvasView.applySeatStyleJSONConfig(buildSVGSeatStyleConfig())
            }
        }
    }

    private fun buildCircleSeatStyleConfig(): ByteArray? {
        val builder = SeatStyleConfigBuilder()
        val context = requireContext()

        val available = ContextCompat.getColor(context, R.color.seat_available)
        val sold = ContextCompat.getColor(context, R.color.seat_sold)
        val locked = ContextCompat.getColor(context, R.color.seat_locked)
        val disabled = ContextCompat.getColor(context, R.color.seat_disabled)
        val overlay = Color.argb((0.7 * 255).toInt(), 0, 0, 0)
        val checkmark = Color.WHITE

        for (status in 0U..3U) {
            val fill = when (status.toInt()) {
                0 -> available
                1 -> sold
                2 -> locked
                else -> disabled
            }
            builder.addCircleStyle(styleId = styleId(status, false), fill = fill)
            builder.addCircleStyle(
                styleId = styleId(status, true),
                fill = fill,
                overlay = overlay,
                checkmark = checkmark
            )
        }

        return builder.toJSONData()
    }

    private fun buildSVGSeatStyleConfig(): ByteArray? {
        val builder = SeatStyleConfigBuilder()

        val available = loadSVGContent("icon_seat_selectable.svg")
        val selected = loadSVGContent("icon_seat_selected.svg")
        val disabled = loadSVGContent("icon_seat_nonselectable.svg")

        if (available == null || selected == null || disabled == null) {
            return null
        }

        builder.addSVGStyle(styleId = styleId(0U, false), content = available)
        builder.addSVGStyle(styleId = styleId(0U, true), content = selected)
        for (status in 1U..3U) {
            builder.addSVGStyle(styleId = styleId(status, false), content = disabled)
            builder.addSVGStyle(styleId = styleId(status, true), content = disabled)
        }

        return builder.toJSONData()
    }

    private fun loadSVGContent(name: String): String? {
        return try {
            val fileName = requireContext().defaultSeatStylePath(name)
            requireContext().assets.open(fileName).use { inputStream ->
                inputStream.bufferedReader().use { reader ->
                    reader.readText()
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    private fun readAssetFileToString(context: Context, fileName: String): String? {
        return try {
            context.assets.open(fileName).use { inputStream ->
                inputStream.bufferedReader().use { reader ->
                    reader.readText()
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    private fun readAssetFileToByteArray(context: Context, fileName: String): ByteArray? {
        return try {
            context.assets.open(fileName).use { inputStream ->
                inputStream.readBytes()
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }

    companion object {
        const val ARG_BASE_MAP_INFO = "baseMapInfo"
    }
}
