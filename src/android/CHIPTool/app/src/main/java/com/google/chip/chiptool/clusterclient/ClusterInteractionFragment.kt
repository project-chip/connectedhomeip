package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.SeekBar
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.RecyclerView
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.cluster_interaction_fragment.*
import kotlinx.android.synthetic.main.cluster_interaction_fragment.view.enrollDeviceBtn
import kotlinx.android.synthetic.main.on_off_client_fragment.commandStatusTv
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel

class ClusterInteractionFragment: Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.cluster_interaction_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())
      enrollDeviceBtn.setOnClickListener { enrollDeviceClick() }
} }

  private fun enrollDeviceClick() {
    try{
      deviceController.updateDevice(
        fabricId.text.toString().toULong().toLong(),
        deviceId.text.toString().toULong().toLong()
      )
//      showMessage("Address update started")
    } catch (ex: Exception) {
//      showMessage("Address update failed: $ex")
    }
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      commandStatusTv.text = msg
    }
  }



  override fun onStart() {
    super.onStart()
    // TODO: use the fabric ID that was used to commission the device
    val testFabricId = "5544332211"
    fabricId.setText(testFabricId)
    deviceId.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
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

  companion object {
    private const val TAG = "ClusterInteractionFragment"
    fun newInstance(): ClusterInteractionFragment = ClusterInteractionFragment()
  }
}