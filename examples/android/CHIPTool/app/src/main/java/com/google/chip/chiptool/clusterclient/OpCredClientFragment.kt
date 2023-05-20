package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OpCredClientFragmentBinding
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class OpCredClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: OpCredClientFragmentBinding? = null
  private val binding get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = OpCredClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.readSupportedFabricBtn.setOnClickListener { scope.launch { sendReadOpCredSupportedFabricAttrClick() } }
    binding.readCommissionedFabricBtn.setOnClickListener { scope.launch { sendReadOpCredCommissionedFabricAttrClick() } }

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
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
      binding.opCredClusterCommandStatus.text = msg
    }
  }

  private suspend fun getOpCredClusterForDevice(): ChipClusters.OperationalCredentialsCluster {
    return ChipClusters.OperationalCredentialsCluster(
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId), 0
    )
  }

  companion object {
    private const val TAG = "OpCredClientFragment"
    fun newInstance(): OpCredClientFragment = OpCredClientFragment()
  }
}
