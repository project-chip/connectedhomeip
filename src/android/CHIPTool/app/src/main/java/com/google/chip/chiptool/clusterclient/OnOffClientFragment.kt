package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.SeekBar
import android.widget.Toast
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipCommandType
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipDeviceControllerException
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.on_off_client_fragment.*
import kotlinx.android.synthetic.main.on_off_client_fragment.view.*

class OnOffClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController()

  private var commandType: ChipCommandType? = null
  private var levelValue: Int? = null

  override fun onCreateView(
      inflater: LayoutInflater,
      container: ViewGroup?,
      savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.on_off_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      onBtn.setOnClickListener { sendOnCommandClick() }
      offBtn.setOnClickListener { sendOffCommandClick() }
      toggleBtn.setOnClickListener { sendToggleCommandClick() }

      levelBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
        override fun onProgressChanged(seekBar: SeekBar, i: Int, b: Boolean) {

        }

        override fun onStartTrackingTouch(seekBar: SeekBar?) {
        }

        override fun onStopTrackingTouch(seekBar: SeekBar?) {
          Toast.makeText(requireContext(),
                  "Level is: " + levelBar.progress,
                  Toast.LENGTH_SHORT).show()
          commandType = ChipCommandType.LEVEL
          levelValue = levelBar.progress
          if (deviceController.isConnected) sendCommand() else connectToDevice()
        }
      })
    }
  }

  override fun onStart() {
    super.onStart()
    ipAddressEd.setText(deviceController.ipAddress ?: requireContext().getString(R.string.enter_ip_address_hint_text))
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
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

    override fun onError(error: Throwable?) {
      Log.d(TAG, "onError: $error")
    }
  }

  private fun sendOnCommandClick() {
    commandType = ChipCommandType.ON
    levelValue = 0
    if (deviceController.isConnected) sendCommand() else connectToDevice()
  }

  private fun sendOffCommandClick() {
    commandType = ChipCommandType.OFF
    levelValue = 0
    if (deviceController.isConnected) sendCommand() else connectToDevice()
  }

  private fun sendToggleCommandClick() {
    commandType = ChipCommandType.TOGGLE
    levelValue = 0
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
        requireContext().getString(R.string.send_command_type_label_text, chipCommandType.name, levelValue)

    try {
      // mask levelValue from integer to uint8_t and if null use 0
      deviceController.beginSendCommand(commandType, ( 0xff and (levelValue ?: 0)))
    } catch (e: ChipDeviceControllerException) {
      commandStatusTv.text = e.toString()
    }
  }

  companion object {
    private const val TAG = "OnOffClientFragment"
    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
