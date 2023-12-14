package com.google.chip.chiptool.clusterclient

import android.content.Context
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import androidx.fragment.app.Fragment
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.databinding.AddressUpdateFragmentBinding
import com.google.chip.chiptool.util.DeviceIdUtil

/** Fragment for updating the address of a device given its fabric and node ID. */
class AddressUpdateFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  val deviceId: Long
    get() = binding.deviceIdEd.text.toString().toULong().toLong()

  var endpointId: Int
    get() = binding.epIdEd.text.toString().toInt()
    set(value) {
      binding.epIdEd.setText(value.toString())
    }

  private var _binding: AddressUpdateFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = AddressUpdateFragmentBinding.inflate(inflater, container, false)
    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    val compressedFabricId = deviceController.compressedFabricId
    binding.fabricIdEd.setText(compressedFabricId.toULong().toString().padStart(16, '0'))
    binding.deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString(16))
    binding.epIdEd.setText(endpointId.toString())

    updateDeviceIdSpinner()
  }

  fun updateDeviceIdSpinner() {
    val deviceIdList = DeviceIdUtil.getCommissionedNodeId(requireContext())
    binding.deviceIdSpinner.adapter =
      ArrayAdapter(requireContext(), android.R.layout.simple_spinner_dropdown_item, deviceIdList)
    binding.deviceIdSpinner.onItemSelectedListener =
      object : AdapterView.OnItemSelectedListener {
        override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
          binding.deviceIdEd.setText(deviceIdList[position].toULong(16).toString())
        }

        override fun onNothingSelected(parent: AdapterView<*>?) {
          Log.d(TAG, "onNothingSelected")
        }
      }
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  suspend fun getDevicePointer(context: Context): Long {
    return if (isGroupId()) {
      deviceController.getGroupDevicePointer(getGroupId().toInt())
    } else {
      ChipClient.getConnectedDevicePointer(context, getNodeId().toLong())
    }
  }

  fun isGroupId(): Boolean {
    return isGroupNodeId(getNodeId())
  }

  fun getGroupId(): UInt {
    return getGroupIdFromNodeId(getNodeId())
  }

  fun getNodeId(): ULong {
    return binding.deviceIdEd.text.toString().toULong()
  }

  companion object {
    private const val TAG = "AddressUpdateFragment"
    // Refer from NodeId.h (src/lib/core/NodeId.h)
    private const val MIN_GROUP_NODE_ID = 0xFFFF_FFFF_FFFF_0000UL
    private const val MASK_GROUP_ID = 0x0000_0000_0000_FFFFUL

    fun isGroupNodeId(nodeId: ULong): Boolean {
      return nodeId >= MIN_GROUP_NODE_ID
    }

    fun getNodeIdFromGroupId(groupId: UInt): ULong {
      return groupId.toULong() or MIN_GROUP_NODE_ID
    }

    fun getGroupIdFromNodeId(nodeId: ULong): UInt {
      return (nodeId and MASK_GROUP_ID).toUInt()
    }
  }
}
