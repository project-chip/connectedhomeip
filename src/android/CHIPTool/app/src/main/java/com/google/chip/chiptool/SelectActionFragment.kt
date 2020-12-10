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
package com.google.chip.chiptool

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.android.synthetic.main.select_action_fragment.view.*

/** Fragment to select from various options to interact with a CHIP device. */
class SelectActionFragment : Fragment() {

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.select_action_fragment, container, false).apply {
      scanQrBtn.setOnClickListener { getCallback()?.handleScanQrCodeClicked() }
      provisionWifiCredentialsBtn.setOnClickListener {
        getCallback()?.onProvisionWifiCredentialsClicked()
      }
      provisionThreadCredentialsBtn.setOnClickListener {
        getCallback()?.onProvisionThreadCredentialsClicked()
      }
      otCommissioningBtn.setOnClickListener { getCallback()?.handleCommissioningClicked() }
      echoClientBtn.setOnClickListener { getCallback()?.handleEchoClientClicked() }
      onOffClusterBtn.setOnClickListener { getCallback()?.handleOnOffClicked() }
      attestationTestBtn.setOnClickListener { getCallback()?.handleAttestationTestClicked() }
    }
  }

  private fun getCallback() = FragmentUtil.getHost(this, Callback::class.java)

  /** Interface for notifying the host. */
  interface Callback {
    /** Notifies listener of Scan QR code button click. */
    fun handleScanQrCodeClicked()
    /** Notifies listener of provision-Wifi-credentials button click. */
    fun onProvisionWifiCredentialsClicked()
    /** Notifies listener of provision-Thread-credentials button click. */
    fun onProvisionThreadCredentialsClicked()
    /** Notifies listener of Commissioning button click. */
    fun handleCommissioningClicked()
    /** Notifies listener of Echo client button click. */
    fun handleEchoClientClicked()
    /** Notifies listener of send command button click. */
    fun handleOnOffClicked()
    /** Notifies listener of attestation command button clicked. */
    fun handleAttestationTestClicked()
  }

  companion object {

    @JvmStatic fun newInstance() = SelectActionFragment()
  }
}
