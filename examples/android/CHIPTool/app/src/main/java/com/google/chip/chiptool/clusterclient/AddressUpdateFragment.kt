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
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.address_update_fragment.deviceIdEd
import kotlinx.android.synthetic.main.address_update_fragment.fabricIdEd

/** Fragment for updating the address of a device given its fabric and node ID. */
class AddressUpdateFragment: Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  val deviceId: Long
    get() = deviceIdEd.text.toString().toULong().toLong()

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.address_update_fragment, container, false).apply { }
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    val compressedFabricId = deviceController.compressedFabricId
    fabricIdEd.setText(compressedFabricId.toULong().toString(16).padStart(16, '0'))
    deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }
}