package com.seatcanvas.sample

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
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

        val baseMapName = arguments?.getString(ARG_BASE_MAP_NAME) ?: DEFAULT_BASE_MAP
        val data = readAssetFileToByteArray(requireContext(), baseMapName) ?: return
        binding.seatCanvasView.loadBaseMap(data)

        binding.tiledSwitch.setOnCheckedChangeListener { _, isChecked ->
            binding.seatCanvasView.enableTiled(isChecked)
        }

        binding.zoomBlurSwitch.setOnCheckedChangeListener { _, isChecked ->
            binding.seatCanvasView.enableZoomBlur(isChecked)
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
        const val ARG_BASE_MAP_NAME = "baseMapName"
        private const val DEFAULT_BASE_MAP = "svg/performbg.svg"
    }
}