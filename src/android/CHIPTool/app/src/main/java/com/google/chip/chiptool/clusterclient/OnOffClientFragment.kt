package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.SeekBar
import android.widget.Toast
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.OnOffCluster
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipDeviceControllerException
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.on_off_client_fragment.commandStatusTv
import kotlinx.android.synthetic.main.on_off_client_fragment.deviceIdEd
import kotlinx.android.synthetic.main.on_off_client_fragment.fabricIdEd
import kotlinx.android.synthetic.main.on_off_client_fragment.levelBar
import kotlinx.android.synthetic.main.on_off_client_fragment.view.levelBar
import kotlinx.android.synthetic.main.on_off_client_fragment.view.offBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.onBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.readBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.toggleBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.view.updateAddressBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class OnOffClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.on_off_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      updateAddressBtn.setOnClickListener { updateAddressClick() }
      onBtn.setOnClickListener { scope.launch { sendOnCommandClick() } }
      offBtn.setOnClickListener { scope.launch { sendOffCommandClick() } }
      toggleBtn.setOnClickListener { scope.launch { sendToggleCommandClick() } }
      readBtn.setOnClickListener { scope.launch { sendReadOnOffClick() } }

      levelBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
        override fun onProgressChanged(seekBar: SeekBar, i: Int, b: Boolean) {

        }

        override fun onStartTrackingTouch(seekBar: SeekBar?) {
        }

        override fun onStopTrackingTouch(seekBar: SeekBar?) {
          Toast.makeText(
            requireContext(),
            "Level is: " + levelBar.progress,
            Toast.LENGTH_SHORT
          ).show()
          scope.launch { sendLevelCommandClick() }
        }
      })
    }
  }

  private suspend fun sendReadOnOffClick() {
    getOnOffClusterForDevice().readOnOffAttribute(object : ChipClusters.BooleanAttributeCallback {
      override fun onSuccess(on: Boolean) {
        Log.v(TAG, "On/Off attribute value: $on")
        showMessage("On/Off attribute value: $on")
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error reading onOff attribute", ex)
      }
    })
  }

  override fun onStart() {
    super.onStart()
    // TODO: use the fabric ID that was used to commission the device
    val testFabricId = "5544332211"
    fabricIdEd.setText(testFabricId)
    deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
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

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  private fun updateAddressClick() {
    try{
      deviceController.updateDevice(
          fabricIdEd.text.toString().toULong().toLong(),
          deviceIdEd.text.toString().toULong().toLong()
      )
      showMessage("Address update started")
    } catch (ex: Exception) {
      showMessage("Address update failed: $ex")
    }
  }

  private suspend fun sendLevelCommandClick() {
    val cluster = ChipClusters.LevelControlCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), deviceIdEd.text.toString().toLong()), 1
    )
    cluster.moveToLevel(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("MoveToLevel command success")
      }

      override fun onError(ex: Exception) {
        showMessage("MoveToLevel command failure $ex")
        Log.e(TAG, "MoveToLevel command failure", ex)
      }

    }, levelBar.progress, 0, 0, 0)
  }

  private suspend fun sendOnCommandClick() {
    getOnOffClusterForDevice().on(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("ON command success")
      }

      override fun onError(ex: Exception) {
        showMessage("ON command failure $ex")
        Log.e(TAG, "ON command failure", ex)
      }

    })
  }

  private suspend fun sendOffCommandClick() {
    getOnOffClusterForDevice().off(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("OFF command success")
      }

      override fun onError(ex: Exception) {
        showMessage("OFF command failure $ex")
        Log.e(TAG, "OFF command failure", ex)
      }
    })
  }

  private suspend fun sendToggleCommandClick() {
    getOnOffClusterForDevice().toggle(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("TOGGLE command success")
      }

      override fun onError(ex: Exception) {
        showMessage("TOGGLE command failure $ex")
        Log.e(TAG, "TOGGLE command failure", ex)
      }
    })
  }

  private suspend fun getOnOffClusterForDevice(): OnOffCluster {
    return OnOffCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), deviceIdEd.text.toString().toLong()), 1
    )
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
