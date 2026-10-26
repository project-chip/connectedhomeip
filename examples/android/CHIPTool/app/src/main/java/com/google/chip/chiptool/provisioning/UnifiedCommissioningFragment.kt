/*
 *   Copyright (c) 2026 Project CHIP Authors
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
import android.content.Context
import android.net.wifi.WifiManager
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.CommissionParameters
import chip.devicecontroller.ICDDeviceInfo
import chip.devicecontroller.ICDRegistrationInfo
import chip.devicecontroller.NetworkCredentials
import com.google.android.material.button.MaterialButtonToggleGroup
import com.google.android.material.progressindicator.LinearProgressIndicator
import com.google.android.material.switchmaterial.SwitchMaterial
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.bluetooth.BluetoothManager
import com.google.chip.chiptool.devicemanagement.CenterConnectionManager
import com.google.chip.chiptool.devicemanagement.PostCommissioningDialogHelper
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.onboardingpayload.OnboardingPayload
import matter.onboardingpayload.OnboardingPayloadParser

class UnifiedCommissioningFragment : Fragment(), BarcodeFragment.Callback {

  companion object {
    private const val TAG = "UnifiedCommFragment"
    private const val ARG_DEVICE_INFO = "saved_device_info"
    private const val ARG_PAYLOAD_TEXT = "saved_payload_text"
    private const val PREFS_COMMISSIONING = "matter_unified_commissioning_prefs"
    private const val KEY_LAST_THREAD_DATASET = "last_thread_dataset_hex"
  }

  private lateinit var deviceController: ChipDeviceController
  private lateinit var scope: CoroutineScope
  @Volatile private var gatt: BluetoothGatt? = null
  @Volatile private var bleConnectionId: Int = 0
  private var deviceInfo: CHIPDeviceInfo? = null
  private var deviceId: Long = -1
  @Volatile private var pairingInProgress: Boolean = false

  @Volatile private var registerIcdClient: Boolean = false
  @Volatile private var pairingJob: kotlinx.coroutines.Job? = null

  private lateinit var payloadEditText: EditText
  private lateinit var parsePayloadButton: Button
  private lateinit var scanQrButton: Button
  private lateinit var barcodeContainer: View
  private lateinit var networkContainer: LinearLayout
  private lateinit var networkHintText: TextView
  private lateinit var networkToggleGroup: MaterialButtonToggleGroup
  private lateinit var wifiButton: Button
  private lateinit var threadButton: Button
  private lateinit var onNetworkButton: Button
  private lateinit var wifiContainer: LinearLayout
  private lateinit var threadContainer: LinearLayout
  private lateinit var ssidEditText: EditText
  private lateinit var passwordEditText: EditText
  private lateinit var threadDatasetEditText: EditText
  private lateinit var fetchThreadDatasetBtn: Button
  private lateinit var icdSwitch: SwitchMaterial
  private lateinit var startCommissioningButton: Button
  private lateinit var progressContainer: LinearLayout
  private lateinit var progressStageText: TextView
  private lateinit var progressBar: LinearProgressIndicator
  private lateinit var errorCard: View
  private lateinit var errorTextView: TextView
  private lateinit var retryButton: Button
  private lateinit var cancelButton: Button

  private val credentialsRequest = CredentialsRequestState()

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    deviceController = ChipClient.getDeviceController(requireContext())
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View {
    scope = viewLifecycleOwner.lifecycleScope
    return inflater.inflate(R.layout.fragment_unified_commissioning, container, false)
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    payloadEditText = view.findViewById(R.id.payloadEditText)
    parsePayloadButton = view.findViewById(R.id.parsePayloadButton)
    scanQrButton = view.findViewById(R.id.scanQrButton)
    barcodeContainer = view.findViewById(R.id.barcode_container)
    networkContainer = view.findViewById(R.id.networkContainer)
    networkHintText = view.findViewById(R.id.networkHintText)
    networkToggleGroup = view.findViewById(R.id.networkToggleGroup)
    wifiButton = view.findViewById(R.id.wifiButton)
    threadButton = view.findViewById(R.id.threadButton)
    onNetworkButton = view.findViewById(R.id.onNetworkButton)
    wifiContainer = view.findViewById(R.id.wifiContainer)
    threadContainer = view.findViewById(R.id.threadContainer)
    ssidEditText = view.findViewById(R.id.ssidEditText)
    passwordEditText = view.findViewById(R.id.passwordEditText)
    threadDatasetEditText = view.findViewById(R.id.threadDatasetEditText)
    fetchThreadDatasetBtn = view.findViewById(R.id.fetchThreadDatasetBtn)
    icdSwitch = view.findViewById(R.id.icdSwitch)
    startCommissioningButton = view.findViewById(R.id.startCommissioningButton)
    progressContainer = view.findViewById(R.id.progressContainer)
    progressStageText = view.findViewById(R.id.progressStageText)
    progressBar = view.findViewById(R.id.progressBar)
    errorCard = view.findViewById(R.id.errorCard)
    errorTextView = view.findViewById(R.id.errorTextView)
    retryButton = view.findViewById(R.id.retryButton)
    cancelButton = view.findViewById(R.id.cancelButton)

    parsePayloadButton.setOnClickListener { parsePayload(payloadEditText.text.toString()) }

    scanQrButton.setOnClickListener {
      barcodeContainer.visibility = View.VISIBLE
      childFragmentManager
        .beginTransaction()
        .replace(R.id.barcode_container, BarcodeFragment.newInstance())
        .commit()
    }

    networkToggleGroup.addOnButtonCheckedListener { _, checkedId, isChecked ->
      if (isChecked) {
        wifiContainer.visibility = if (checkedId == R.id.wifiButton) View.VISIBLE else View.GONE
        threadContainer.visibility = if (checkedId == R.id.threadButton) View.VISIBLE else View.GONE
      }
    }

    fetchThreadDatasetBtn.setOnClickListener {
      if (GmsThreadDatasetHelper.IS_AVAILABLE) {
        GmsThreadDatasetHelper.fetchFromGms(this) {
          Toast.makeText(requireContext(), R.string.gms_thread_not_available, Toast.LENGTH_SHORT)
            .show()
        }
      } else {
        Toast.makeText(requireContext(), R.string.gms_thread_not_available, Toast.LENGTH_SHORT)
          .show()
      }
    }

    startCommissioningButton.setOnClickListener {
      if (credentialsRequest.shouldSubmitCredentials) {
        submitNetworkCredentials()
      } else {
        startCommissioning()
      }
    }

    retryButton.setOnClickListener {
      errorCard.visibility = View.GONE
      if (credentialsRequest.shouldSubmitCredentials) {
        submitNetworkCredentials()
      } else {
        startCommissioning()
      }
    }

    cancelButton.setOnClickListener { cancelCommissioning() }

    prefillWifi()
    prefillThreadDataset()

    if (savedInstanceState != null) {
      @Suppress("DEPRECATION")
      val restoredInfo = savedInstanceState.getParcelable<CHIPDeviceInfo>(ARG_DEVICE_INFO)
      val restoredPayload = savedInstanceState.getString(ARG_PAYLOAD_TEXT)
      if (!restoredPayload.isNullOrEmpty()) {
        payloadEditText.setText(restoredPayload)
      }
      if (restoredInfo != null) {
        deviceInfo = restoredInfo
        setupFlowForPayload()
      }
    }
  }

  override fun onSaveInstanceState(outState: Bundle) {
    super.onSaveInstanceState(outState)
    outState.putParcelable(ARG_DEVICE_INFO, deviceInfo)
    if (this::payloadEditText.isInitialized) {
      outState.putString(ARG_PAYLOAD_TEXT, payloadEditText.text?.toString())
    }
  }

  private fun prefillWifi() {
    try {
      val wifiManager =
        requireContext().applicationContext.getSystemService(Context.WIFI_SERVICE) as? WifiManager
      val info = wifiManager?.connectionInfo
      if (info != null && info.ssid != null && info.ssid != "<unknown ssid>") {
        ssidEditText.setText(info.ssid.removePrefix("\"").removeSuffix("\""))
      }
    } catch (e: Exception) {
      Log.w(TAG, "Failed to prefill WiFi SSID: ${e.message}")
    }
  }

  private fun prefillThreadDataset() {
    try {
      val prefs = requireContext().getSharedPreferences(PREFS_COMMISSIONING, Context.MODE_PRIVATE)
      val savedHex = prefs.getString(KEY_LAST_THREAD_DATASET, null)
      if (!savedHex.isNullOrBlank() && threadDatasetEditText.text.isNullOrBlank()) {
        threadDatasetEditText.setText(savedHex)
      }
    } catch (e: Exception) {
      Log.w(TAG, "Failed to prefill Thread dataset: ${e.message}")
    }
  }

  override fun onDestroyView() {
    pairingJob?.cancel()
    pairingJob = null
    if (pairingInProgress && deviceId != -1L) {
      try {
        deviceController.stopDevicePairing(deviceId)
      } catch (e: Exception) {
        Log.w(TAG, "Error stopping device pairing in onDestroyView", e)
      }
      pairingInProgress = false
    }
    closeGatt()
    credentialsRequest.onSessionEnded()
    deviceController.setCompletionListener(null)
    deviceController.setWiFiCredentialsNeededListener(null)
    deviceController.setThreadCredentialsNeededListener(null)
    deviceController.setDeviceAttestationDelegate(0) { devicePtr, _, errorCode ->
      safeContinueCommissioning(devicePtr, errorCode == 0L)
    }
    super.onDestroyView()
  }

  override fun onCHIPDeviceInfoReceived(deviceInfo: CHIPDeviceInfo) {
    val child = childFragmentManager.findFragmentById(R.id.barcode_container)
    if (child != null) {
      childFragmentManager.beginTransaction().remove(child).commit()
    }
    barcodeContainer.visibility = View.GONE

    this.deviceInfo = deviceInfo
    payloadEditText.setText(deviceInfo.setupCode)
    setupFlowForPayload()
  }

  private fun parseOnboardingPayload(rawPayload: String): OnboardingPayload? {
    val code = rawPayload.trim()
    if (code.isEmpty()) return null
    val parser = OnboardingPayloadParser()
    return try {
      if (code.startsWith("MT:", ignoreCase = true)) {
        parser.parseQrCode(code)
      } else {
        parser.parseManualPairingCode(code)
      }
    } catch (_: Exception) {
      null
    }
  }

  private fun parsePayload(payloadString: String) {
    val code = payloadString.trim()
    val payload = parseOnboardingPayload(code)
    if (payload != null) {
      deviceInfo = CHIPDeviceInfo.fromSetupPayload(payload, true, code)
      setupFlowForPayload()
    } else {
      Toast.makeText(requireContext(), R.string.invalid_payload, Toast.LENGTH_SHORT).show()
    }
  }

  private fun setupFlowForPayload() {
    val info = deviceInfo ?: return
    networkContainer.visibility = View.VISIBLE
    icdSwitch.isChecked = info.isLIT

    // Preselection
    val payload = parseOnboardingPayload(payloadEditText.text.toString())

    val netType = UnifiedCommissioningLogic.guessNetworkTypeFromPayload(payload)
    when {
      info.ipAddress != null || netType == NetworkType.ON_NETWORK ->
        networkToggleGroup.check(R.id.onNetworkButton)
      else -> networkToggleGroup.check(R.id.wifiButton)
    }
  }

  private fun showError(message: String) {
    errorTextView.text = message
    errorCard.visibility = View.VISIBLE
  }

  /**
   * Terminal failure of the commissioning session. The session is gone, so a pending credentials
   * request must be dropped too: Retry then starts a new session instead of sending credentials
   * nobody is waiting for.
   */
  private fun onCommissioningFailed(message: String) {
    pairingInProgress = false
    credentialsRequest.onSessionEnded()
    progressContainer.visibility = View.GONE
    wifiButton.isEnabled = true
    threadButton.isEnabled = true
    onNetworkButton.isEnabled = true
    startCommissioningButton.text = getString(R.string.start_commissioning)
    showError(message)
    closeGatt()
    deviceController.setCompletionListener(null)
  }

  private fun updateProgress(stage: String) {
    progressStageText.text = stage
  }

  private fun closeGatt() {
    try {
      deviceController.close()
    } catch (e: Exception) {
      Log.w(TAG, "Error closing deviceController BLE state", e)
    }
    if (bleConnectionId != 0) {
      try {
        context?.let {
          ChipClient.getAndroidChipPlatform(it).bleManager.removeConnection(bleConnectionId)
        }
      } catch (e: Exception) {
        Log.w(TAG, "Error removing bleConnectionId $bleConnectionId", e)
      }
      bleConnectionId = 0
    }
    try {
      gatt?.disconnect()
      gatt?.close()
    } catch (e: Exception) {
      Log.w(TAG, "Error closing BluetoothGatt", e)
    }
    gatt = null
  }

  private fun cancelCommissioning() {
    pairingJob?.cancel()
    pairingJob = null
    if (pairingInProgress && deviceId != -1L) {
      try {
        deviceController.stopDevicePairing(deviceId)
      } catch (e: Exception) {
        Log.w(TAG, "Error stopping device pairing in cancelCommissioning", e)
      }
      pairingInProgress = false
    }
    closeGatt()
    deviceController.setCompletionListener(null)
    credentialsRequest.onSessionEnded()

    progressContainer.visibility = View.GONE
    networkContainer.visibility = View.VISIBLE
    networkToggleGroup.isEnabled = true
    wifiButton.isEnabled = true
    threadButton.isEnabled = true
    onNetworkButton.isEnabled = true
    startCommissioningButton.text = getString(R.string.start_commissioning)
  }

  private fun submitNetworkCredentials() {
    val type =
      UnifiedCommissioningLogic.getValidationTypeForState(
        networkToggleGroup.checkedButtonId,
        credentialsRequest.awaiting,
      )

    var networkCredentials: NetworkCredentials? = null
    when (type) {
      NetworkType.WIFI -> {
        val ssid = ssidEditText.text.toString()
        val pass = passwordEditText.text.toString()
        if (!UnifiedCommissioningLogic.isValidWifiCredentials(ssid, pass)) {
          showError(getString(R.string.invalid_wifi))
          return
        }
        networkCredentials =
          NetworkCredentials.forWiFi(NetworkCredentials.WiFiCredentials(ssid, pass))
      }
      NetworkType.THREAD -> {
        val datasetHex = threadDatasetEditText.text.toString()
        if (!UnifiedCommissioningLogic.isValidThreadDataset(datasetHex)) {
          showError(getString(R.string.invalid_thread))
          return
        }
        val datasetBytes = datasetHex.chunked(2).map { it.toInt(16).toByte() }.toByteArray()
        networkCredentials =
          NetworkCredentials.forThread(NetworkCredentials.ThreadCredentials(datasetBytes))
      }
      else -> {}
    }

    credentialsRequest.onSessionEnded()
    errorCard.visibility = View.GONE
    networkContainer.visibility = View.GONE
    progressContainer.visibility = View.VISIBLE
    updateProgress(getString(R.string.stage_network_setup))

    networkToggleGroup.isEnabled = true
    wifiButton.isEnabled = true
    threadButton.isEnabled = true
    onNetworkButton.isEnabled = true
    startCommissioningButton.text = getString(R.string.start_commissioning)

    if (networkCredentials != null) {
      deviceController.updateCommissioningNetworkCredentials(networkCredentials)
    }
  }

  private fun startCommissioning() {
    val info = deviceInfo ?: return
    progressContainer.visibility = View.VISIBLE
    networkContainer.visibility = View.GONE
    errorCard.visibility = View.GONE
    updateProgress(getString(R.string.stage_connecting))

    registerIcdClient = icdSwitch.isChecked
    credentialsRequest.onSessionEnded()

    deviceController.setWiFiCredentialsNeededListener {
      activity?.runOnUiThread {
        if (isAdded) {
          credentialsRequest.onCredentialsNeeded(NetworkType.WIFI)
          networkContainer.visibility = View.VISIBLE
          networkToggleGroup.check(R.id.wifiButton)
          wifiButton.isEnabled = true
          threadButton.isEnabled = false
          onNetworkButton.isEnabled = false
          startCommissioningButton.text = getString(R.string.continue_btn)
          progressContainer.visibility = View.GONE
        }
      }
    }

    deviceController.setThreadCredentialsNeededListener {
      activity?.runOnUiThread {
        if (isAdded) {
          credentialsRequest.onCredentialsNeeded(NetworkType.THREAD)
          networkContainer.visibility = View.VISIBLE
          networkToggleGroup.check(R.id.threadButton)
          wifiButton.isEnabled = false
          threadButton.isEnabled = true
          onNetworkButton.isEnabled = false
          startCommissioningButton.text = getString(R.string.continue_btn)
          progressContainer.visibility = View.GONE
        }
      }
    }

    if (networkToggleGroup.checkedButtonId == R.id.onNetworkButton || info.ipAddress != null) {
      pairOnNetwork(info)
    } else {
      pairOverBle(info)
    }
  }

  private fun pairOnNetwork(info: CHIPDeviceInfo) {
    deviceId = DeviceIdUtil.getNextAvailableId(requireContext())
    pairingInProgress = true
    deviceController.setCompletionListener(ConnectionCallback(deviceId))
    setAttestationDelegate()

    val paramsBuilder = CommissionParameters.Builder()
    if (registerIcdClient) {
      paramsBuilder.setICDRegistrationInfo(ICDRegistrationInfo.createForDeferredConfiguration())
    }
    val params = paramsBuilder.build()

    if (info.ipAddress != null) {
      deviceController.pairDeviceWithAddress(
        deviceId,
        info.ipAddress,
        info.port,
        info.discriminator,
        info.setupPinCode,
        params,
      )
    } else {
      val code = info.setupCode.ifEmpty { payloadEditText.text.toString().trim() }
      deviceController.pairDeviceWithCode(
        deviceId,
        code,
        false,
        false,
        params,
      )
    }
  }

  private fun pairOverBle(info: CHIPDeviceInfo) {
    closeGatt()
    pairingJob?.cancel()
    pairingJob =
      scope.launch {
        UnifiedCommissioningLogic.runReportingFailures(
          onFailure = { e ->
            pairingInProgress = false
            activity?.runOnUiThread {
              if (isAdded) {
                onCommissioningFailed(getString(R.string.ble_pairing_failed, e.message))
              }
            }
          }
        ) {
          val bluetoothManager = BluetoothManager()
          val device =
            bluetoothManager.getBluetoothDevice(
              requireContext(),
              info.discriminator,
              info.isShortDiscriminator,
            )
          if (device == null) {
            activity?.runOnUiThread {
              if (isAdded) {
                onCommissioningFailed(getString(R.string.ble_not_found))
              }
            }
            return@runReportingFailures
          }
          activity?.runOnUiThread {
            if (isAdded) updateProgress(getString(R.string.stage_connecting_ble))
          }
          gatt = bluetoothManager.connect(requireContext(), device)

          deviceId = DeviceIdUtil.getNextAvailableId(requireContext())
          pairingInProgress = true
          bleConnectionId = bluetoothManager.connectionId
          val connId = bleConnectionId

          deviceController.setCompletionListener(ConnectionCallback(deviceId))
          setAttestationDelegate()

          val paramsBuilder = CommissionParameters.Builder()
          if (registerIcdClient) {
            paramsBuilder.setICDRegistrationInfo(
              ICDRegistrationInfo.createForDeferredConfiguration()
            )
          }
          val params = paramsBuilder.build()

          deviceController.pairDeviceThroughBLE(gatt, connId, deviceId, info.setupPinCode, params)
        }
      }
  }

  private fun safeContinueCommissioning(devicePtr: Long, proceed: Boolean) {
    try {
      deviceController.continueCommissioning(devicePtr, proceed)
    } catch (e: Exception) {
      Log.w(TAG, "Error in continueCommissioning", e)
    }
  }

  private fun setAttestationDelegate() {
    deviceController.setDeviceAttestationDelegate(600) { devicePtr, _, errorCode ->
      val act = activity
      if (act == null) {
        safeContinueCommissioning(devicePtr, false)
        return@setDeviceAttestationDelegate
      }
      act.runOnUiThread {
        if (!isAdded) {
          safeContinueCommissioning(devicePtr, false)
          return@runOnUiThread
        }
        if (errorCode == 0L) {
          safeContinueCommissioning(devicePtr, true)
          return@runOnUiThread
        }
        AlertDialog.Builder(act)
          .setTitle(R.string.attestation_failed)
          .setMessage(getString(R.string.attestation_failed_msg, errorCode))
          .setPositiveButton(R.string.continue_btn) { _, _ ->
            safeContinueCommissioning(devicePtr, true)
          }
          .setNegativeButton(android.R.string.cancel) { _, _ ->
            safeContinueCommissioning(devicePtr, false)
            cancelCommissioning()
          }
          .setCancelable(false)
          .show()
      }
    }
  }

  @Deprecated("Deprecated in Java")
  override fun onActivityResult(requestCode: Int, resultCode: Int, data: android.content.Intent?) {
    super.onActivityResult(requestCode, resultCode, data)
    val candidates =
      GmsThreadDatasetHelper.parseConsentResult(requestCode, resultCode, data) ?: return
    if (view == null) return
    when (candidates.size) {
      0 -> Toast.makeText(requireContext(), R.string.no_thread_network, Toast.LENGTH_SHORT).show()
      1 -> onThreadDatasetFetched(candidates[0].activeDatasetTlvs)
      else -> showThreadNetworkPicker(candidates)
    }
  }

  private fun showThreadNetworkPicker(candidates: List<ThreadDatasetCandidate>) {
    val labels =
      candidates
        .map {
          "${it.networkName} (${it.extendedPanId.joinToString("") { byte -> "%02X".format(byte) }})"
        }
        .toTypedArray()
    AlertDialog.Builder(requireContext())
      .setTitle(R.string.select_thread_network)
      .setItems(labels) { _, which -> onThreadDatasetFetched(candidates[which].activeDatasetTlvs) }
      .setNegativeButton(android.R.string.cancel, null)
      .show()
  }

  private fun onThreadDatasetFetched(tlvs: ByteArray) {
    val hex = tlvs.joinToString("") { "%02X".format(it) }
    threadDatasetEditText.setText(hex)
    try {
      context
        ?.getSharedPreferences(PREFS_COMMISSIONING, Context.MODE_PRIVATE)
        ?.edit()
        ?.putString(KEY_LAST_THREAD_DATASET, hex)
        ?.apply()
    } catch (_: Exception) {}
  }

  inner class ConnectionCallback(private val cbDeviceId: Long) : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {
      activity?.runOnUiThread { if (isAdded) updateProgress(getString(R.string.stage_attestation)) }
    }

    override fun onCommissioningComplete(nodeId: Long, errorCode: Long) {
      pairingInProgress = false
      activity?.runOnUiThread {
        if (isAdded) {
          if (errorCode == 0L) {
            closeGatt()
            deviceController.setCompletionListener(null)
            credentialsRequest.onSessionEnded()
            updateProgress(getString(R.string.stage_done))
            val registry = CenterConnectionManager.registry(requireContext())
            DeviceIdUtil.setCommissionedNodeId(requireContext(), nodeId)
            DeviceIdUtil.setNextAvailableId(requireContext(), maxOf(cbDeviceId, nodeId) + 1)

            PostCommissioningDialogHelper.showPostCommissioningDialog(
              requireActivity(),
              nodeId,
              registry,
              deviceInfo,
            ) {
              if (isAdded) {
                parentFragmentManager.popBackStack()
              }
            }
          } else {
            onCommissioningFailed(getString(R.string.commissioning_failed, errorCode))
          }
        }
      }
    }

    override fun onReadCommissioningInfo(
      vendorId: Int,
      productId: Int,
      wifiEndpointId: Int,
      threadEndpointId: Int,
    ) {
      activity?.runOnUiThread {
        if (isAdded) {
          var capabilities = ""
          if (wifiEndpointId != -1) capabilities += "Wi-Fi "
          if (threadEndpointId != -1) capabilities += "Thread"
          if (capabilities.isNotEmpty()) {
            networkHintText.visibility = View.VISIBLE
            networkHintText.text = getString(R.string.supports_hint, capabilities.trim())
          }
          if (wifiEndpointId == -1 && threadEndpointId != -1) {
            networkToggleGroup.check(R.id.threadButton)
          } else if (wifiEndpointId != -1 && threadEndpointId == -1) {
            networkToggleGroup.check(R.id.wifiButton)
          }
        }
      }
    }

    override fun onCommissioningStatusUpdate(nodeId: Long, stage: String, errorCode: Long) {
      activity?.runOnUiThread {
        if (isAdded && stage != null) {
          updateProgress(stage)
        }
      }
    }

    override fun onICDRegistrationInfoRequired() {
      if (registerIcdClient) {
        deviceController.updateCommissioningICDRegistrationInfo(
          ICDRegistrationInfo.newBuilder().setICDStayActiveDurationMsec(30000L).build()
        )
      }
    }

    override fun onICDRegistrationComplete(errorCode: Long, icdDeviceInfo: ICDDeviceInfo) {
      activity?.runOnUiThread {
        if (isAdded) {
          Log.i(TAG, "ICD registration complete. error: $errorCode")
        }
      }
    }

    override fun onPairingComplete(code: Long) {
      activity?.runOnUiThread {
        if (isAdded) {
          if (code != 0L) {
            onCommissioningFailed(getString(R.string.commissioning_failed, code))
          }
        }
      }
    }

    override fun onPairingDeleted(code: Long) {}

    override fun onCloseBleComplete() {
      android.os.Handler(android.os.Looper.getMainLooper()).post { closeGatt() }
    }

    override fun onNotifyChipConnectionClosed() {
      android.os.Handler(android.os.Looper.getMainLooper()).post { closeGatt() }
    }

    override fun onError(error: Throwable?) {
      pairingInProgress = false
      activity?.runOnUiThread {
        if (isAdded) {
          onCommissioningFailed(getString(R.string.error_msg, error?.message))
        }
      }
    }
  }
}
