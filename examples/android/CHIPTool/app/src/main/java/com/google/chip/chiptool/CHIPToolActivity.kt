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

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.top_activity)

    ChipClient.startDnssd(this)

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
  }

  override fun onNewIntent(intent: Intent) {
    super.onNewIntent(intent)
    setIntent(intent)
    if (intent.getBooleanExtra("open_scan", false)) {
      supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
      showFragment(BarcodeFragment.newInstance(), false)
    }
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
    DeviceIdUtil.setCommissionedNodeId(this, nodeId)
    ChipClient.getDeviceController(this).close()

    if (code == 0L && nodeId > 0L) {
      val registry =
        CenterConnectionManager.registry(this).also { reg ->
          reg.autoRegisterCommissionedNode(nodeId)
          reg.saveFabricToPreferences(this)
        }
      CenterConnectionManager.onNodeCommissioned(nodeId)
      runOnUiThread {
        Toast.makeText(this, getString(R.string.commissioning_completed, code), Toast.LENGTH_SHORT)
          .show()
        if (!isFinishing && !isDestroyed) {
          PostCommissioningDialogHelper.showPostCommissioningDialog(
            activity = this,
            nodeId = nodeId,
            registry = registry,
            deviceInfo = deviceInfo,
            onComplete = {
              if (!isFinishing && !isDestroyed) {
                supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
                showFragment(DeviceCenterFragment.newInstance())
              }
            },
          )
        }
      }
    } else {
      runOnUiThread {
        Toast.makeText(this, getString(R.string.commissioning_completed, code), Toast.LENGTH_SHORT)
          .show()
        if (!isFinishing && !isDestroyed) {
          supportFragmentManager.popBackStack(null, FragmentManager.POP_BACK_STACK_INCLUSIVE)
          showFragment(SelectActionFragment.newInstance(), false)
        }
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
  }
}
