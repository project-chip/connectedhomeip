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

import android.content.Intent
import android.net.Uri
import android.nfc.NdefMessage
import android.nfc.NfcAdapter
import android.os.Bundle
import android.util.Base64
import android.util.Log
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import androidx.fragment.app.FragmentManager
import com.google.chip.chiptool.devicemanagement.CenterConnectionManager
import com.google.chip.chiptool.devicemanagement.CommissionedNodeRegistry
import com.google.chip.chiptool.devicemanagement.DeviceCenterFragment
import com.google.chip.chiptool.devicemanagement.PostCommissioningDialogHelper
import com.google.chip.chiptool.provisioning.AddressCommissioningFragment
import com.google.chip.chiptool.provisioning.DeviceProvisioningFragment
import com.google.chip.chiptool.provisioning.EnterNetworkFragment
import com.google.chip.chiptool.provisioning.ProvisionNetworkType
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceDetailsFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.setuppayloadscanner.CHIPLedgerDetailsFragment
import com.google.chip.chiptool.util.DeviceIdUtil
import matter.onboardingpayload.OnboardingPayload
import matter.onboardingpayload.OnboardingPayloadParser
import matter.onboardingpayload.UnrecognizedQrCodeException
import org.json.JSONObject

class CHIPToolActivity :
  AppCompatActivity(),
  BarcodeFragment.Callback,
  SelectActionFragment.Callback,
  DeviceProvisioningFragment.Callback,
  EnterNetworkFragment.Callback,
  CHIPDeviceDetailsFragment.Callback,
  CHIPLedgerDetailsFragment.Callback {

  private var networkType: ProvisionNetworkType? = null
  private var deviceInfo: CHIPDeviceInfo? = null
  private var isAutomatedCommissioningIntent: Boolean = false
  private var isInteractiveLaunch: Boolean = false

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.top_activity)

    if (savedInstanceState == null) {
      isInteractiveLaunch = isInteractiveIntent(intent)
      val fragment =
        if (intent?.getBooleanExtra("open_scan", false) == true) {
          BarcodeFragment.newInstance()
        } else {
          SelectActionFragment.newInstance()
        }
      supportFragmentManager
        .beginTransaction()
        .replace(R.id.nav_host_fragment, fragment, fragment.javaClass.simpleName)
        .commit()

      if (intent?.action == ACTION_COMMISSION_BLE_WIFI) {
        onCommissionBleWifiIntent(intent)
      } else if (intent?.action == ACTION_COMMISSION_BLE_THREAD) {
        onCommissionBleThreadIntent(intent)
      }
    } else {
      networkType =
        ProvisionNetworkType.fromName(savedInstanceState.getString(ARG_PROVISION_NETWORK_TYPE))
      @Suppress("DEPRECATION")
      deviceInfo = savedInstanceState.getParcelable(ARG_DEVICE_INFO)
      isAutomatedCommissioningIntent =
        savedInstanceState.getBoolean(ARG_IS_AUTOMATED_COMMISSIONING_INTENT, false)
      isInteractiveLaunch = savedInstanceState.getBoolean(ARG_IS_INTERACTIVE_LAUNCH, false)
    }

    if (intent?.action == NfcAdapter.ACTION_NDEF_DISCOVERED) onNfcIntent(intent)

    if (Intent.ACTION_VIEW == intent?.action) {
      onReturnIntent(intent)
    }
  }

  override fun onNewIntent(intent: Intent) {
    super.onNewIntent(intent)
    setIntent(intent)
    isAutomatedCommissioningIntent = false
    isInteractiveLaunch = isInteractiveIntent(intent)
    if (intent.getBooleanExtra("open_scan", false)) {
      clearBackStackIfSafe()
      showFragment(BarcodeFragment.newInstance(), false)
    } else if (intent.action == ACTION_COMMISSION_BLE_WIFI) {
      onCommissionBleWifiIntent(intent)
    } else if (intent.action == ACTION_COMMISSION_BLE_THREAD) {
      onCommissionBleThreadIntent(intent)
    } else if (intent.action == NfcAdapter.ACTION_NDEF_DISCOVERED) {
      onNfcIntent(intent)
    } else if (intent.action == Intent.ACTION_VIEW) {
      onReturnIntent(intent)
    } else {
      clearBackStackIfSafe()
      showFragment(SelectActionFragment.newInstance(), false)
    }
  }

  private fun isInteractiveIntent(candidate: Intent?): Boolean =
    candidate?.hasCategory(Intent.CATEGORY_LAUNCHER) == true ||
      candidate?.action == NfcAdapter.ACTION_NDEF_DISCOVERED ||
      candidate?.action == Intent.ACTION_VIEW

  private fun canCommitFragments(): Boolean =
    !isFinishing && !isDestroyed && !supportFragmentManager.isStateSaved

  private fun clearBackStackIfSafe() {
    if (canCommitFragments()) {
      supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
    }
  }

  private fun popBackStackOnceIfSafe() {
    if (canCommitFragments()) {
      supportFragmentManager.popBackStack()
    }
  }

  private fun hasValidCommissioningParams(discriminator: Int, setupPinCode: Long): Boolean =
    discriminator in 0..MAX_DISCRIMINATOR && setupPinCode in MIN_SETUP_PIN_CODE..MAX_SETUP_PIN_CODE

  private fun rejectCommissioningIntent(logMessage: String, toastMessage: String) {
    Log.e(TAG, logMessage)
    Toast.makeText(this, toastMessage, Toast.LENGTH_SHORT).show()
  }

  private fun onCommissionBleWifiIntent(intent: Intent) {
    isAutomatedCommissioningIntent = false
    val ssid = intent.getStringExtra(EXTRA_WIFI_SSID)
    if (ssid.isNullOrEmpty() || ssid.toByteArray(Charsets.UTF_8).size !in 1..MAX_WIFI_SSID_BYTES) {
      rejectCommissioningIntent(
        "Invalid or missing Wi-Fi SSID in commissioning intent",
        "Valid Wi-Fi SSID (1..32 UTF-8 bytes) required",
      )
      return
    }
    val psk = intent.getStringExtra(EXTRA_WIFI_PASSWORD) ?: ""
    if (psk.toByteArray(Charsets.UTF_8).size !in 0..MAX_WIFI_PSK_BYTES) {
      rejectCommissioningIntent(
        "Invalid Wi-Fi password length in commissioning intent",
        "Wi-Fi password must be 0..64 UTF-8 bytes",
      )
      return
    }
    val discriminator = intent.getIntExtra(EXTRA_DISCRIMINATOR, DEFAULT_DISCRIMINATOR)
    val setupPinCode = intent.getLongExtra(EXTRA_SETUP_PIN_CODE, DEFAULT_SETUP_PIN_CODE)
    if (!hasValidCommissioningParams(discriminator, setupPinCode)) {
      rejectCommissioningIntent(
        "Out-of-range discriminator=$discriminator or setupPinCode",
        "Invalid commissioning discriminator or setup PIN code",
      )
      return
    }

    isAutomatedCommissioningIntent = true
    isInteractiveLaunch = false
    val devInfo = CHIPDeviceInfo(discriminator = discriminator, setupPinCode = setupPinCode)
    this.deviceInfo = devInfo
    val networkCreds =
      NetworkCredentialsParcelable.forWiFi(NetworkCredentialsParcelable.WiFiCredentials(ssid, psk))
    clearBackStackIfSafe()
    showFragment(DeviceProvisioningFragment.newInstance(devInfo, networkCreds), false)
  }

  private fun decodeHexBytes(hex: String, expectedByteLength: Int? = null): ByteArray? {
    val normalized = hex.filterNot { it == ':' || it == ' ' }
    if (normalized.isEmpty() || normalized.length % 2 != 0) {
      return null
    }
    if (expectedByteLength != null && normalized.length != expectedByteLength * 2) {
      return null
    }
    return try {
      ByteArray(normalized.length / 2) { idx ->
        normalized.substring(idx * 2, idx * 2 + 2).toUByte(16).toByte()
      }
    } catch (_: NumberFormatException) {
      null
    }
  }

  private fun resolveThreadDatasetBytes(intent: Intent): ByteArray? {
    if (intent.hasExtra(EXTRA_THREAD_OPERATIONAL_DATASET)) {
      val rawDatasetHex = intent.getStringExtra(EXTRA_THREAD_OPERATIONAL_DATASET) ?: return null
      val decoded = decodeHexBytes(rawDatasetHex) ?: return null
      return decoded.takeIf { it.size in 1..MAX_THREAD_DATASET_BYTES }
    }
    val channel = intent.getIntExtra(EXTRA_THREAD_CHANNEL, DEFAULT_THREAD_CHANNEL)
    val panId = intent.getIntExtra(EXTRA_THREAD_PAN_ID, DEFAULT_THREAD_PAN_ID)
    if (channel !in MIN_THREAD_CHANNEL..MAX_THREAD_CHANNEL || panId !in 0..MAX_THREAD_PAN_ID) {
      return null
    }
    val xpanIdStr = intent.getStringExtra(EXTRA_THREAD_XPAN_ID) ?: "11:11:11:11:22:22:22:22"
    val masterKeyStr =
      intent.getStringExtra(EXTRA_THREAD_MASTER_KEY)
        ?: "00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF"
    val xpanIdBytes = decodeHexBytes(xpanIdStr, expectedByteLength = 8) ?: return null
    val masterKeyBytes = decodeHexBytes(masterKeyStr, expectedByteLength = 16) ?: return null
    var dataset =
      byteArrayOf(
        0.toByte(),
        3.toByte(),
        0.toByte(),
        (channel.shr(8) and 0xFF).toByte(),
        (channel and 0xFF).toByte(),
      )
    dataset +=
      byteArrayOf(
        1.toByte(),
        2.toByte(),
        (panId.shr(8) and 0xFF).toByte(),
        (panId and 0xFF).toByte(),
      )
    dataset += byteArrayOf(2.toByte(), 8.toByte()) + xpanIdBytes
    dataset += byteArrayOf(5.toByte(), 16.toByte()) + masterKeyBytes
    return dataset.takeIf { it.size in 1..MAX_THREAD_DATASET_BYTES }
  }

  private fun onCommissionBleThreadIntent(intent: Intent) {
    isAutomatedCommissioningIntent = false
    val discriminator = intent.getIntExtra(EXTRA_DISCRIMINATOR, DEFAULT_DISCRIMINATOR)
    val setupPinCode = intent.getLongExtra(EXTRA_SETUP_PIN_CODE, DEFAULT_SETUP_PIN_CODE)
    if (!hasValidCommissioningParams(discriminator, setupPinCode)) {
      rejectCommissioningIntent(
        "Out-of-range discriminator=$discriminator or setupPinCode",
        "Invalid commissioning discriminator or setup PIN code",
      )
      return
    }
    // LIT ICD registration (ICD Management RegisterClient during commissioning).
    // Defaults to true like CHIPDeviceInfo; tests pass false as a negative control.
    val isLIT = intent.getBooleanExtra(EXTRA_IS_LIT, true)

    val datasetBytes = resolveThreadDatasetBytes(intent)
    if (datasetBytes == null) {
      rejectCommissioningIntent(
        "Invalid Thread operational dataset parameters in commissioning intent",
        "Invalid Thread operational dataset",
      )
      return
    }

    isAutomatedCommissioningIntent = true
    isInteractiveLaunch = false
    val devInfo =
      CHIPDeviceInfo(discriminator = discriminator, setupPinCode = setupPinCode, isLIT = isLIT)
    this.deviceInfo = devInfo

    val networkCreds =
      NetworkCredentialsParcelable.forThread(
        NetworkCredentialsParcelable.ThreadCredentials(datasetBytes)
      )
    clearBackStackIfSafe()
    showFragment(DeviceProvisioningFragment.newInstance(devInfo, networkCreds), false)
  }

  override fun onSaveInstanceState(outState: Bundle) {
    outState.putString(ARG_PROVISION_NETWORK_TYPE, networkType?.name)
    outState.putParcelable(ARG_DEVICE_INFO, deviceInfo)
    outState.putBoolean(ARG_IS_AUTOMATED_COMMISSIONING_INTENT, isAutomatedCommissioningIntent)
    outState.putBoolean(ARG_IS_INTERACTIVE_LAUNCH, isInteractiveLaunch)

    super.onSaveInstanceState(outState)
  }

  override fun onCHIPDeviceInfoReceived(deviceInfo: CHIPDeviceInfo) {
    this.deviceInfo = deviceInfo
    popBackStackOnceIfSafe()
    val targetNetworkType = networkType
    if (targetNetworkType == null) {
      showFragment(CHIPDeviceDetailsFragment.newInstance(deviceInfo))
    } else if (deviceInfo.ipAddress != null) {
      showFragment(DeviceProvisioningFragment.newInstance(deviceInfo, null))
    } else {
      showFragment(EnterNetworkFragment.newInstance(targetNetworkType))
    }
  }

  private fun showCompletionToastIfActive(code: Long) {
    if (!isFinishing && !isDestroyed) {
      Toast.makeText(this, getString(R.string.commissioning_completed, code), Toast.LENGTH_SHORT)
        .show()
    }
  }

  private fun openDeviceCenterIfActive() {
    if (!canCommitFragments()) return
    supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
    showFragment(DeviceCenterFragment.newInstance())
  }

  private fun presentPostCommissioningDialog(
    code: Long,
    nodeId: Long,
    registry: CommissionedNodeRegistry,
  ) {
    showCompletionToastIfActive(code)
    if (!canCommitFragments()) return
    PostCommissioningDialogHelper.showPostCommissioningDialog(
      activity = this,
      nodeId = nodeId,
      registry = registry,
      deviceInfo = deviceInfo,
      onComplete = { openDeviceCenterIfActive() },
    )
  }

  override fun onCommissioningComplete(code: Long, nodeId: Long) {
    runOnUiThread {
      ChipClient.getDeviceController(this).close()
      val wasAutomatedIntent = isAutomatedCommissioningIntent
      isAutomatedCommissioningIntent = false

      if (code == 0L && nodeId > 0L) {
        DeviceIdUtil.setCommissionedNodeId(this, nodeId)
        val registry = CenterConnectionManager.registry(this)
        PostCommissioningDialogHelper.autoRegisterNode(registry, nodeId, deviceInfo, this)
        val defaultShowDialog = !wasAutomatedIntent && isInteractiveLaunch
        val shouldShowDialog =
          !wasAutomatedIntent &&
            (intent?.getBooleanExtra(EXTRA_SHOW_POST_COMMISSIONING_DIALOG, defaultShowDialog)
              ?: defaultShowDialog)
        if (shouldShowDialog) {
          presentPostCommissioningDialog(code, nodeId, registry)
          return@runOnUiThread
        }
      }

      showCompletionToastIfActive(code)
      if (canCommitFragments()) {
        clearBackStackIfSafe()
        showFragment(SelectActionFragment.newInstance(), false)
      }
    }
  }

  override fun onShowDeviceAddressInput() {
    popBackStackOnceIfSafe()
    showFragment(AddressCommissioningFragment.newInstance())
  }

  override fun onNetworkCredentialsEntered(networkCredentials: NetworkCredentialsParcelable) {
    val targetDeviceInfo = deviceInfo
    if (targetDeviceInfo == null) {
      Log.e(TAG, "onNetworkCredentialsEntered called with null deviceInfo; returning to scanner")
      Toast.makeText(this, "Device info lost; please scan the device again.", Toast.LENGTH_SHORT)
        .show()
      popBackStackOnceIfSafe()
      showFragment(BarcodeFragment.newInstance())
      return
    }
    showFragment(DeviceProvisioningFragment.newInstance(targetDeviceInfo, networkCredentials))
  }

  override fun handleReadFromLedgerClicked(deviceInfo: CHIPDeviceInfo) {
    showFragment(CHIPLedgerDetailsFragment.newInstance(deviceInfo))
  }

  override fun handleCustomFlowRedirectClicked(redirectUrl: String) {
    val redirectIntent = Intent(Intent.ACTION_VIEW, Uri.parse(redirectUrl))
    startActivity(redirectIntent)
  }

  override fun setNetworkType(type: ProvisionNetworkType?) {
    networkType = type
  }

  private fun showFragment(fragment: Fragment, showOnBack: Boolean = true) {
    if (!canCommitFragments()) return
    val fragmentTransaction =
      supportFragmentManager
        .beginTransaction()
        .replace(R.id.nav_host_fragment, fragment, fragment.javaClass.simpleName)

    if (showOnBack) {
      fragmentTransaction.addToBackStack(null)
    }

    fragmentTransaction.commit()
  }

  private fun onNfcIntent(intent: Intent?) {
    // Require 1 NDEF message containing 1 NDEF record
    val messages = intent?.getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES)
    if (messages?.size != 1) return

    val records = (messages[0] as NdefMessage).records
    if (records.size != 1) return

    // Require NDEF URI record starting with "mt:"
    val uri = records[0].toUri()
    if (!uri?.scheme.equals("mt", true)) return

    lateinit var setupPayload: OnboardingPayload
    try {
      setupPayload = OnboardingPayloadParser().parseQrCode(uri.toString().uppercase())
    } catch (ex: UnrecognizedQrCodeException) {
      Log.e(TAG, "Unrecognized QR Code", ex)
      Toast.makeText(this, "Unrecognized QR Code", Toast.LENGTH_SHORT).show()
      return
    }

    val deviceInfo = CHIPDeviceInfo.fromSetupPayload(setupPayload)

    val buttons =
      arrayOf(
        getString(R.string.nfc_tag_action_show),
        getString(R.string.nfc_tag_action_wifi),
        getString(R.string.nfc_tag_action_thread),
      )

    AlertDialog.Builder(this)
      .setTitle(R.string.nfc_tag_action_title)
      .setItems(buttons) { _, which ->
        this.networkType =
          when (which) {
            1 -> ProvisionNetworkType.WIFI
            2 -> ProvisionNetworkType.THREAD
            else -> null
          }
        onCHIPDeviceInfoReceived(deviceInfo)
      }
      .create()
      .show()
  }

  private fun onReturnIntent(intent: Intent) {
    val appLinkData = intent.data
    // Require URI schema "mt:"
    if (!appLinkData?.scheme.equals("mt", true)) {
      Log.d(TAG, "Unrecognized URI schema : ${appLinkData?.scheme}")
      return
    }
    // Require URI host "modelinfo"
    if (!appLinkData?.host.equals("modelinfo", true)) {
      Log.d(TAG, "Unrecognized URI host : ${appLinkData?.host}")
      return
    }

    // parse payload
    try {
      val payloadBase64String = appLinkData?.getQueryParameter("payload")
      if (payloadBase64String.isNullOrEmpty()) {
        Log.d(TAG, "Unrecognized payload")
        return
      }

      val decodeBytes = Base64.decode(payloadBase64String, Base64.DEFAULT)
      val payloadString = String(decodeBytes)
      val payload = JSONObject(payloadString)

      // parse payload from JSON
      val setupPayload = OnboardingPayload()
      // set defaults
      setupPayload.discoveryCapabilities = mutableSetOf()
      setupPayload.optionalQRCodeInfo = HashMap()

      // read from payload
      setupPayload.version = payload.getInt("version")
      setupPayload.vendorId = payload.getInt("vendorId")
      setupPayload.productId = payload.getInt("productId")
      setupPayload.commissioningFlow = payload.getInt("commissioningFlow")
      setupPayload.discriminator = payload.getInt("discriminator")
      setupPayload.setupPinCode = payload.getLong("setupPinCode")

      val deviceInfo = CHIPDeviceInfo.fromSetupPayload(setupPayload)
      val buttons = arrayOf(getString(R.string.nfc_tag_action_show))

      AlertDialog.Builder(this)
        .setTitle(R.string.provision_custom_flow_alert_title)
        .setItems(buttons) { _, _ -> onCHIPDeviceInfoReceived(deviceInfo) }
        .create()
        .show()
    } catch (ex: UnrecognizedQrCodeException) {
      Log.e(TAG, "Unrecognized Payload", ex)
      Toast.makeText(this, "Unrecognized Setup Payload", Toast.LENGTH_SHORT).show()
      return
    }
  }

  override fun onResume() {
    super.onResume()
    ChipClient.startDnssd(this)
  }

  override fun onPause() {
    ChipClient.stopDnssd(this)
    super.onPause()
  }

  companion object {
    private const val TAG = "CHIPToolActivity"
    private const val ADDRESS_COMMISSIONING_FRAGMENT_TAG = "address_commissioning_fragment"
    private const val ARG_PROVISION_NETWORK_TYPE = "provision_network_type"
    private const val ARG_DEVICE_INFO = "device_info"
    private const val ARG_IS_AUTOMATED_COMMISSIONING_INTENT = "is_automated_commissioning_intent"
    private const val ARG_IS_INTERACTIVE_LAUNCH = "is_interactive_launch"
    private const val EXTRA_SHOW_POST_COMMISSIONING_DIALOG = "show_post_commissioning_dialog"

    private const val DEFAULT_DISCRIMINATOR = 3840
    private const val MAX_DISCRIMINATOR = 0xFFF
    private const val DEFAULT_SETUP_PIN_CODE = 20202021L
    private const val MIN_SETUP_PIN_CODE = 1L
    private const val MAX_SETUP_PIN_CODE = 99_999_998L
    private const val MAX_WIFI_SSID_BYTES = 32
    private const val MAX_WIFI_PSK_BYTES = 64
    private const val MAX_THREAD_DATASET_BYTES = 254
    private const val DEFAULT_THREAD_CHANNEL = 15
    private const val MIN_THREAD_CHANNEL = 11
    private const val MAX_THREAD_CHANNEL = 26
    private const val DEFAULT_THREAD_PAN_ID = 0x1234
    private const val MAX_THREAD_PAN_ID = 0xFFFE

    const val ACTION_COMMISSION_BLE_WIFI = "com.google.chip.chiptool.action.COMMISSION_BLE_WIFI"
    const val EXTRA_DISCRIMINATOR = "discriminator"
    const val EXTRA_SETUP_PIN_CODE = "setupPinCode"
    const val EXTRA_WIFI_SSID = "wifiSsid"
    const val EXTRA_WIFI_PASSWORD = "wifiPassword"
    const val ACTION_COMMISSION_BLE_THREAD = "com.google.chip.chiptool.action.COMMISSION_BLE_THREAD"
    const val EXTRA_THREAD_OPERATIONAL_DATASET = "threadOperationalDataset"
    const val EXTRA_THREAD_CHANNEL = "threadChannel"
    const val EXTRA_THREAD_PAN_ID = "threadPanId"
    const val EXTRA_THREAD_XPAN_ID = "threadXpanId"
    const val EXTRA_THREAD_MASTER_KEY = "threadMasterKey"
    const val EXTRA_IS_LIT = "isLIT"
  }
}
