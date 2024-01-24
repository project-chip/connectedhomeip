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
import com.google.chip.chiptool.attestation.AttestationTestFragment
import com.google.chip.chiptool.clusterclient.*
import com.google.chip.chiptool.clusterclient.GroupSettingFragment
import com.google.chip.chiptool.clusterclient.clusterinteraction.ClusterInteractionFragment
import com.google.chip.chiptool.databinding.SelectActionFragmentBinding
import com.google.chip.chiptool.provisioning.ProvisionNetworkType
import com.google.chip.chiptool.provisioning.UnpairDeviceFragment
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.util.FragmentUtil

/** Fragment to select from various options to interact with a CHIP device. */
class SelectActionFragment : Fragment() {
  private var _binding: SelectActionFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = SelectActionFragmentBinding.inflate(inflater, container, false)

    binding.provisionWiFiCredentialsBtn.apply {
      isEnabled = hasLocationPermission()
      setOnClickListener { handleProvisionWiFiCredentialsClicked() }
    }

    binding.provisionThreadCredentialsBtn.apply {
      isEnabled = hasLocationPermission()
      setOnClickListener { handleProvisionThreadCredentialsClicked() }
    }

    binding.scanQrBtn.setOnClickListener { handleScanQrCodeClicked() }
    binding.onOffClusterBtn.setOnClickListener { handleOnOffClicked() }
    binding.sensorClustersBtn.setOnClickListener { handleSensorClicked() }
    binding.multiAdminClusterBtn.setOnClickListener { handleMultiAdminClicked() }
    binding.opCredClustersBtn.setOnClickListener { handleOpCredClicked() }
    binding.basicClusterBtn.setOnClickListener { handleBasicClicked() }
    binding.attestationTestBtn.setOnClickListener { handleAttestationTestClicked() }
    binding.clusterInteractionBtn.setOnClickListener { handleClusterInteractionClicked() }
    binding.provisionCustomFlowBtn.setOnClickListener { handleProvisionCustomFlowClicked() }
    binding.wildcardBtn.setOnClickListener { handleWildcardClicked() }
    binding.unpairDeviceBtn.setOnClickListener { handleUnpairDeviceClicked() }
    binding.groupSettingBtn.setOnClickListener { handleGroupSettingClicked() }
    binding.otaProviderBtn.setOnClickListener { handleOTAProviderClicked() }
    binding.icdBtn.setOnClickListener { handleICDClicked() }

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
    /** Notifies listener of a click to manually input the CHIP device address. */
    fun onShowDeviceAddressInput()

    fun SetNetworkType(type: ProvisionNetworkType)
  }

  private fun showFragment(fragment: Fragment, showOnBack: Boolean = true) {
    val fragmentTransaction =
      parentFragmentManager
        .beginTransaction()
        .replace(R.id.nav_host_fragment, fragment, fragment.javaClass.simpleName)

    if (showOnBack) {
      fragmentTransaction.addToBackStack(null)
    }

    fragmentTransaction.commit()
  }

  /** Notifies listener of Scan QR code button click. */
  private fun handleScanQrCodeClicked() {
    showFragment(BarcodeFragment.newInstance())
  }

  /** Notifies listener of Light On/Off & Level Cluster button click. */
  private fun handleOnOffClicked() {
    showFragment(OnOffClientFragment.newInstance())
  }

  /** Notifies listener of Sensor Clusters button click. */
  private fun handleSensorClicked() {
    showFragment(SensorClientFragment.newInstance())
  }

  /** Notifies listener of Multi-admin Clusters button click. */
  private fun handleMultiAdminClicked() {
    showFragment(MultiAdminClientFragment.newInstance())
  }

  /** Notifies listener of Operational Credentials Clusters button click. */
  private fun handleOpCredClicked() {
    showFragment(OpCredClientFragment.newInstance())
  }

  /** Notifies listener of Basic Clusters button click. */
  private fun handleBasicClicked() {
    showFragment(BasicClientFragment.newInstance())
  }

  /** Notifies listener of attestation command button clicked. */
  private fun handleAttestationTestClicked() {
    showFragment(AttestationTestFragment.newInstance())
  }

  /** Notifies listener of cluster interaction button click. */
  private fun handleClusterInteractionClicked() {
    showFragment(ClusterInteractionFragment.newInstance())
  }

  /** Notifies listener of wildcard button click. */
  private fun handleWildcardClicked() {
    showFragment(WildcardFragment.newInstance())
  }

  /** Notifies listener of unpair button click. */
  private fun handleUnpairDeviceClicked() {
    showFragment(UnpairDeviceFragment.newInstance())
  }

  private fun handleOTAProviderClicked() {
    showFragment(OtaProviderClientFragment.newInstance())
  }

  /** Notifies listener of provision-WiFi-credentials button click. */
  private fun handleProvisionWiFiCredentialsClicked() {
    getCallback()?.SetNetworkType(ProvisionNetworkType.WIFI)
    showFragment(BarcodeFragment.newInstance())
  }

  /** Notifies listener of provision-Thread-credentials button click. */
  private fun handleProvisionThreadCredentialsClicked() {
    getCallback()?.SetNetworkType(ProvisionNetworkType.THREAD)
    showFragment(BarcodeFragment.newInstance())
  }

  /** Notifies listener of provision-custom-flow button click. */
  private fun handleProvisionCustomFlowClicked() {
    showFragment(BarcodeFragment.newInstance())
  }

  private fun handleGroupSettingClicked() {
    showFragment(GroupSettingFragment.newInstance())
  }

  private fun handleICDClicked() {
    showFragment(ICDFragment.newInstance())
  }

  companion object {

    @JvmStatic fun newInstance() = SelectActionFragment()
  }
}
