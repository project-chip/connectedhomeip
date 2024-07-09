package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping.OperationalCredentials
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OpCredClientFragmentBinding
import com.google.chip.chiptool.util.toAny
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OpCredClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: OpCredClientFragmentBinding? = null
  private val binding
    get() = _binding!!

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

    binding.readSupportedFabricBtn.setOnClickListener {
      scope.launch { readClusterAttribute(OperationalCredentials.Attribute.SupportedFabrics) }
    }
    binding.readCommissionedFabricBtn.setOnClickListener {
      scope.launch { readClusterAttribute(OperationalCredentials.Attribute.CommissionedFabrics) }
    }
    binding.readFabricsBtn.setOnClickListener {
      scope.launch { readClusterAttribute(OperationalCredentials.Attribute.Fabrics) }
    }
    binding.removeFabricsBtn.setOnClickListener {
      scope.launch { sendRemoveFabricsBtnClick(binding.fabricIndexEd.text.toString().toUInt()) }
    }

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Long) {
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

  private suspend fun readClusterAttribute(attribute: OperationalCredentials.Attribute) {
    val endpointId = addressUpdateFragment.endpointId
    val clusterId = OperationalCredentials.ID
    val attributeName = attribute.name
    val attributeId = attribute.id

    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    ChipClient.getDeviceController(requireContext())
      .readPath(
        object : ReportCallback {
          override fun onError(
            attributePath: ChipAttributePath?,
            eventPath: ChipEventPath?,
            ex: java.lang.Exception
          ) {
            showMessage("Read $attributeName failure $ex")
            Log.e(TAG, "Read $attributeName failure", ex)
          }

          override fun onReport(nodeState: NodeState?) {
            val tlv =
              nodeState
                ?.getEndpointState(endpointId)
                ?.getClusterState(clusterId)
                ?.getAttributeState(attributeId)
                ?.tlv

            val value = tlv?.let { TlvReader(it).toAny() }
            Log.i(TAG, "OpCred $attributeName value: $value")
            showMessage("OpCred $attributeName value: $value")
          }
        },
        devicePtr,
        listOf(ChipAttributePath.newInstance(endpointId, clusterId, attributeId)),
        null,
        false,
        0 /* imTimeoutMs */
      )
  }

  private suspend fun sendRemoveFabricsBtnClick(fabricIndex: UInt) {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    // TODO : Need to be implement poj-to-tlv
    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.put(
      ContextSpecificTag(OperationalCredentials.RemoveFabricCommandField.FabricIndex.id),
      fabricIndex
    )
    tlvWriter.endStructure()
    val invokeElement =
      InvokeElement.newInstance(
        addressUpdateFragment.endpointId,
        OperationalCredentials.ID,
        OperationalCredentials.Command.RemoveFabric.id,
        tlvWriter.getEncoded(),
        null
      )

    deviceController.invoke(
      object : InvokeCallback {
        override fun onError(ex: Exception?) {
          showMessage("RemoveFabric failure $ex")
          Log.e(TAG, "RemoveFabric failure", ex)
        }

        override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
          Log.e(TAG, "onResponse : $invokeElement, Code : $successCode")
          showMessage("RemoveFabric success")
        }
      },
      devicePtr,
      invokeElement,
      0,
      0
    )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.opCredClusterCommandStatus.text = msg }
  }

  override fun onResume() {
    super.onResume()
    addressUpdateFragment.endpointId = OPERATIONAL_CREDENTIALS_ENDPOINT_ID
  }

  companion object {
    private const val TAG = "OpCredClientFragment"
    private const val OPERATIONAL_CREDENTIALS_ENDPOINT_ID = 0

    fun newInstance(): OpCredClientFragment = OpCredClientFragment()
  }
}
