package com.seatcanvas.sample

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import androidx.core.os.bundleOf
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import com.seatcanvas.sample.databinding.FragmentFirstBinding

/**
 * A simple [Fragment] subclass as the default destination in the navigation.
 */
class FirstFragment : Fragment() {

    private var _binding: FragmentFirstBinding? = null

    private val binding get() = _binding!!
    private val baseMapSections = mutableListOf<List<BaseMapFileInfo>>()

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {

        _binding = FragmentFirstBinding.inflate(inflater, container, false)
        return binding.root

    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        baseMapSections.clear()
        baseMapSections.add(requireContext().defaultBaseMaps())
        baseMapSections.add(requireContext().customizedBaseMaps())

        val allBaseMaps = baseMapSections.flatten()
        val displayNames = allBaseMaps.map { it.filename }

        val adapter = ArrayAdapter(requireContext(), android.R.layout.simple_list_item_1, displayNames)
        binding.listBaseMaps.adapter = adapter
        binding.listBaseMaps.setOnItemClickListener { _, _, position, _ ->
            val baseMap = allBaseMaps[position]
            val bundle = bundleOf(SecondFragment.ARG_BASE_MAP_INFO to baseMap)
            findNavController().navigate(R.id.action_FirstFragment_to_SecondFragment, bundle)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}