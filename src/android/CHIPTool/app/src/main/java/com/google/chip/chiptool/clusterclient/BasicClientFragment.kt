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
import kotlinx.android.synthetic.main.basic_client_fragment.basicClusterCommandStatus
import kotlinx.android.synthetic.main.basic_client_fragment.userLabelEd
import kotlinx.android.synthetic.main.basic_client_fragment.view.readProductNameBtn
import kotlinx.android.synthetic.main.basic_client_fragment.view.readUserLabelBtn
import kotlinx.android.synthetic.main.basic_client_fragment.view.writeUserLabelBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class BasicClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.basic_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

      readUserLabelBtn.setOnClickListener { scope.launch { sendReadUserLabelCommandClick() }}
      writeUserLabelBtn.setOnClickListener { scope.launch { sendWriteUserLabelCommandClick() }}
      readProductNameBtn.setOnClickListener { scope.launch { sendReadProductNameCommandClick() }}
    }
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
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId), 0
    )
  }

  companion object {
    private const val TAG = "BasicClientFragment"
    fun newInstance(): BasicClientFragment = BasicClientFragment()
  }
}
