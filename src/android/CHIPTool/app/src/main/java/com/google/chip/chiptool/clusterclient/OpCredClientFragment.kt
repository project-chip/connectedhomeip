package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlinx.android.synthetic.main.op_cred_client_fragment.*
import kotlinx.android.synthetic.main.op_cred_client_fragment.view.*
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

class OpCredClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.op_cred_client_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())

      opCredClusterUpdateAddressBtn.setOnClickListener { scope.launch { updateAddressClick() } }
      readSupportedFabricBtn.setOnClickListener { scope.launch { sendReadOpCredSupportedFabricAttrClick() } }
      readCommissionedFabricBtn.setOnClickListener { scope.launch { sendReadOpCredCommissionedFabricAttrClick() } }
    }
  }

  override fun onStart() {
    super.onStart()
    // TODO: use the fabric ID that was used to commission the device
    val testFabricId = "5544332211"
    opCredClusterFabricIdEd.setText(testFabricId)
    opCredClusterDeviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString())
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
              opCredClusterFabricIdEd.text.toString().toULong().toLong(),
              opCredClusterDeviceIdEd.text.toString().toULong().toLong()
      )
      showMessage("Address update started")
    } catch (ex: Exception) {
      showMessage("Address update failed: $ex")
    }
  }

  private suspend fun sendReadOpCredSupportedFabricAttrClick() {
    getOpCredClusterForDevice().readSupportedFabricsAttribute(object : ChipClusters.IntegerAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.v(TAG, "OpCred supported Fabric attribute value: $value")
        showMessage("OpCred supported Fabric attribute value: $value")
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error reading OpCred supported Fabric attribute", ex)
      }
    })
  }

  private suspend fun sendReadOpCredCommissionedFabricAttrClick() {
    getOpCredClusterForDevice().readCommissionedFabricsAttribute(object : ChipClusters.IntegerAttributeCallback {
      override fun onSuccess(value: Int) {
        Log.v(TAG, "OpCred Commissioned Fabric attribute value: $value")
        showMessage("OpCred Commissioned Fabric attribute value: $value")
      }

      override fun onError(ex: Exception) {
        Log.e(TAG, "Error reading OpCred Commissioned Fabric attribute", ex)
      }
    })
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      opCredClusterCommandStatus.text = msg
    }
  }

  private suspend fun getOpCredClusterForDevice(): ChipClusters.OperationalCredentialsCluster {
    return ChipClusters.OperationalCredentialsCluster(
            ChipClient.getConnectedDevicePointer(requireContext(), opCredClusterDeviceIdEd.text.toString().toLong()), 0
    )
  }

  companion object {
    private const val TAG = "OpCredClientFragment"
    fun newInstance(): OpCredClientFragment = OpCredClientFragment()
  }
}
