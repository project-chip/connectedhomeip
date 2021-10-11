package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipClusters.BasicCluster
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.basic_client_fragment.*
import kotlinx.android.synthetic.main.basic_client_fragment.view.*
import kotlinx.android.synthetic.main.on_off_client_fragment.*
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class BasicClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.basic_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      basicClusterUpdateAddressBtn.setOnClickListener { scope.launch { updateAddressClick() }}
      readUserLabelBtn.setOnClickListener { scope.launch { sendReadUserLabelCommandClick() }}
      writeUserLabelBtn.setOnClickListener { scope.launch { sendWriteUserLabelCommandClick() }}
      readProductNameBtn.setOnClickListener { scope.launch { sendReadProductNameCommandClick() }}
    }
  }

  override fun onStart() {
    super.onStart()
    val compressedFabricId = deviceController.compressedFabricId
    basicClusterFabricIdEd.setText(compressedFabricId.toULong().toString(16).padStart(16,'0'))
    basicClusterDeviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
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

  private fun updateAddressClick() {
    try {
      deviceController.updateDevice(
        basicClusterFabricIdEd.text.toString().toULong(16).toLong(),
        basicClusterDeviceIdEd.text.toString().toULong().toLong()
      )
      showMessage("Address update started")
    } catch (ex: Exception) {
      showMessage("Address update failed: $ex")
    }
  }

  private suspend fun sendReadProductNameCommandClick() {
    getBasicClusterForDevice().readProductNameAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        showMessage("Read ProductName command success: value: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read ProductName command failure $ex")
        Log.e(TAG, "Read ProductName command failure", ex)
      }
    })
  }

  private suspend fun sendReadUserLabelCommandClick() {
    getBasicClusterForDevice().readUserLabelAttribute(object : ChipClusters.CharStringAttributeCallback {
      override fun onSuccess(value: String) {
        showMessage("Read UserLabel command success: value: $value")
      }

      override fun onError(ex: Exception) {
        showMessage("Read UserLabel command failure $ex")
        Log.e(TAG, "Read UserLabel command failure", ex)
      }
    })
  }

  private suspend fun sendWriteUserLabelCommandClick() {
    getBasicClusterForDevice().writeUserLabelAttribute(object : ChipClusters.DefaultClusterCallback {
      override fun onSuccess() {
        showMessage("Write UserLabel command success")
      }

      override fun onError(ex: Exception) {
        showMessage("Write UserLabel command failure $ex")
        Log.e(TAG, "Write UserLabel command failure", ex)
      }

    }, userLabelEd.text.toString())
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      basicClusterCommandStatus.text = msg
    }
  }

  private suspend fun getBasicClusterForDevice(): BasicCluster {
    return BasicCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), basicClusterDeviceIdEd.text.toString().toLong()), 0
    )
  }

  companion object {
    private const val TAG = "BasicClientFragment"
    fun newInstance(): BasicClientFragment = BasicClientFragment()
  }
}
