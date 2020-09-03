package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipCommandType
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.on_off_client_fragment.*
import kotlinx.android.synthetic.main.on_off_client_fragment.view.*

class OnOffClientFragment : Fragment(), ChipDeviceController.CompletionListener {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController()

  private var commandType: ChipCommandType? = null

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.on_off_client_fragment, container, false).apply {
      deviceController.setCompletionListener(this@OnOffClientFragment)

      onBtn.setOnClickListener { sendOnCommandClick() }
      offBtn.setOnClickListener { sendOffCommandClick() }
      toggleBtn.setOnClickListener { sendToggleCommandClick() }
    }
  }

  override fun onConnectDeviceComplete() {
    sendCommand()
  }

  override fun onSendMessageComplete(message: String?) {
    commandStatusTv.text = requireContext().getString(R.string.echo_status_response, message)
  }

  override fun onNotifyChipConnectionClosed() {
    Log.d(TAG, "onNotifyChipConnectionClosed")
  }

  override fun onCloseBleComplete() {
    Log.d(TAG, "onCloseBleComplete")
  }

  override fun onError(error: Throwable) {
    Log.d(TAG, "onError: $error")
  }

  private fun sendOnCommandClick() {
    commandType = ChipCommandType.ON
    if (deviceController.isConnected) sendCommand() else connectToDevice()
  }

  private fun sendOffCommandClick() {
    commandType = ChipCommandType.OFF
    if (deviceController.isConnected) sendCommand() else connectToDevice()
  }

  private fun sendToggleCommandClick() {
    commandType = ChipCommandType.TOGGLE
    if (deviceController.isConnected) sendCommand() else connectToDevice()
  }

  private fun connectToDevice() {
    commandStatusTv.text = requireContext().getString(R.string.echo_status_connecting)
    deviceController.apply {
      disconnectDevice()
      beginConnectDevice(ipAddressEd.text.toString())
    }
  }

  private fun sendCommand() {
    val chipCommandType = commandType ?: run {
      Log.e(TAG, "No ChipCommandType specified.")
      return
    }

    commandStatusTv.text =
        requireContext().getString(R.string.send_command_type_label_text, chipCommandType.name)

    deviceController.beginSendCommand(commandType)
  }

  companion object {
    private const val TAG = "OnOffClientFragment"
    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
