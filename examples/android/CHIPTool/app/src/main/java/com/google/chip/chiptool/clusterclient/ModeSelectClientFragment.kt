package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping
import chip.devicecontroller.ClusterIDMapping.ModeSelect
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.cluster.structs.ModeSelectClusterModeOptionStruct
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.NodeState
import chip.devicecontroller.model.Status
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.ModeSelectFragmentBinding
import java.util.Optional
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ModeSelectClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: ModeSelectFragmentBinding? = null

  private val startUpMode: UInt
    get() = binding.startUpModeEd.text.toString().toUIntOrNull() ?: 0U

  private val onMode: UInt
    get() = binding.onModeEd.text.toString().toUIntOrNull() ?: 0U

  private val currentMode: Int
    get() = binding.supportedModesSp.selectedItem.toString().split("-")[0].toInt()

  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ModeSelectFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.readAttributeBtn.setOnClickListener { scope.launch { readAttributeBtnClick() } }
    binding.changeToModeBtn.setOnClickListener { scope.launch { changeToModeBtnClick() } }
    binding.onModeWriteBtn.setOnClickListener {
      scope.launch { writeAttributeBtnClick(ClusterIDMapping.ModeSelect.Attribute.OnMode, onMode) }
    }
    binding.startUpModeWriteBtn.setOnClickListener {
      scope.launch {
        writeAttributeBtnClick(ClusterIDMapping.ModeSelect.Attribute.StartUpMode, startUpMode)
      }
    }

    return binding.root
  }

  private suspend fun readAttributeBtnClick() {
    val endpointId = addressUpdateFragment.endpointId
    val clusterId = ModeSelect.ID
    val attributeId = ChipPathId.forWildcard().id
    val path = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    deviceController.readAttributePath(
      object : ReportCallback {
        override fun onError(
          attributePath: ChipAttributePath?,
          eventPath: ChipEventPath?,
          e: Exception
        ) {
          requireActivity().runOnUiThread {
            Toast.makeText(
                requireActivity(),
                R.string.ota_provider_invalid_attribute,
                Toast.LENGTH_SHORT
              )
              .show()
          }
        }

        override fun onReport(nodeState: NodeState?) {
          val attributeStates =
            nodeState?.getEndpointState(endpointId)?.getClusterState(clusterId)?.attributeStates
              ?: return

          requireActivity().runOnUiThread {
            val description = attributeStates[ClusterIDMapping.ModeSelect.Attribute.Description.id]
            binding.descriptionEd.setText(description?.value.toString())

            val standardNamespace =
              attributeStates[ClusterIDMapping.ModeSelect.Attribute.StandardNamespace.id]
            binding.standardNamespaceEd.setText(standardNamespace?.value.toString())

            val currentMode = attributeStates[ClusterIDMapping.ModeSelect.Attribute.CurrentMode.id]
            binding.currentModeEd.setText(currentMode?.value.toString())

            val startUpMode = attributeStates[ClusterIDMapping.ModeSelect.Attribute.StartUpMode.id]
            val startUpModeVisibility =
              if (startUpMode != null) {
                binding.startUpModeEd.setText(startUpMode.value.toString())
                View.VISIBLE
              } else {
                View.GONE
              }
            binding.startUpModeEd.visibility = startUpModeVisibility
            binding.startUpModeTv.visibility = startUpModeVisibility
            binding.startUpModeWriteBtn.visibility = startUpModeVisibility

            val onMode = attributeStates[ClusterIDMapping.ModeSelect.Attribute.OnMode.id]
            val onModeVisibility =
              if (onMode != null) {
                binding.onModeEd.setText(onMode.value.toString())
                View.VISIBLE
              } else {
                View.GONE
              }
            binding.onModeEd.visibility = onModeVisibility
            binding.onModeTv.visibility = onModeVisibility
            binding.onModeWriteBtn.visibility = onModeVisibility

            val supportedModesTlv =
              attributeStates[ClusterIDMapping.ModeSelect.Attribute.SupportedModes.id]?.tlv
            if (supportedModesTlv != null) {
              var pos = 0
              var currentItemId = 0
              val modeOptionStructList: List<ModeSelectClusterModeOptionStruct>
              TlvReader(supportedModesTlv).also {
                modeOptionStructList = buildList {
                  it.enterArray(AnonymousTag)
                  while (!it.isEndOfContainer()) {
                    val struct = ModeSelectClusterModeOptionStruct.fromTlv(AnonymousTag, it)
                    add(struct)
                    if (
                      currentMode != null && struct.mode == currentMode.value.toString().toUInt()
                    ) {
                      currentItemId = pos
                    }
                    pos++
                  }
                  it.exitContainer()
                }
                binding.supportedModesSp.adapter =
                  ArrayAdapter(
                    requireContext(),
                    android.R.layout.simple_spinner_dropdown_item,
                    modeOptionStructList.map { it.show() }
                  )
                binding.supportedModesSp.setSelection(currentItemId)
                binding.currentModeEd.setText(binding.supportedModesSp.selectedItem.toString())
              }
            }
          }
        }
      },
      devicePtr,
      listOf<ChipAttributePath>(path),
      0
    )
  }

  private suspend fun changeToModeBtnClick() {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    ChipClusters.ModeSelectCluster(devicePtr, addressUpdateFragment.endpointId)
      .changeToMode(
        object : ChipClusters.DefaultClusterCallback {
          override fun onError(error: java.lang.Exception?) {
            Log.d(TAG, "onError", error)
            showMessage("Error : ${error.toString()}")
          }

          override fun onSuccess() {
            showMessage("Change Success")
            scope.launch { readAttributeBtnClick() }
          }
        },
        currentMode
      )
  }

  private suspend fun writeAttributeBtnClick(attribute: ModeSelect.Attribute, value: UInt) {
    val clusterId = ModeSelect.ID
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }
    deviceController.write(
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
          TlvWriter().put(AnonymousTag, value).getEncoded(),
          Optional.empty()
        )
      ),
      0,
      0
    )
  }

  private fun ModeSelectClusterModeOptionStruct.show(): String {
    val value = this
    return StringBuilder()
      .apply {
        append("${value.mode}-${value.label}")
        append("[")
        for (semanticTag in value.semanticTags) {
          append("${semanticTag.value}:${semanticTag.mfgCode}")
          append(",")
        }
        append("]")
      }
      .toString()
  }

  override fun onDestroyView() {
    super.onDestroyView()
    deviceController.finishOTAProvider()
    _binding = null
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.commandStatusTv.text = msg }
  }

  companion object {
    private const val TAG = "ModeSelectClientFragment"

    fun newInstance(): ModeSelectClientFragment = ModeSelectClientFragment()
  }
}
