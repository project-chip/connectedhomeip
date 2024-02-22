package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.EditorInfo
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping.*
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.NodeState
import chip.devicecontroller.model.Status
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.BasicClientFragmentBinding
import com.google.chip.chiptool.util.toAny
import java.util.Optional
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class BasicClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: BasicClientFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = BasicClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment
    binding.writeNodeLabelBtn.setOnClickListener {
      scope.launch {
        // TODO : Need to be implement poj-to-tlv
        sendWriteAttribute(
          BasicInformation.Attribute.NodeLabel,
          TlvWriter().put(AnonymousTag, binding.nodeLabelEd.text.toString()).getEncoded()
        )
        binding.nodeLabelEd.onEditorAction(EditorInfo.IME_ACTION_DONE)
      }
    }
    binding.writeLocationBtn.setOnClickListener {
      scope.launch {
        // TODO : Need to be implement poj-to-tlv
        sendWriteAttribute(
          BasicInformation.Attribute.Location,
          TlvWriter().put(AnonymousTag, binding.locationEd.text.toString()).getEncoded()
        )
        binding.locationEd.onEditorAction(EditorInfo.IME_ACTION_DONE)
      }
    }
    binding.writeLocalConfigDisabledSwitch.setOnCheckedChangeListener { _, isChecked ->
      scope.launch {
        // TODO : Need to be implement poj-to-tlv
        sendWriteAttribute(
          BasicInformation.Attribute.LocalConfigDisabled,
          TlvWriter().put(AnonymousTag, isChecked).getEncoded()
        )
      }
    }
    makeAttributeList()
    binding.attributeNameSpinner.adapter = makeAttributeNamesAdapter()
    binding.readAttributeBtn.setOnClickListener { scope.launch { readAttributeButtonClick() } }

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

  private fun makeAttributeNamesAdapter(): ArrayAdapter<String> {
    return ArrayAdapter(
        requireContext(),
        android.R.layout.simple_spinner_dropdown_item,
        ATTRIBUTES.toList()
      )
      .apply { setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item) }
  }

  private suspend fun readAttributeButtonClick() {
    try {
      readBasicClusters(binding.attributeNameSpinner.selectedItemPosition)
    } catch (ex: Exception) {
      showMessage("readBasicCluster failed: $ex")
    }
  }

  private suspend fun readBasicClusters(itemIndex: Int) {
    val endpointId = addressUpdateFragment.endpointId
    val clusterId = BasicInformation.ID
    val attributeName = ATTRIBUTES[itemIndex]
    val attributeId = BasicInformation.Attribute.valueOf(attributeName).id

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
            Log.i(TAG, "[Read Success] $attributeName: $value")
            showMessage("[Read Success] $attributeName: $value")
          }
        },
        devicePtr,
        listOf(ChipAttributePath.newInstance(endpointId, clusterId, attributeId)),
        null,
        false,
        0 /* imTimeoutMs */
      )
  }

  private fun makeAttributeList() {
    for (attribute in BasicInformation.Attribute.values()) {
      ATTRIBUTES.add(attribute.name)
    }
  }

  private suspend fun sendWriteAttribute(attribute: BasicInformation.Attribute, tlv: ByteArray) {
    val clusterId = BasicInformation.ID
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    ChipClient.getDeviceController(requireContext())
      .write(
        object : WriteAttributesCallback {
          override fun onError(attributePath: ChipAttributePath?, ex: java.lang.Exception?) {
            showMessage("Write ${attribute.name} failure $ex")
            Log.e(TAG, "Write ${attribute.name} failure", ex)
          }

          override fun onResponse(attributePath: ChipAttributePath, status: Status) {
            showMessage("Write ${attribute.name} response: $status")
          }
        },
        devicePtr,
        listOf(
          AttributeWriteRequest.newInstance(
            addressUpdateFragment.endpointId,
            clusterId,
            attribute.id,
            tlv,
            Optional.empty()
          )
        ),
        0,
        0
      )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.basicClusterCommandStatus.text = msg }
  }

  override fun onResume() {
    super.onResume()
    addressUpdateFragment.endpointId = ENDPOINT
  }

  companion object {
    private const val TAG = "BasicClientFragment"
    private const val ENDPOINT = 0
    private val ATTRIBUTES: MutableList<String> = mutableListOf()

    fun newInstance(): BasicClientFragment = BasicClientFragment()
  }
}
