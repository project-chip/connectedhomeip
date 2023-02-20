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
import android.os.Build
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AlertDialog
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.databinding.SelectActionFragmentBinding
import com.google.chip.chiptool.util.FragmentUtil

/** Fragment to select from various options to interact with a CHIP device. */
class SelectActionFragment : Fragment() {
  private var _binding: SelectActionFragmentBinding? = null
  private val binding get() = _binding!!

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    _binding = SelectActionFragmentBinding.inflate(inflater, container, false)

    binding.scanQrBtn.setOnClickListener { getCallback()?.handleScanQrCodeClicked() }
    binding.provisionWiFiCredentialsBtn.apply {
      isEnabled = hasLocationPermission()
      setOnClickListener { getCallback()?.handleProvisionWiFiCredentialsClicked() }
    }
    binding.provisionThreadCredentialsBtn.apply {
      isEnabled = hasLocationPermission()
      setOnClickListener { getCallback()?.handleProvisionThreadCredentialsClicked() }
    }
    binding.onOffClusterBtn.setOnClickListener { getCallback()?.handleOnOffClicked() }
    binding.sensorClustersBtn.setOnClickListener { getCallback()?.handleSensorClicked() }
    binding.multiAdminClusterBtn.setOnClickListener { getCallback()?.handleMultiAdminClicked() }
    binding.opCredClustersBtn.setOnClickListener { getCallback()?.handleOpCredClicked() }
    binding.basicClusterBtn.setOnClickListener { getCallback()?.handleBasicClicked() }
    binding.attestationTestBtn.setOnClickListener { getCallback()?.handleAttestationTestClicked() }
    binding.clusterInteractionBtn.setOnClickListener { getCallback()?.handleClusterInteractionClicked() }
    binding.provisionCustomFlowBtn.setOnClickListener{  getCallback()?.handleProvisionCustomFlowClicked() }
    binding.wildcardBtn.setOnClickListener { getCallback()?.handleWildcardClicked() }
    binding.unpairDeviceBtn.setOnClickListener{ getCallback()?.handleUnpairDeviceClicked() }

    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    if (savedInstanceState != null) return
    if (hasLocationPermission()) return

    val permissionRequest =
      registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { grantResults
        ->
        var granted = true
        for (value in grantResults.values) {
          if (!value) {
            granted = false
          }
        }
        if (granted) {
          binding.provisionWiFiCredentialsBtn.isEnabled = true
          binding.provisionThreadCredentialsBtn.isEnabled = true
        } else {
          binding.provisionWiFiCredentialsBtn.isEnabled = false
          binding.provisionThreadCredentialsBtn.isEnabled = false

          AlertDialog.Builder(requireContext())
            .setTitle(R.string.location_permission_denied_title)
            .setMessage(R.string.location_permission_denied_message)
            .setPositiveButton(R.string.text_ok) { dialog, _ -> dialog.dismiss() }
            .setCancelable(false)
            .create()
            .show()
        }
      }

    val permissions: Array<String> =
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        arrayOf(
          Manifest.permission.BLUETOOTH_SCAN,
          Manifest.permission.BLUETOOTH_CONNECT,
          Manifest.permission.ACCESS_FINE_LOCATION,
        )
      } else {
        arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
      }
    permissionRequest.launch(permissions)
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun hasLocationPermission(): Boolean {
    val locationPermissionGranted =
      ContextCompat.checkSelfPermission(
        requireContext(),
        Manifest.permission.ACCESS_FINE_LOCATION
      ) == PackageManager.PERMISSION_GRANTED

    // Android 12 new permission
    var bleScanPermissionGranted = true
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
      bleScanPermissionGranted =
        ContextCompat.checkSelfPermission(requireContext(), Manifest.permission.BLUETOOTH_SCAN) ==
          PackageManager.PERMISSION_GRANTED
    }

    return locationPermissionGranted && bleScanPermissionGranted
  }


  private fun getCallback() = FragmentUtil.getHost(this, Callback::class.java)

  /** Interface for notifying the host. */
  interface Callback {
    /** Notifies listener of Scan QR code button click. */
    fun handleScanQrCodeClicked()
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
    /** Notifies listener of provision-WiFi-credentials button click. */
    fun handleProvisionWiFiCredentialsClicked()
    /** Notifies listener of provision-Thread-credentials button click. */
    fun handleProvisionThreadCredentialsClicked()
    /** Notifies listener of provision-custom-flow button click. */
    fun handleProvisionCustomFlowClicked()
    /** Notifies listener of unpair button click. */
    fun handleUnpairDeviceClicked()
  }

  companion object {

    @JvmStatic fun newInstance() = SelectActionFragment()
  }
}
