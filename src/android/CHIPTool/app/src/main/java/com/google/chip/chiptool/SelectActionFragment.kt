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
import kotlinx.android.synthetic.main.select_action_fragment.provisionWiFiCredentialsBtn
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
      provisionWiFiCredentialsBtn.apply {
        isEnabled = hasLocationPermission()
        setOnClickListener { getCallback()?.onProvisionWiFiCredentialsClicked() }
      }
      provisionThreadCredentialsBtn.apply {
        isEnabled = hasLocationPermission()
        setOnClickListener { getCallback()?.onProvisionThreadCredentialsClicked() }
      }
      onOffClusterBtn.setOnClickListener { getCallback()?.handleOnOffClicked() }
      sensorClustersBtn.setOnClickListener { getCallback()?.handleSensorClicked() }
      multiAdminClusterBtn.setOnClickListener { getCallback()?.handleMultiAdminClicked() }
      opCredClustersBtn.setOnClickListener { getCallback()?.handleOpCredClicked() }
      basicClusterBtn.setOnClickListener { getCallback()?.handleBasicClicked() }
      attestationTestBtn.setOnClickListener { getCallback()?.handleAttestationTestClicked() }
      clusterInteractionBtn.setOnClickListener { getCallback()?.handleClusterInteractionClicked() }
      provisionCustomFlowBtn.setOnClickListener{  getCallback()?.handleCustomFlowClicked() }
      wildcardBtn.setOnClickListener { getCallback()?.handleWildcardClicked() }
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
        provisionWiFiCredentialsBtn.isEnabled = true
        provisionThreadCredentialsBtn.isEnabled = true
      } else {
        provisionWiFiCredentialsBtn.isEnabled = false
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
    /** Notifies listener of provision-WiFi-credentials button click. */
    fun onProvisionWiFiCredentialsClicked()
    /** Notifies listener of provision-Thread-credentials button click. */
    fun onProvisionThreadCredentialsClicked()
    /** Notifies listener of Light On/Off & Level Cluster button click. */
    fun handleOnOffClicked()
    /** Notifies listener of Sensor Clusters button click. */
    fun handleSensorClicked()
    /** Notifies listener of Multi-admin Clusters button click. */
    fun handleMultiAdminClicked()
    /** Notifies listener of Operational Credentials Clusters button click. */
    fun handleOpCredClicked()
    /** Notifies listener of Basic Clusters button click. */
    fun handleBasicClicked()
    /** Notifies listener of attestation command button clicked. */
    fun handleAttestationTestClicked()
    /** Notifies listener of a click to manually input the CHIP device address. */
    fun onShowDeviceAddressInput()
    /** Notifies listener of cluster interaction button click. */
    fun handleClusterInteractionClicked()
    /** Notifies listener of wildcard button click. */
    fun handleWildcardClicked()
    /** Notifies listener of provision-custom-flow button click. */
    fun handleCustomFlowClicked()
  }

  companion object {

    @JvmStatic fun newInstance() = SelectActionFragment()
  }
}
