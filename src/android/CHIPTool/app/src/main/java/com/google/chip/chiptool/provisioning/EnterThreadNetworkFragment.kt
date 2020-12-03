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
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.enter_wifi_network_fragment.view.*
import kotlinx.android.synthetic.main.enter_thread_network_fragment.*

class EnterThreadNetworkFragment : Fragment() {

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?): View {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.enter_thread_network_fragment, container, false).apply {
      saveNetworkBtn.setOnClickListener { onSaveNetworkClicked() }
    }
  }

  private fun onSaveNetworkClicked() {
    val channelStr = channelEd.text.toString()
    val panIdStr = panidEd.text.toString()
    val xpanIdStr = xpanidEd.text.toString().filterNot { c -> c == ':' }
    val masterKeyStr = masterKeyEd.text.toString().filterNot { c -> c == ':' }

    if (channelStr.isEmpty()) {
      Toast.makeText(requireContext(), "Channel is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (panIdStr.isEmpty()) {
      Toast.makeText(requireContext(), "PAN ID is empty", Toast.LENGTH_SHORT).show()
      return
    }

    if (xpanIdStr.length != NUM_XPANID_BYTES * 2) {
      Toast.makeText(requireContext(), "Extended PAN ID is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    if (masterKeyStr.length != NUM_MASTER_KEY_BYTES * 2) {
      Toast.makeText(requireContext(), "Master key is invalid", Toast.LENGTH_SHORT).show()
      return
    }

    ChipClient.getDeviceController().sendThreadCredentials(
        channelStr.toInt(),
        panIdStr.toInt(16),
        xpanIdStr.hexToByteArray(),
        masterKeyStr.hexToByteArray())
  }

  companion object {
    private const val NUM_XPANID_BYTES = 8
    private const val NUM_MASTER_KEY_BYTES = 16

    private fun String.hexToByteArray(): ByteArray {
      return chunked(2).map{ byteStr -> byteStr.toUByte(16).toByte()}.toByteArray()
    }

    fun newInstance() = EnterThreadNetworkFragment()
  }
}