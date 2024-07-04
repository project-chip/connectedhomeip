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

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.ChipDeviceInfoFragmentBinding
import com.google.chip.chiptool.util.FragmentUtil
import java.net.URLEncoder
import matter.onboardingpayload.OnboardingPayload
import matter.onboardingpayload.OnboardingPayloadException
import matter.onboardingpayload.QRCodeOnboardingPayloadGenerator

/** Show the [CHIPDeviceInfo]. */
class CHIPDeviceDetailsFragment : Fragment() {
  private lateinit var deviceInfo: CHIPDeviceInfo
  private var _binding: ChipDeviceInfoFragmentBinding? = null
  private val binding
    get() = _binding!!

  private lateinit var onBoardingPayload: OnboardingPayload

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ChipDeviceInfoFragmentBinding.inflate(inflater, container, false)
    deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))

    binding.versionEd.setText(deviceInfo.version.toString())
    binding.vendorIdEd.setText(deviceInfo.vendorId.toString())
    binding.productIdEd.setText(deviceInfo.productId.toString())
    binding.setupCodeEd.setText(deviceInfo.setupPinCode.toString())
    binding.discriminatorEd.setText(deviceInfo.discriminator.toString())
    binding.serialNumberEd.setText(deviceInfo.serialNumber)
    binding.discoveryCapabilitiesTv.text = "${deviceInfo.discoveryCapabilities}"
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

    binding.commissioningFlowEd.setText(deviceInfo.commissioningFlow.toString())

    // commissioningFlow = 2 (Custom), read device info from Ledger
    if (deviceInfo.commissioningFlow == 2) {
      binding.customFlowBtn.visibility = View.VISIBLE
      binding.customFlowBtn.setOnClickListener {
        FragmentUtil.getHost(this@CHIPDeviceDetailsFragment, Callback::class.java)
          ?.handleReadFromLedgerClicked(deviceInfo)
      }
    }

    onBoardingPayload = deviceInfo.toSetupPayload()
    binding.qrCodeTv.text =
      QRCodeOnboardingPayloadGenerator(onBoardingPayload)
        .payloadBase38RepresentationWithAutoTLVBuffer()

    setDiscoveryCapabilitiesEditTextListener()
    setQRCodeButtonListener()

    return binding.root
  }

  private fun setDiscoveryCapabilitiesEditTextListener() {
    binding.discoveryCapabilitiesEd.setText(onBoardingPayload.getRendezvousInformation().toString())
    binding.discoveryCapabilitiesEd.addTextChangedListener(
      object : TextWatcher {
        override fun onTextChanged(value: CharSequence?, p1: Int, p2: Int, p3: Int) {
          if (value.isNullOrEmpty()) {
            return
          }
          onBoardingPayload.setRendezvousInformation(value.toString().toLong())
          binding.discoveryCapabilitiesTv.text = "${onBoardingPayload.discoveryCapabilities}"
        }

        override fun afterTextChanged(p0: Editable?) {
          // no_op
        }

        override fun beforeTextChanged(p0: CharSequence?, p1: Int, p2: Int, p3: Int) {
          // no_op
        }
      }
    )
  }

  private fun setQRCodeButtonListener() {
    binding.showQRCodeBtn.setOnClickListener {
      onBoardingPayload.apply {
        version = binding.versionEd.text.toString().toInt()
        vendorId = binding.vendorIdEd.text.toString().toInt()
        productId = binding.productIdEd.text.toString().toInt()
        commissioningFlow = binding.commissioningFlowEd.text.toString().toInt()
        setupPinCode = binding.setupCodeEd.text.toString().toLong()
        discriminator = binding.discriminatorEd.text.toString().toInt()
        val serialNumber = binding.serialNumberEd.text.toString()
        try {
          removeSerialNumber()
        } catch (e: OnboardingPayloadException) {
          Log.d(TAG, "Serial Number not set!", e)
        }
        if (serialNumber.isNotEmpty()) {
          addSerialNumber(binding.serialNumberEd.text.toString())
        }
      }
      val qrCode =
        QRCodeOnboardingPayloadGenerator(onBoardingPayload)
          .payloadBase38RepresentationWithAutoTLVBuffer()
      Log.d(TAG, "QR Code : $qrCode")
      binding.qrCodeTv.text = qrCode
    }

    binding.showQRCodeUriBtn.setOnClickListener {
      val qrCodeEncode = URLEncoder.encode(binding.qrCodeTv.text.toString(), "UTF-8")
      val qrCodeUri = getString(R.string.chip_device_info_qrcode_uri, qrCodeEncode)
      val intent = Intent(Intent.ACTION_VIEW, Uri.parse(qrCodeUri))
      startActivity(intent)
    }
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
    private const val TAG = "CHIPDeviceDetailsFragment"
    private const val ARG_DEVICE_INFO = "device_info"

    @JvmStatic
    fun newInstance(deviceInfo: CHIPDeviceInfo): CHIPDeviceDetailsFragment {
      return CHIPDeviceDetailsFragment().apply {
        arguments = Bundle(1).apply { putParcelable(ARG_DEVICE_INFO, deviceInfo) }
      }
    }
  }
}
