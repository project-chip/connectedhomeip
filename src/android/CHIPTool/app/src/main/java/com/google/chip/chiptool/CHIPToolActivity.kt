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
import androidx.appcompat.app.AppCompatActivity
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.echoclient.EchoClientFragment
import com.google.chip.chiptool.setuppayloadscanner.BarcodeFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceDetailsFragment
import com.google.chip.chiptool.setuppayloadscanner.CHIPDeviceInfo

class CHIPToolActivity :
    AppCompatActivity(),
    BarcodeFragment.Callback,
    SelectActionFragment.Callback {

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
  }

  override fun onCHIPDeviceInfoReceived(deviceInfo: CHIPDeviceInfo) {
    showFragment(CHIPDeviceDetailsFragment.newInstance(deviceInfo))
  }

  override fun handleScanQrCodeClicked() {
    showFragment(BarcodeFragment.newInstance())
  }

  override fun handleEchoClientClicked() {
    showFragment(EchoClientFragment.newInstance())
  }

  override fun handleOnOffClicked() {
    TODO("Not yet implemented")
  }

  private fun showFragment(fragment: Fragment) {
    supportFragmentManager
        .beginTransaction()
        .replace(R.id.fragment_container, fragment, fragment.javaClass.simpleName)
        .addToBackStack(null)
        .commit()
  }
}
