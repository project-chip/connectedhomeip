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

import android.bluetooth.BluetoothGatt
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import android.widget.Toast
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.bluetooth.BluetoothManager
import kotlinx.android.synthetic.main.chip_device_info_fragment.view.*
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

/** Show the [CHIPDeviceInfo]. */
class CHIPDeviceDetailsFragment : Fragment(), ChipDeviceController.CompletionListener, ChipDeviceController.PairingListener {

    private lateinit var deviceInfo: CHIPDeviceInfo
    private var gatt: BluetoothGatt? = null
    private val scope = CoroutineScope(Dispatchers.Main + Job())

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

    override fun onStop() {
        super.onStop()
        gatt = null
        scope.cancel()
    }

    private fun onRendezvousBleClicked() {
        if (gatt == null) {
            scope.launch {
                val deviceController = ChipClient.getDeviceController()
                val bluetoothManager = BluetoothManager()

                showMessage(requireContext().getString(R.string.rendezvous_over_ble_scanning_text) + " " + deviceInfo.discriminator.toString())
                val device = bluetoothManager.getBluetoothDevice(deviceInfo.discriminator) ?: run {
                    showMessage(requireContext().getString(R.string.rendezvous_over_ble_scanning_failed_text))
                    return@launch
                }

                showMessage(requireContext().getString(R.string.rendezvous_over_ble_connecting_text) + " " + (device.name ?: device.address.toString()))
                gatt = bluetoothManager.connect(requireContext(), device)

                showMessage(requireContext().getString(R.string.rendezvous_over_ble_pairing_text))
                deviceController.setPairingListener(this@CHIPDeviceDetailsFragment)
                deviceController.setCompletionListener(this@CHIPDeviceDetailsFragment)
                deviceController.beginConnectDeviceBle(gatt, deviceInfo.setupPinCode);
            }
        }
    }

    override fun onConnectDeviceComplete() {
        showMessage(requireContext().getString(R.string.rendezvous_over_ble_success_text))
    }

    override fun onCloseBleComplete() {
        Log.d(TAG, "onCloseBleComplete")
    }

    override fun onNotifyChipConnectionClosed() {
        Log.d(TAG, "onNotifyChipConnectionClosed")
    }

    override fun onSendMessageComplete(message: String?) {
        Log.d(TAG, "Message received: $message")
    }

    override fun onError(error: Throwable?) {
        Log.d(TAG, "onError: $error")
    }

    private fun onRendezvousSoftApClicked() {
        // TODO: once rendezvous over hotspot is ready in CHIP
    }

    private fun showMessage(msg: String) {
        requireActivity().runOnUiThread {
            Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
        }
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

    override fun onNetworkCredentialsRequested() {
        Log.e(TAG, "onNetworkCredentialsRequested is not yet implemented");
    }

    override fun onOperationalCredentialsRequested(p0: ByteArray?) {
        Log.e(TAG, "Operational credentials request not yet implemented");
    }

    override fun onStatusUpdate(status: Int) {
        Log.i(TAG, "Pairing status update: $status");
    }

    override fun onPairingComplete(code: Int) {
        Log.i(TAG, "Pairing complete: $code");
    }

    override fun onPairingDeleted(code: Int) {
        Log.i(TAG, "Pairing deleted: $code");
    }



}
