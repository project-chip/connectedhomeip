package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipIdLookup
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.AddressUpdateFragmentBinding
import com.google.chip.chiptool.util.DeviceIdUtil

/** Fragment for updating the address of a device given its fabric and node ID. */
class AddressUpdateFragment: Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  val deviceId: Long
    get() = binding.deviceIdEd.text.toString().toULong().toLong()

  private var _binding: AddressUpdateFragmentBinding? = null
  private val binding get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = AddressUpdateFragmentBinding.inflate(inflater, container, false)
    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    val compressedFabricId = deviceController.compressedFabricId
    binding.fabricIdEd.setText(compressedFabricId.toULong().toString(16).padStart(16, '0'))
    binding.deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }
}