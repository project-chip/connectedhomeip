/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package com.google.chip.chiptool.setuppayloadscanner

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.discoveryCapabilitiesTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.discriminatorTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.productIdTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.setupCodeTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.vendorIdTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.vendorTagsContainer
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.vendorTagsLabelTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.versionTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.commissioningFlowTv
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.customFlowBtn

/** Show the [CHIPDeviceInfo]. */
class CHIPDeviceDetailsFragment : Fragment() {

  private lateinit var deviceInfo: CHIPDeviceInfo

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))

    return inflater.inflate(R.layout.chip_device_info_fragment, container, false).apply {

      versionTv.text = "${deviceInfo.version}"
      vendorIdTv.text = "${deviceInfo.vendorId}"
      productIdTv.text = "${deviceInfo.productId}"
      setupCodeTv.text = "${deviceInfo.setupPinCode}"
      discriminatorTv.text = "${deviceInfo.discriminator}"
      discoveryCapabilitiesTv.text = requireContext().getString(
        R.string.chip_device_info_discovery_capabilities_text,
        deviceInfo.discoveryCapabilities
      )

      if (deviceInfo.optionalQrCodeInfoMap.isEmpty()) {
        vendorTagsLabelTv.visibility = View.GONE
        vendorTagsContainer.visibility = View.GONE
      } else {
        vendorTagsLabelTv.visibility = View.VISIBLE
        vendorTagsContainer.visibility = View.VISIBLE

        deviceInfo.optionalQrCodeInfoMap.forEach { (_, qrCodeInfo) ->
          val tv = inflater.inflate(R.layout.barcode_vendor_tag, null, false) as TextView
          val info = "${qrCodeInfo.tag}. ${qrCodeInfo.data}, ${qrCodeInfo.intDataValue}"
          tv.text = info
          vendorTagsContainer.addView(tv)
        }
      }

      commissioningFlowTv.text = "${deviceInfo.commissioningFlow}"

      // commissioningFlow = 2 (Custom), read device info from Ledger
      if (deviceInfo.commissioningFlow == 2) {
        customFlowBtn.visibility = View.VISIBLE
        customFlowBtn.setOnClickListener {
          FragmentUtil.getHost(this@CHIPDeviceDetailsFragment, Callback::class.java)
            ?.handleReadFromLedgerClicked(deviceInfo)
        }
      }
    }
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
