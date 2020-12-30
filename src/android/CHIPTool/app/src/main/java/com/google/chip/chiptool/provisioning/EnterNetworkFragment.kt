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

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.enter_thread_network_fragment.channelEd
import kotlinx.android.synthetic.main.enter_thread_network_fragment.masterKeyEd
import kotlinx.android.synthetic.main.enter_thread_network_fragment.panIdEd
import kotlinx.android.synthetic.main.enter_thread_network_fragment.xpanIdEd
import kotlinx.android.synthetic.main.enter_wifi_network_fragment.*
import kotlinx.android.synthetic.main.enter_wifi_network_fragment.view.*

/**
 * Fragment to collect Wi-Fi network information from user and send it to device being provisioned.
 */
class EnterNetworkFragment : Fragment() {

  private val networkType: ProvisionNetworkType
    get() = requireNotNull(
        ProvisionNetworkType.fromName(arguments?.getString(ARG_PROVISION_NETWORK_TYPE))
    )

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View? {
    val layoutRes = when (networkType) {
      ProvisionNetworkType.WIFI -> R.layout.enter_wifi_network_fragment
      ProvisionNetworkType.THREAD -> R.layout.enter_thread_network_fragment
    }

    return inflater.inflate(layoutRes, container, false).apply {
      saveNetworkBtn.setOnClickListener { onSaveNetworkClicked() }
    }
  }

  private fun onSaveNetworkClicked() {
    if (networkType == ProvisionNetworkType.WIFI) {
      saveWifiNetwork()
    } else {
      saveThreadNetwork()
    }
  }

  private fun saveWifiNetwork() {
    val ssid = ssidEd.text
    val pwd = pwdEd.text

    if (ssid.isNullOrBlank() || pwd.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Ssid and password required.", Toast.LENGTH_SHORT).show()
      return
    }

    ChipClient.getDeviceController().apply {
      sendWiFiCredentials(ssid.toString(), pwd.toString())
    }
  }

  private fun saveThreadNetwork() {
    val channelStr = channelEd.text
    val panIdStr = panIdEd.text

    if (channelStr.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Channel is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (panIdStr.isNullOrBlank()) {
      Toast.makeText(requireContext(), "PAN ID is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (xpanIdEd.text.isNullOrBlank()) {
      Toast.makeText(requireContext(), "XPAN ID is empty", Toast.LENGTH_SHORT).show()
      return
    }

    val xpanIdStr = xpanIdEd.text.toString().filterNot { c -> c == ':' }
    if (xpanIdStr.length != NUM_XPANID_BYTES * 2) {
      Toast.makeText(requireContext(), "Extended PAN ID is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    if (masterKeyEd.text.isNullOrBlank()) {
      Toast.makeText(requireContext(), "Master Key is empty", Toast.LENGTH_SHORT).show()
      return
    }

    val masterKeyStr = masterKeyEd.text.toString().filterNot { c -> c == ':' }
    if (masterKeyStr.length != NUM_MASTER_KEY_BYTES * 2) {
      Toast.makeText(requireContext(), "Master key is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    ChipClient.getDeviceController().sendThreadCredentials(
        channelStr.toString().toInt(),
        panIdStr.toString().toInt(16),
        xpanIdStr.hexToByteArray(),
        masterKeyStr.hexToByteArray())
  }

  private fun String.hexToByteArray(): ByteArray {
    return chunked(2).map{ byteStr -> byteStr.toUByte(16).toByte()}.toByteArray()
  }

  companion object {
    private const val TAG = "EnterNetworkFragment"
    private const val ARG_PROVISION_NETWORK_TYPE = "provision_network_type"
    private const val NUM_XPANID_BYTES = 8
    private const val NUM_MASTER_KEY_BYTES = 16

    fun newInstance(provisionNetworkType: ProvisionNetworkType): EnterNetworkFragment {
      return EnterNetworkFragment().apply {
        arguments = Bundle(1).apply {
          putString(ARG_PROVISION_NETWORK_TYPE, provisionNetworkType.name)
        }
      }
    }
  }

}
