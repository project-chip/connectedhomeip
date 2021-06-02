package com.google.chip.chiptool.clusterclient

import android.content.Context
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
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
import com.google.chip.chiptool.util.DeviceIdUtil
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

      updateAddressBtn.setOnClickListener{ updateAddressClick() }
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
          sendCommand()
        }
      })
    }
  }

  override fun onStart() {
    super.onStart()
    // TODO: use the fabric ID that was used to commission the device
    val testFabricId = "5544332211"
    fabricIdEd.setText(testFabricId)
    deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
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

  private fun updateAddressClick() {
    val serviceInfo = NsdServiceInfo().apply {
      serviceName = "%016X-%016X".format(fabricIdEd.text.toString().toLong(), deviceIdEd.text.toString().toLong())
      serviceType = "_chip._tcp"
    }

    // TODO: implement the common CHIP mDNS interface for Android and make CHIP stack call the resolver
    val resolverListener = object : NsdManager.ResolveListener {
      override fun onResolveFailed(serviceInfo: NsdServiceInfo?, errorCode: Int) {
        showMessage("Address resolution failed: $errorCode")
      }

      override fun onServiceResolved(serviceInfo: NsdServiceInfo?) {
        val hostAddress = serviceInfo?.host?.hostAddress ?: ""
        val port = serviceInfo?.port ?: 0

        showMessage("Address: ${hostAddress}:${port}")

        if (hostAddress == "" || port == 0)
          return

        try {
          deviceController.updateAddress(deviceIdEd.text.toString().toLong(), hostAddress, port)
        } catch (e: ChipDeviceControllerException) {
          showMessage(e.toString())
        }
      }
    }

    (requireContext().getSystemService(Context.NSD_SERVICE) as NsdManager).apply {
      resolveService(serviceInfo, resolverListener)
    }
  }

  private fun sendOnCommandClick() {
    commandType = ChipCommandType.ON
    levelValue = 0
    sendCommand()
  }

  private fun sendOffCommandClick() {
    commandType = ChipCommandType.OFF
    levelValue = 0
    sendCommand()
  }

  private fun sendToggleCommandClick() {
    commandType = ChipCommandType.TOGGLE
    levelValue = 0
    sendCommand()
  }

  private fun sendCommand() {
    val chipCommandType = commandType ?: run {
      Log.e(TAG, "No ChipCommandType specified.")
      return
    }

    commandStatusTv.text = requireContext()
      .getString(R.string.send_command_type_label_text, chipCommandType.name, levelValue)

    try {
      // mask levelValue from integer to uint8_t and if null use 0
      deviceController.sendCommand(
        DeviceIdUtil.getLastDeviceId(requireContext()),
        commandType,
        ( 0xff and (levelValue ?: 0))
      )
    } catch (e: ChipDeviceControllerException) {
      showMessage(e.toString())
    }
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      commandStatusTv.text = msg
    }
  }

  companion object {
    private const val TAG = "OnOffClientFragment"
    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
