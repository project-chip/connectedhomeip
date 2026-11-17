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

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.top_activity)

    if (savedInstanceState == null) {
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
    } else {
      networkType =
        ProvisionNetworkType.fromName(savedInstanceState.getString(ARG_PROVISION_NETWORK_TYPE))
      @Suppress("DEPRECATION")
      deviceInfo = savedInstanceState.getParcelable(ARG_DEVICE_INFO)
    }

    if (intent?.action == NfcAdapter.ACTION_NDEF_DISCOVERED) onNfcIntent(intent)

    if (Intent.ACTION_VIEW == intent?.action) {
      onReturnIntent(intent)
    }

    if (intent?.action == ACTION_COMMISSION_BLE_WIFI) {
      onCommissionBleWifiIntent(intent)
    }

    if (intent?.action == ACTION_COMMISSION_BLE_THREAD) {
      onCommissionBleThreadIntent(intent)
    }
  }

  override fun onNewIntent(intent: Intent) {
    super.onNewIntent(intent)
    setIntent(intent)
    if (intent.getBooleanExtra("open_scan", false)) {
      isAutomatedCommissioningIntent = false
      supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
      showFragment(BarcodeFragment.newInstance(), false)
    } else if (intent.action == ACTION_COMMISSION_BLE_WIFI) {
      onCommissionBleWifiIntent(intent)
    } else if (intent.action == ACTION_COMMISSION_BLE_THREAD) {
      onCommissionBleThreadIntent(intent)
    } else if (
      intent.action != NfcAdapter.ACTION_NDEF_DISCOVERED && intent.action != Intent.ACTION_VIEW
    ) {
      supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
      showFragment(SelectActionFragment.newInstance(), false)
    }
  }

  private fun onCommissionBleWifiIntent(intent: Intent) {
    isAutomatedCommissioningIntent = true
    val discriminator = intent.getIntExtra(EXTRA_DISCRIMINATOR, 3840)
    val setupPinCode = intent.getLongExtra(EXTRA_SETUP_PIN_CODE, 20202021L)
    val ssid = intent.getStringExtra(EXTRA_WIFI_SSID)
    val psk = intent.getStringExtra(EXTRA_WIFI_PASSWORD)
    val devInfo = CHIPDeviceInfo(discriminator = discriminator, setupPinCode = setupPinCode)
    this.deviceInfo = devInfo
    val networkCreds =
      NetworkCredentialsParcelable.forWiFi(NetworkCredentialsParcelable.WiFiCredentials(ssid, psk))
    showFragment(DeviceProvisioningFragment.newInstance(devInfo, networkCreds), false)
  }

  private fun onCommissionBleThreadIntent(intent: Intent) {
    isAutomatedCommissioningIntent = true
    val discriminator = intent.getIntExtra(EXTRA_DISCRIMINATOR, 3840)
    val setupPinCode = intent.getLongExtra(EXTRA_SETUP_PIN_CODE, 20202021L)
    // LIT ICD registration (ICD Management RegisterClient during commissioning).
    // Defaults to true like CHIPDeviceInfo; tests pass false as a negative control.
    val isLIT = intent.getBooleanExtra(EXTRA_IS_LIT, true)
    val devInfo =
      CHIPDeviceInfo(discriminator = discriminator, setupPinCode = setupPinCode, isLIT = isLIT)
    this.deviceInfo = devInfo

    val rawDatasetHex = intent.getStringExtra(EXTRA_THREAD_OPERATIONAL_DATASET)
    val datasetBytes =
      if (!rawDatasetHex.isNullOrBlank()) {
        rawDatasetHex
          .filterNot { it == ':' || it == ' ' }
          .chunked(2)
          .map { it.toUByte(16).toByte() }
          .toByteArray()
      } else {
        val channel = intent.getIntExtra(EXTRA_THREAD_CHANNEL, 15)
        val panId = intent.getIntExtra(EXTRA_THREAD_PAN_ID, 0x1234)
        val xpanIdStr =
          (intent.getStringExtra(EXTRA_THREAD_XPAN_ID) ?: "11:11:11:11:22:22:22:22").filterNot {
            it == ':' || it == ' '
          }
        val masterKeyStr =
          (intent.getStringExtra(EXTRA_THREAD_MASTER_KEY)
              ?: "00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF")
            .filterNot { it == ':' || it == ' ' }
        val xpanIdBytes = xpanIdStr.chunked(2).map { it.toUByte(16).toByte() }.toByteArray()
        val masterKeyBytes = masterKeyStr.chunked(2).map { it.toUByte(16).toByte() }.toByteArray()

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
        dataset
      }

    val networkCreds =
      NetworkCredentialsParcelable.forThread(
        NetworkCredentialsParcelable.ThreadCredentials(datasetBytes)
      )
    showFragment(DeviceProvisioningFragment.newInstance(devInfo, networkCreds), false)
  }

  override fun onSaveInstanceState(outState: Bundle) {
    outState.putString(ARG_PROVISION_NETWORK_TYPE, networkType?.name)
    outState.putParcelable(ARG_DEVICE_INFO, deviceInfo)

    super.onSaveInstanceState(outState)
  }

  override fun onCHIPDeviceInfoReceived(deviceInfo: CHIPDeviceInfo) {
    this.deviceInfo = deviceInfo
    supportFragmentManager.popBackStack()
    val targetNetworkType = networkType
    if (targetNetworkType == null) {
      showFragment(CHIPDeviceDetailsFragment.newInstance(deviceInfo))
    } else {
      if (deviceInfo.ipAddress != null) {
        showFragment(DeviceProvisioningFragment.newInstance(deviceInfo, null))
      } else {
        showFragment(EnterNetworkFragment.newInstance(targetNetworkType))
      }
    }
  }

  override fun onCommissioningComplete(code: Long, nodeId: Long) {
    ChipClient.getDeviceController(this).close()

    if (code == 0L && nodeId > 0L) {
      DeviceIdUtil.setCommissionedNodeId(this, nodeId)
      val registry = CenterConnectionManager.registry(this)
      PostCommissioningDialogHelper.autoRegisterNode(registry, nodeId, deviceInfo, this)
      if (
        !isAutomatedCommissioningIntent &&
          intent?.getBooleanExtra("show_post_commissioning_dialog", false) == true
      ) {
        runOnUiThread {
          Toast.makeText(
              this,
              getString(R.string.commissioning_completed, code),
              Toast.LENGTH_SHORT,
            )
            .show()
          if (!isFinishing && !isDestroyed) {
            PostCommissioningDialogHelper.showPostCommissioningDialog(
              activity = this,
              nodeId = nodeId,
              registry = registry,
              deviceInfo = deviceInfo,
              onComplete = {
                if (!isFinishing && !isDestroyed) {
                  supportFragmentManager.popBackStack(
                    null,
                    FragmentManager.POP_BACK_STACK_INCLUSIVE,
                  )
                  showFragment(DeviceCenterFragment.newInstance())
                }
              },
            )
          }
        }
        return
      }
    }

    runOnUiThread {
      Toast.makeText(this, getString(R.string.commissioning_completed, code), Toast.LENGTH_SHORT)
        .show()
      if (!isFinishing && !isDestroyed) {
        supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
        showFragment(SelectActionFragment.newInstance(), false)
      }
    }
  }

  override fun onShowDeviceAddressInput() {
    supportFragmentManager.popBackStack()
    showFragment(AddressCommissioningFragment.newInstance())
  }

  override fun onNetworkCredentialsEntered(networkCredentials: NetworkCredentialsParcelable) {
    val targetDeviceInfo = deviceInfo
    if (targetDeviceInfo == null) {
      Log.e(TAG, "onNetworkCredentialsEntered called with null deviceInfo; returning to scanner")
      Toast.makeText(this, "Device info lost; please scan the device again.", Toast.LENGTH_SHORT)
        .show()
      supportFragmentManager.popBackStack()
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
