/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

package com.google.chip.chiptool.setuppayloadscanner

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.ChipDeviceInfoFragmentBinding
import com.google.chip.chiptool.util.FragmentUtil

/** Show the [CHIPDeviceInfo]. */
class CHIPDeviceDetailsFragment : Fragment() {
  private lateinit var deviceInfo: CHIPDeviceInfo
  private var _binding: ChipDeviceInfoFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ChipDeviceInfoFragmentBinding.inflate(inflater, container, false)
    deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))

    binding.versionTv.text = "${deviceInfo.version}"
    binding.vendorIdTv.text = "${deviceInfo.vendorId}"
    binding.productIdTv.text = "${deviceInfo.productId}"
    binding.setupCodeTv.text = "${deviceInfo.setupPinCode}"
    binding.discriminatorTv.text = "${deviceInfo.discriminator}"
    binding.discoveryCapabilitiesTv.text =
      requireContext()
        .getString(
          R.string.chip_device_info_discovery_capabilities_text,
          deviceInfo.discoveryCapabilities
        )

    if (deviceInfo.optionalQrCodeInfoMap.isEmpty()) {
      binding.vendorTagsLabelTv.visibility = View.GONE
      binding.vendorTagsContainer.visibility = View.GONE
    } else {
      binding.vendorTagsLabelTv.visibility = View.VISIBLE
      binding.vendorTagsContainer.visibility = View.VISIBLE

      deviceInfo.optionalQrCodeInfoMap.forEach { (_, qrCodeInfo) ->
        val tv = inflater.inflate(R.layout.barcode_vendor_tag, null, false) as TextView
        val info = "${qrCodeInfo.tag}. ${qrCodeInfo.data}, ${qrCodeInfo.intDataValue}"
        tv.text = info
        binding.vendorTagsContainer.addView(tv)
      }
    }

    binding.commissioningFlowTv.text = "${deviceInfo.commissioningFlow}"

    // commissioningFlow = 2 (Custom), read device info from Ledger
    if (deviceInfo.commissioningFlow == 2) {
      binding.customFlowBtn.visibility = View.VISIBLE
      binding.customFlowBtn.setOnClickListener {
        FragmentUtil.getHost(this@CHIPDeviceDetailsFragment, Callback::class.java)
          ?.handleReadFromLedgerClicked(deviceInfo)
      }
    }

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  /** Interface for notifying the host. */
  interface Callback {
    /** Notifies listener of Read Device Info from Ledger button click. */
    fun handleReadFromLedgerClicked(deviceInfo: CHIPDeviceInfo)
  }

  companion object {
    private const val ARG_DEVICE_INFO = "device_info"

    @JvmStatic
    fun newInstance(deviceInfo: CHIPDeviceInfo): CHIPDeviceDetailsFragment {
      return CHIPDeviceDetailsFragment().apply {
        arguments = Bundle(1).apply { putParcelable(ARG_DEVICE_INFO, deviceInfo) }
      }
    }
  }
}
