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

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.util.FragmentUtil
import kotlinx.android.synthetic.main.select_action_fragment.provisionThreadCredentialsBtn
import kotlinx.android.synthetic.main.select_action_fragment.provisionWifiCredentialsBtn
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
      provisionWifiCredentialsBtn.apply {
        isEnabled = hasLocationPermission()
        setOnClickListener { getCallback()?.onProvisionWifiCredentialsClicked() }
      }
      provisionThreadCredentialsBtn.apply {
        isEnabled = hasLocationPermission()
        setOnClickListener { getCallback()?.onProvisionThreadCredentialsClicked() }
      }
      echoClientBtn.setOnClickListener { getCallback()?.handleEchoClientClicked() }
      onOffClusterBtn.setOnClickListener { getCallback()?.handleOnOffClicked() }
      sensorClustersBtn.setOnClickListener{ getCallback()?.handleSensorClicked() }
      attestationTestBtn.setOnClickListener { getCallback()?.handleAttestationTestClicked() }
    }
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    if (savedInstanceState != null) return
    if (hasLocationPermission()) return

    val permissionRequest = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { granted ->
      if (granted) {
        provisionWifiCredentialsBtn.isEnabled = true
        provisionThreadCredentialsBtn.isEnabled = true
      } else {
        provisionWifiCredentialsBtn.isEnabled = false
        provisionThreadCredentialsBtn.isEnabled = false

        AlertDialog.Builder(requireContext())
            .setTitle(R.string.location_permission_denied_title)
            .setMessage(R.string.location_permission_denied_message)
            .setPositiveButton(R.string.text_ok) { dialog, _ -> dialog.dismiss() }
            .setCancelable(false)
            .create()
            .show()
      }
    }

    permissionRequest.launch(Manifest.permission.ACCESS_FINE_LOCATION)
  }

  private fun hasLocationPermission(): Boolean {
    val locationPermissionState =
        ContextCompat.checkSelfPermission(
            requireContext(),
            Manifest.permission.ACCESS_FINE_LOCATION
        )

    return PackageManager.PERMISSION_GRANTED == locationPermissionState
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
    /** Notifies listener of Echo client button click. */
    fun handleEchoClientClicked()
    /** Notifies listener of Light On/Off & Level Cluster button click. */
    fun handleOnOffClicked()
    /** Notifies listener of Sensor Clusters button click. */
    fun handleSensorClicked()
    /** Notifies listener of attestation command button clicked. */
    fun handleAttestationTestClicked()
    /** Notifies listener of a click to manually input the CHIP device address.. */
    fun onShowDeviceAddressInput()
  }

  companion object {

    @JvmStatic fun newInstance() = SelectActionFragment()
  }
}
