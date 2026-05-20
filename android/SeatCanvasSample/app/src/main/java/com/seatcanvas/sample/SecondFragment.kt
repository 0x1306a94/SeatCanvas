package com.seatcanvas.sample

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.libseatcanvas.BaseMapFormat
import com.libseatcanvas.SVGBaseMapParseConfig
import com.seatcanvas.sample.databinding.FragmentSecondBinding

class SecondFragment : Fragment() {

    private var _binding: FragmentSecondBinding? = null
    private val binding get() = _binding!!

    private val sampleModel = SeatCanvasSampleModel()

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

        binding.seatCanvasView.canvasColor = ContextCompat.getColor(requireContext(), R.color.canvas_bg)
        binding.seatCanvasView.setDelegate(sampleModel.rendererDelegate)
        binding.seatCanvasView.loadBaseMap(data, BaseMapFormat.SVG, parseConfig)

        sampleModel.loadMockData(requireContext(), baseMapInfo, binding.seatCanvasView)
        sampleModel.regenerateRandomAvailableSeats()
        sampleModel.startAvailableSeatsTimer()
        binding.seatCanvasView.applySeatStyleJSONConfig(
            SeatStyleBuilder.buildSVGSeatStyleConfig(requireContext(), sampleModel.prices)
        )

        setupStyleSwitches()
    }

    override fun onDestroyView() {
        sampleModel.stopAvailableSeatsTimer()
        sampleModel.detachSeatCanvasView()
        binding.seatCanvasView.setDelegate(null)
        binding.seatCanvasView.onDestroy()
        super.onDestroyView()
        _binding = null
    }

    private fun setupStyleSwitches() {
        binding.circleSeatStyleSwitch.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                binding.svgSeatStyleSwitch.isChecked = false
                binding.seatCanvasView.applySeatStyleJSONConfig(
                    SeatStyleBuilder.buildCircleSeatStyleConfig(requireContext(), sampleModel.prices)
                )
            }
        }

        binding.svgSeatStyleSwitch.setOnCheckedChangeListener { _, isChecked ->
            if (isChecked) {
                binding.circleSeatStyleSwitch.isChecked = false
                binding.seatCanvasView.applySeatStyleJSONConfig(
                    SeatStyleBuilder.buildSVGSeatStyleConfig(requireContext(), sampleModel.prices)
                )
            }
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
