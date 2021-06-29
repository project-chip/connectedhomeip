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

package com.google.chip.chiptool.provisioning

import android.bluetooth.BluetoothGatt
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.bluetooth.BluetoothManager
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.DeviceIdUtil
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

@ExperimentalCoroutinesApi
class DeviceProvisioningFragment : Fragment() {

  private lateinit var deviceInfo: CHIPDeviceInfo

  private var gatt: BluetoothGatt? = null

  private val networkType: ProvisionNetworkType
    get() = requireNotNull(
      ProvisionNetworkType.fromName(arguments?.getString(ARG_PROVISION_NETWORK_TYPE))
    )

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    deviceInfo = checkNotNull(requireArguments().getParcelable(ARG_DEVICE_INFO))
    return inflater.inflate(R.layout.single_fragment_container, container, false).apply {
      if (savedInstanceState == null) {
        startConnectingToDevice()
      }
    }
  }

  override fun onStop() {
    super.onStop()
    gatt = null
    scope.cancel()
  }

  private fun startConnectingToDevice() {
    if (gatt != null) {
      return
    }

    scope.launch {
      val deviceController = ChipClient.getDeviceController()
      val bluetoothManager = BluetoothManager()

      showMessage(
        R.string.rendezvous_over_ble_scanning_text,
        deviceInfo.discriminator.toString()
      )
      val device = bluetoothManager.getBluetoothDevice(deviceInfo.discriminator) ?: run {
        showMessage(R.string.rendezvous_over_ble_scanning_failed_text)
        return@launch
      }

      showMessage(
        R.string.rendezvous_over_ble_connecting_text,
        device.name ?: device.address.toString()
      )
      gatt = bluetoothManager.connect(requireContext(), device)

      showMessage(R.string.rendezvous_over_ble_pairing_text)
      deviceController.setCompletionListener(ConnectionCallback())

      val deviceId = DeviceIdUtil.getNextAvailableId(requireContext())
      deviceController.pairDevice(gatt, deviceId, deviceInfo.setupPinCode)
      DeviceIdUtil.setNextAvailableId(requireContext(), deviceId + 1)
    }
  }

  private fun showMessage(msgResId: Int, stringArgs: String? = null) {
    requireActivity().runOnUiThread {
      val context = requireContext()
      Toast.makeText(context, context.getString(msgResId, stringArgs), Toast.LENGTH_SHORT)
        .show()
    }
  }

  inner class ConnectionCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {
      Log.d(TAG, "onConnectDeviceComplete")
    }

    override fun onStatusUpdate(status: Int) {
      Log.d(TAG, "Pairing status update: $status")
    }

    override fun onPairingComplete(code: Int) {
      Log.d(TAG, "onPairingComplete: $code")

      if (code == STATUS_PAIRING_SUCCESS) {
        childFragmentManager.beginTransaction()
            .add(R.id.fragment_container, EnterNetworkFragment.newInstance(networkType))
            .commit()
      } else {
        showMessage(R.string.rendezvous_over_ble_pairing_failure_text)
      }
    }

    override fun onOpCSRGenerationComplete(errorCode: ByteArray) {
      Log.d(TAG,String(errorCode))
    }

    override fun onPairingDeleted(code: Int) {
      Log.d(TAG, "onPairingDeleted: $code")
    }

    override fun onNetworkCommissioningComplete(code: Int) {
      Log.d(TAG, "onNetworkCommissioningComplete: $code")

      if (code == 0) {
        showMessage(R.string.rendezvous_over_ble_commissioning_success_text)
      } else {
        showMessage(R.string.rendezvous_over_ble_commissioning_failure_text)
      }

      FragmentUtil.getHost(this@DeviceProvisioningFragment, Callback::class.java)
          ?.onCommissioningComplete(code)
    }

    override fun onCloseBleComplete() {
      Log.d(TAG, "onCloseBleComplete")
    }

    override fun onError(error: Throwable?) {
      Log.d(TAG, "onError: $error")
    }
  }

  /** Callback from [DeviceProvisioningFragment] notifying any registered listeners. */
  interface Callback {
    /** Notifies that commissioning has been completed. */
    fun onCommissioningComplete(code: Int)
  }

  companion object {
    private const val TAG = "DeviceProvisioningFragment"
    private const val ARG_DEVICE_INFO = "device_info"
    private const val ARG_PROVISION_NETWORK_TYPE = "provision_network_type"
    private const val STATUS_PAIRING_SUCCESS = 0

    fun newInstance(
      deviceInfo: CHIPDeviceInfo,
      networkType: ProvisionNetworkType
    ): DeviceProvisioningFragment {
      return DeviceProvisioningFragment().apply {
        arguments = Bundle(2).apply {
          putParcelable(ARG_DEVICE_INFO, deviceInfo)
          putString(ARG_PROVISION_NETWORK_TYPE, networkType.name)
        }
      }
    }
  }
}
