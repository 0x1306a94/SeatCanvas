package com.seatcanvas.sample

import android.content.Context
import android.graphics.Color
import android.os.Bundle
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
import com.libseatcanvas.SeatZoneData
import com.libseatcanvas.style.SeatStyleConfigBuilder
import com.seatcanvas.sample.databinding.FragmentSecondBinding

/**
 * A simple [Fragment] subclass as the second destination in the navigation.
 */
class SecondFragment : Fragment() {

    private var _binding: FragmentSecondBinding? = null

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentSecondBinding.inflate(inflater, container, false)
        return binding.root

    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        val baseMapInfo = arguments?.getParcelable<BaseMapFileInfo>(ARG_BASE_MAP_INFO) ?: return
        val data = readAssetFileToByteArray(requireContext(), baseMapInfo.assetPath) ?: return
        binding.seatCanvasView.loadBaseMap(data, BaseMapFormat.SVG)
        binding.seatCanvasView.canvasColor = ContextCompat.getColor(context, R.color.canvas_bg)
        // 应用默认样式（SVG样式）
        binding.seatCanvasView.applySeatStyleJSONConfig(buildSVGSeatStyleConfig())

        // 设置样式切换开关监听器
        setupStyleSwitches()

        // 设置座位选择代理
        binding.seatCanvasView.setDelegate(object : SeatCanvasRendererDelegate {
            override fun didTapZone(zoneId: String) {
                android.util.Log.d(
                    "SecondFragment",
                    "didTapZone: zoneId=$zoneId"
                )
            }

            override fun shouldSelectSeat(zoneId: String, seatId: String): Boolean {
                // 返回 true 表示可以选中，false 表示不能选中
                return true
            }

            override fun didSelectSeat(zoneId: String, seatId: String) {
                android.util.Log.d(
                    "SecondFragment",
                    "didSelectSeat: zoneId=$zoneId, seatId=$seatId"
                )
            }

            override fun didDeselectSeat(zoneId: String, seatId: String) {
                android.util.Log.d(
                    "SecondFragment",
                    "didDeselectSeat: zoneId=$zoneId, seatId=$seatId"
                )
            }
        })

        loadMockData(baseMapInfo)
    }

    private fun loadMockData(baseMapInfo: BaseMapFileInfo) {
        val zoneDatas = loadZoneDatas(baseMapInfo)
        binding.seatCanvasView.updateSeatZones(zoneDatas)

        val seatDatas = loadSeatDatas(baseMapInfo)
        for ((zoneId, seats) in seatDatas) {
            val seatDataArray = seats.map { mockSeat ->
                SeatData(
                    seatId = mockSeat.seatId,
                    status = mockSeat.status,
                    selected = mockSeat.selected,
                    x = mockSeat.x,
                    y = mockSeat.y
                )
            }.toTypedArray()
            binding.seatCanvasView.updateSeats(zoneId, seatDataArray)
        }
    }

    private fun loadZoneDatas(baseMapInfo: BaseMapFileInfo): Array<SeatZoneData> {
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
                SeatZoneData(
                    zoneId = zone.zoneId,
                    color = zone.color,
                    priceColor = zone.priceColor
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

    override fun onPause() {
        super.onPause()
    }

    override fun onResume() {
//        binding.seatCanvasView.onResume()
        super.onResume()
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
        val overlay = Color.argb((0.7 * 255).toInt(), 0, 0, 0) // 黑色，透明度 0.7
        val checkmark = Color.WHITE

        builder.addCircleStyle(
            status = 0U,
            selected = false,
            fill = available,
            overlay = overlay,
            checkmark = checkmark
        )
        builder.addCircleStyle(
            status = 0U,
            selected = true,
            fill = available,
            overlay = overlay,
            checkmark = checkmark
        )

        builder.addCircleStyle(
            status = 1U,
            selected = false,
            fill = sold,
            overlay = overlay,
            checkmark = checkmark
        )
        builder.addCircleStyle(
            status = 1U,
            selected = true,
            fill = sold,
            overlay = overlay,
            checkmark = checkmark
        )

        builder.addCircleStyle(
            status = 2U,
            selected = false,
            fill = locked,
            overlay = overlay,
            checkmark = checkmark
        )
        builder.addCircleStyle(
            status = 2U,
            selected = true,
            fill = locked,
            overlay = overlay,
            checkmark = checkmark
        )

        builder.addCircleStyle(
            status = 3U,
            selected = false,
            fill = disabled,
            overlay = overlay,
            checkmark = checkmark
        )
        builder.addCircleStyle(
            status = 3U,
            selected = true,
            fill = disabled,
            overlay = overlay,
            checkmark = checkmark
        )

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

        builder.addSVGStyle(status = 0U, selected = false, content = available)
        builder.addSVGStyle(status = 0U, selected = true, content = selected)

        builder.addSVGStyle(status = 1U, selected = false, content = disabled)
        builder.addSVGStyle(status = 1U, selected = true, content = disabled)

        builder.addSVGStyle(status = 2U, selected = false, content = disabled)
        builder.addSVGStyle(status = 2U, selected = true, content = disabled)

        builder.addSVGStyle(status = 3U, selected = false, content = disabled)
        builder.addSVGStyle(status = 3U, selected = true, content = disabled)

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