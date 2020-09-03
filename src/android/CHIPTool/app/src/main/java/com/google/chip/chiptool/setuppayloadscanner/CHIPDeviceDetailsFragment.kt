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
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.*

/** Show the [CHIPDeviceInfo]. */
class CHIPDeviceDetailsFragment : Fragment(), ChipDeviceController.CompletionListener {

    private lateinit var deviceInfo: CHIPDeviceInfo

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))

        return inflater.inflate(R.layout.chip_device_info_fragment, container, false).apply {

            // Display CHIP setup code info to user for manual connect to soft AP
            versionTv.text = "${deviceInfo.version}"
            vendorIdTv.text = "${deviceInfo.vendorId}"
            productIdTv.text = "${deviceInfo.productId}"
            setupCodeTv.text = "${deviceInfo.setupPinCode}"
            discriminatorTv.text = "${deviceInfo.discriminator}"

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

            ble_rendezvous_btn.setOnClickListener { onRendezvousBleClicked() }
            softap_rendezvous_btn.setOnClickListener { onRendezvousSoftApClicked() }
        }
    }

    private fun onRendezvousBleClicked() { }

    override fun onConnectDeviceComplete() {
        Log.d(TAG, "TODO: Retrieve Wi-Fi credentials and send to device.")
    }

    override fun onCloseBleComplete() {
        Log.d(TAG, "onCloseBleComplete")
    }

    override fun onNotifyChipConnectionClosed() {
        Log.d(TAG, "onNotifyChipConnectionClosed")
    }

    override fun onSendMessageComplete(message: String?) {
        Log.d(TAG, "Echo message received: $message")
    }

    override fun onError(error: Throwable?) {
        Log.d(TAG, "onError: $error")
    }

    private fun onRendezvousSoftApClicked() {
        // TODO: once rendezvous over hotspot is ready in CHIP
    }

    companion object {
        private const val TAG = "CHIPDeviceDetailsFragment"
        private const val ARG_DEVICE_INFO = "device_info"

        @JvmStatic fun newInstance(deviceInfo: CHIPDeviceInfo): CHIPDeviceDetailsFragment {
            return CHIPDeviceDetailsFragment().apply {
                arguments = Bundle(1).apply { putParcelable(ARG_DEVICE_INFO, deviceInfo) }
            }
        }
    }
}
