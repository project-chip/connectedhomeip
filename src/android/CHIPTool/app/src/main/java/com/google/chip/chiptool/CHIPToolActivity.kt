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
import android.nfc.NdefMessage
import android.nfc.NfcAdapter
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import chip.setuppayload.SetupPayloadParser
import com.google.chip.chiptool.attestation.AttestationTestFragment
import com.google.chip.chiptool.clusterclient.OnOffClientFragment
import com.google.chip.chiptool.commissioner.CommissionerActivity
import com.google.chip.chiptool.echoclient.EchoClientFragment
import com.google.chip.chiptool.provisioning.DeviceProvisioningFragment
import com.google.chip.chiptool.provisioning.EnterWifiNetworkFragment
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceDetailsFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo
import com.google.chip.chiptool.setuppayloadscanner.QrCodeInfo

class CHIPToolActivity :
    AppCompatActivity(),
    BarcodeFragment.Callback,
    SelectActionFragment.Callback,
    CHIPDeviceDetailsFragment.Callback {

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    setContentView(R.layout.top_activity)

    if (savedInstanceState == null) {
      val fragment = SelectActionFragment.newInstance()
      supportFragmentManager
          .beginTransaction()
          .add(R.id.fragment_container, fragment, fragment.javaClass.simpleName)
          .commit()
    }

    if (intent?.action == NfcAdapter.ACTION_NDEF_DISCOVERED)
      onNfcIntent(intent)
  }

  override fun onStartRendezvousOverBle(deviceInfo: CHIPDeviceInfo) {
    showFragment(DeviceProvisioningFragment.newInstance(deviceInfo))
  }

  override fun onCHIPDeviceInfoReceived(deviceInfo: CHIPDeviceInfo) {
    showFragment(CHIPDeviceDetailsFragment.newInstance(deviceInfo))
  }

  override fun handleScanQrCodeClicked() {
    showFragment(BarcodeFragment.newInstance())
  }

  override fun handleCommissioningClicked() {
    var intent = Intent(this, CommissionerActivity::class.java)
    startActivityForResult(intent, REQUEST_CODE_COMMISSIONING)
  }

  override fun handleEchoClientClicked() {
    showFragment(EchoClientFragment.newInstance())
  }

  override fun handleOnOffClicked() {
    showFragment(OnOffClientFragment.newInstance())
  }

  override fun handleAttestationTestClicked() {
    showFragment(AttestationTestFragment.newInstance())
  }

  override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
    super.onActivityResult(requestCode, resultCode, data)

    if (requestCode == REQUEST_CODE_COMMISSIONING) {
      // Simply ignore the commissioning result.
      // TODO: tracking commissioned devices.
    }
  }

  private fun showFragment(fragment: Fragment) {
    supportFragmentManager
        .beginTransaction()
        .replace(R.id.fragment_container, fragment, fragment.javaClass.simpleName)
        .addToBackStack(null)
        .commit()
  }

  private fun onNfcIntent(intent: Intent?) {
    // Require 1 NDEF message containing 1 NDEF record
    val messages = intent?.getParcelableArrayExtra(NfcAdapter.EXTRA_NDEF_MESSAGES)
    if (messages?.size != 1) return

    val records = (messages[0] as NdefMessage).records
    if (records.size != 1) return

    // Require NDEF URI record starting with "ch:"
    val uri = records[0].toUri()
    if (!uri?.scheme.equals("ch", true)) return

    val setupPayload = SetupPayloadParser().parseQrCode(uri.toString().toUpperCase())
    val deviceInfo = CHIPDeviceInfo(
            setupPayload.version,
            setupPayload.vendorId,
            setupPayload.productId,
            setupPayload.discriminator,
            setupPayload.setupPinCode,
            setupPayload.optionalQRCodeInfo.mapValues { (_, info) ->  QrCodeInfo(info.tag, info.type, info.data, info.int32) }
    )

    onCHIPDeviceInfoReceived(deviceInfo)
  }

  companion object {
    var REQUEST_CODE_COMMISSIONING = 0xB003
  }
}
