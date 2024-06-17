package com.google.chip.chiptool.clusterclient

import android.content.Context
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipClusterException
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ICDClientInfo
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.databinding.AddressUpdateFragmentBinding
import com.google.chip.chiptool.util.DeviceIdUtil
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

/** Fragment for updating the address of a device given its fabric and node ID. */
class AddressUpdateFragment : ICDCheckInCallback, Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  val deviceId: Long
    get() = binding.deviceIdEd.text.toString().toULong(16).toLong()

  var endpointId: Int
    get() = binding.epIdEd.text.toString().toInt()
    set(value) {
      binding.epIdEd.setText(value.toString())
    }

  private var _binding: AddressUpdateFragmentBinding? = null
  private val binding
    get() = _binding!!

  private lateinit var scope: CoroutineScope

  private var icdDeviceId: Long = 0L
  private var icdTotalRemainStayActiveTimeMs = 0L
  private var icdDeviceRemainStayActiveTimeMs = 0L
  private var isSendingStayActiveCommand = false
  private val icdRequestActiveDurationMs: Long
    get() = binding.icdActiveDurationEd.text.toString().toLong() * 1000

  private val handler = Handler(Looper.getMainLooper())

  private var externalICDCheckInMessageCallback: ICDCheckInMessageCallback? = null

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = AddressUpdateFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope
    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    super.onViewCreated(view, savedInstanceState)

    ChipClient.setICDCheckInCallback(this)

    val compressedFabricId = deviceController.compressedFabricId
    binding.fabricIdEd.setText(compressedFabricId.toULong().toString(16).padStart(16, '0'))
    binding.deviceIdEd.setText(DeviceIdUtil.getLastDeviceId(requireContext()).toString(16))
    binding.epIdEd.setText(endpointId.toString())
    binding.icdActiveDurationEd.setText((ICD_STAY_ACTIVE_DURATION / 1000).toString())

    binding.icdInteractionSwitch.setOnClickListener {
      val isChecked = binding.icdInteractionSwitch.isChecked
      if (updateUIForICDInteractionSwitch(isChecked)) {
        icdInteractionSwitchClick(isChecked)
      }
    }

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

  private fun icdInteractionSwitchClick(isEnabled: Boolean) {
    if (isEnabled) {
      icdDeviceId = deviceId
    } else {
      icdDeviceId = 0
      if (icdDeviceRemainStayActiveTimeMs != 0L || icdTotalRemainStayActiveTimeMs != 0L) {
        scope.launch {
          icdDeviceRemainStayActiveTimeMs = sendStayActive(0L)
          icdTotalRemainStayActiveTimeMs = icdDeviceRemainStayActiveTimeMs
        }
      }
    }
  }

  private suspend fun sendStayActive(duration: Long): Long {
    isSendingStayActiveCommand = true
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showToastMessage("Get DevicePointer fail!")
        throw e
      }

    val cluster = ChipClusters.IcdManagementCluster(devicePtr, 0)
    val retDuration = suspendCoroutine { cont ->
      cluster.stayActiveRequest(
        object : ChipClusters.IcdManagementCluster.StayActiveResponseCallback {
          override fun onError(error: Exception) {
            Log.d(TAG, "onError", error)
            cont.resume(0L)
          }

          override fun onSuccess(promisedActiveDuration: Long) {
            cont.resume(promisedActiveDuration)
          }
        },
        duration
      )
    }
    isSendingStayActiveCommand = false
    return retDuration
  }

  private fun updateUIForICDInteractionSwitch(isEnabled: Boolean): Boolean {
    val isICD = isICDDevice()
    if (isEnabled && !isICD) {
      binding.icdInteractionSwitch.isChecked = false
      return false
    }

    return true
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

  private fun showToastMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireActivity(), msg, Toast.LENGTH_SHORT).show()
    }
  }

  fun isGroupId(): Boolean {
    return isGroupNodeId(getNodeId())
  }

  fun getGroupId(): UInt {
    return getGroupIdFromNodeId(getNodeId())
  }

  fun getNodeId(): ULong {
    return binding.deviceIdEd.text.toString().toULong(16)
  }

  fun isICDDevice(): Boolean {
    return deviceController.icdClientInfo.firstOrNull { info -> info.peerNodeId == deviceId } !=
      null
  }

  override fun notifyCheckInMessage(info: ICDClientInfo) {
    externalICDCheckInMessageCallback?.notifyCheckInMessage()
    if (info.peerNodeId != icdDeviceId) {
      return
    }

    scope.launch {
      try {
        icdDeviceRemainStayActiveTimeMs = sendStayActive(icdRequestActiveDurationMs)
        icdTotalRemainStayActiveTimeMs = icdRequestActiveDurationMs
        turnOnActiveMode()
      } catch (e: IllegalStateException) {
        Log.d(TAG, "IlligalStateException", e)
      } catch (e: ChipClusterException) {
        Log.d(TAG, "ChipClusterException", e)
      }
    }
  }

  fun setNotifyCheckInMessageCallback(callback: ICDCheckInMessageCallback?) {
    externalICDCheckInMessageCallback = callback
  }

  interface ICDCheckInMessageCallback {
    fun notifyCheckInMessage()
  }

  private fun turnOnActiveMode() {
    requireActivity().runOnUiThread {
      binding.icdProgressBar.max = (icdTotalRemainStayActiveTimeMs / 1000).toInt()
      binding.icdProgressBar.progress = (icdTotalRemainStayActiveTimeMs / 1000).toInt()
    }

    val runnable =
      object : Runnable {
        override fun run() {
          if (!isAdded) {
            Log.d(TAG, "Fragment is not attached")
            return
          }
          if (icdTotalRemainStayActiveTimeMs >= ICD_PROGRESS_STEP) {
            icdDeviceRemainStayActiveTimeMs -= ICD_PROGRESS_STEP
            icdTotalRemainStayActiveTimeMs -= ICD_PROGRESS_STEP
            handler.postDelayed(this, ICD_PROGRESS_STEP)
            requireActivity().runOnUiThread {
              binding.icdProgressBar.progress = (icdTotalRemainStayActiveTimeMs / 1000).toInt()
            }

            if (
              !isSendingStayActiveCommand &&
                (ICD_RESEND_STAY_ACTIVE_TIME > icdDeviceRemainStayActiveTimeMs) &&
                (ICD_RESEND_STAY_ACTIVE_TIME < icdTotalRemainStayActiveTimeMs)
            )
              scope.launch {
                icdDeviceRemainStayActiveTimeMs = sendStayActive(icdTotalRemainStayActiveTimeMs)
              }
          } else {
            requireActivity().runOnUiThread { binding.icdProgressBar.progress = 0 }
          }
        }
      }
    handler.post(runnable)
  }

  companion object {
    private const val TAG = "AddressUpdateFragment"
    // Refer from NodeId.h (src/lib/core/NodeId.h)
    private const val MIN_GROUP_NODE_ID = 0xFFFF_FFFF_FFFF_0000UL
    private const val MASK_GROUP_ID = 0x0000_0000_0000_FFFFUL

    private const val ICD_STAY_ACTIVE_DURATION = 30000L // 30 secs.
    private const val ICD_PROGRESS_STEP = 1000L // 1 sec.
    private const val ICD_RESEND_STAY_ACTIVE_TIME = 2000L // 2 secs.

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

interface ICDCheckInCallback {
  fun notifyCheckInMessage(info: ICDClientInfo)
}
