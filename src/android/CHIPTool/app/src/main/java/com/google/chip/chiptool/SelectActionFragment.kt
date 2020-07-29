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
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.util.FragmentUtil
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
      echoClientBtn.setOnClickListener { getCallback()?.handleEchoClientClicked() }
      onOffClusterBtn.setOnClickListener { getCallback()?.handleOnOffClicked() }
    }
  }

  private fun getCallback() = FragmentUtil.getHost(this, Callback::class.java)

  /** Interface for notifying the host. */
  interface Callback {
    /** Notifies listener of Scan QR code button click. */
    fun handleScanQrCodeClicked()
    /** Notifies listener of Echo client button click. */
    fun handleEchoClientClicked()
    /** Notifies listener of send command button click. */
    fun handleOnOffClicked()
  }

  companion object {

    @JvmStatic fun newInstance() = SelectActionFragment()
  }
}
