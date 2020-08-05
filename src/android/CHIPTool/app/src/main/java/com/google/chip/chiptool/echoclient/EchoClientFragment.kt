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
package com.google.chip.chiptool.echoclient

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.send_command_fragment.*
import kotlinx.android.synthetic.main.send_command_fragment.view.*

/** Sends echo messages to the CHIP device. */
class EchoClientFragment : Fragment(), ChipDeviceController.CompletionListener {

  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController()

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.send_command_fragment, container, false).apply {
      deviceController.setCompletionListener(this@EchoClientFragment)

      inputTextEd.hint = requireContext().getString(R.string.echo_input_hint_text)

      sendCommandBtn.setOnClickListener { onSendCommandClick() }
    }
  }

  private fun onSendCommandClick() {
    if (deviceController.isConnected) {
      sendEcho()
    } else {
      commandStatusTv.text = requireContext().getString(R.string.echo_status_connecting)
      deviceController.beginConnectDevice(ipAddressEd.text.toString())
    }
  }

  override fun onConnectDeviceComplete() {
    sendEcho()
  }

  override fun onSendMessageComplete(message: String?) {
    commandStatusTv.text = requireContext().getString(R.string.echo_status_response, message)
  }

  private fun sendEcho() {
    commandStatusTv.text = requireContext().getString(R.string.echo_status_sending_message)

    val echoEditTextContents = inputTextEd.text.toString()
    val echoText = if (echoEditTextContents.isEmpty()) DEFAULT_ECHO_MSG else echoEditTextContents
    deviceController.beginSendMessage(echoText)
  }

  companion object {

    private const val DEFAULT_ECHO_MSG = "Hello! from the Android CHIP demo app"

    fun newInstance(): EchoClientFragment = EchoClientFragment()
  }
}
