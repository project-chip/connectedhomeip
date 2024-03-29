package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.SeekBar
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterIDMapping.*
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.OnOffClientFragmentBinding
import com.google.chip.chiptool.util.toAny
import java.text.SimpleDateFormat
import java.util.Calendar
import java.util.Locale
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class OnOffClientFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private var _binding: OnOffClientFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = OnOffClientFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    deviceController.setCompletionListener(ChipControllerCallback())

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    binding.onBtn.setOnClickListener { scope.launch { sendOnOffClusterCommand(OnOff.Command.On) } }
    binding.offBtn.setOnClickListener {
      scope.launch { sendOnOffClusterCommand(OnOff.Command.Off) }
    }
    binding.toggleBtn.setOnClickListener {
      scope.launch { sendOnOffClusterCommand(OnOff.Command.Toggle) }
    }
    binding.readBtn.setOnClickListener { scope.launch { sendReadOnOffClick() } }
    binding.showSubscribeDialogBtn.setOnClickListener { showSubscribeDialog() }

    binding.levelBar.setOnSeekBarChangeListener(
      object : SeekBar.OnSeekBarChangeListener {
        override fun onProgressChanged(seekBar: SeekBar, i: Int, b: Boolean) {}

        override fun onStartTrackingTouch(seekBar: SeekBar?) {}

        override fun onStopTrackingTouch(seekBar: SeekBar?) {
          Toast.makeText(
              requireContext(),
              "Level is: " + binding.levelBar.progress,
              Toast.LENGTH_SHORT
            )
            .show()
          scope.launch { sendLevelCommandClick() }
        }
      }
    )

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private suspend fun sendReadOnOffClick() {
    val endpointId = addressUpdateFragment.endpointId
    val clusterId = OnOff.ID
    val attributeId = OnOff.Attribute.OnOff.id

    val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)

    val devicePointer =
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
            Log.e(TAG, "Error reading onOff attribute", ex)
          }

          override fun onReport(nodeState: NodeState?) {
            val tlv =
              nodeState
                ?.getEndpointState(endpointId)
                ?.getClusterState(clusterId)
                ?.getAttributeState(attributeId)
                ?.tlv
            val value = tlv?.let { TlvReader(it).toAny() }
            Log.v(TAG, "On/Off attribute value: $value")
            showMessage("On/Off attribute value: $value")
          }
        },
        devicePointer,
        listOf(attributePath),
        null,
        false,
        0 /* imTimeoutMs */
      )
  }

  private fun showSubscribeDialog() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    dialogView.findViewById<Button>(R.id.subscribeBtn).setOnClickListener {
      scope.launch {
        sendSubscribeOnOffClick(
          minIntervalEd.text.toString().toInt(),
          maxIntervalEd.text.toString().toInt()
        )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun sendSubscribeOnOffClick(minInterval: Int, maxInterval: Int) {
    val endpointId = addressUpdateFragment.endpointId
    val clusterId = OnOff.ID
    val attributeId = OnOff.Attribute.OnOff.id

    val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)

    val subscriptionEstablishedCallback = SubscriptionEstablishedCallback { subscriptionId ->
      Log.i(TAG, "Subscription to device established : ${subscriptionId.toULong()}")
      requireActivity().runOnUiThread {
        Toast.makeText(
            requireActivity(),
            "${getString(R.string.wildcard_subscribe_established_toast_message)} : $subscriptionId",
            Toast.LENGTH_SHORT
          )
          .show()
      }
    }

    val resubscriptionAttemptCallback =
      ResubscriptionAttemptCallback { terminationCause, nextResubscribeIntervalMsec ->
        Log.i(
          TAG,
          "ResubscriptionAttempt terminationCause:$terminationCause, nextResubscribeIntervalMsec:$nextResubscribeIntervalMsec"
        )
      }

    val devicePointer =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    deviceController.subscribeToPath(
      subscriptionEstablishedCallback,
      resubscriptionAttemptCallback,
      object : ReportCallback {
        override fun onError(
          attributePath: ChipAttributePath?,
          eventPath: ChipEventPath?,
          ex: Exception
        ) {
          Log.e(TAG, "Error configuring on/off attribute", ex)
        }

        override fun onReport(nodeState: NodeState?) {
          val tlv =
            nodeState
              ?.getEndpointState(endpointId)
              ?.getClusterState(clusterId)
              ?.getAttributeState(attributeId)
              ?.tlv
              ?: return
          // TODO : Need to be implement poj-to-tlv
          val value = TlvReader(tlv).getBool(AnonymousTag)
          val formatter = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
          val time = formatter.format(Calendar.getInstance(Locale.getDefault()).time)
          val message = "Subscribed on/off value at $time: ${if (value) "ON" else "OFF"}"

          Log.v(TAG, message)
          showReportMessage(message)
        }
      },
      devicePointer,
      listOf(attributePath),
      null,
      minInterval,
      maxInterval,
      false,
      false,
      /* imTimeoutMs= */ 0
    )
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Long) {
      Log.d(TAG, "onCommissioningComplete for nodeId $nodeId: $errorCode")
      showMessage("Address update complete for nodeId $nodeId with code $errorCode")
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

  private suspend fun sendLevelCommandClick() {
    // TODO : Need to be implement poj-to-tlv
    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.put(
      ContextSpecificTag(LevelControl.MoveToLevelCommandField.Level.id),
      binding.levelBar.progress.toUInt()
    )
    tlvWriter.put(ContextSpecificTag(LevelControl.MoveToLevelCommandField.OptionsMask.id), 0u)
    tlvWriter.put(ContextSpecificTag(LevelControl.MoveToLevelCommandField.OptionsOverride.id), 0u)
    tlvWriter.put(ContextSpecificTag(LevelControl.MoveToLevelCommandField.TransitionTime.id), 0u)
    tlvWriter.endStructure()

    val invokeElement =
      InvokeElement.newInstance(
        addressUpdateFragment.endpointId,
        LevelControl.ID,
        LevelControl.Command.MoveToLevel.id,
        tlvWriter.getEncoded(),
        null
      )

    val devicePointer =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    deviceController.invoke(
      object : InvokeCallback {
        override fun onError(ex: Exception?) {
          showMessage("MoveToLevel command failure $ex")
          Log.e(TAG, "MoveToLevel command failure", ex)
        }

        override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
          Log.e(TAG, "onResponse : $invokeElement, Code : $successCode")
          showMessage("MoveToLevel command success")
        }
      },
      devicePointer,
      invokeElement,
      0,
      0
    )
  }

  private suspend fun sendOnOffClusterCommand(commandId: OnOff.Command) {
    // TODO : Need to be implement poj-to-tlv
    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()
    val invokeElement =
      InvokeElement.newInstance(
        addressUpdateFragment.endpointId,
        OnOff.ID,
        commandId.id,
        tlvWriter.getEncoded(),
        null
      )

    val devicePointer =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return
      }

    deviceController.invoke(
      object : InvokeCallback {
        override fun onError(ex: Exception?) {
          showMessage("${commandId.name} command failure $ex")
          Log.e(TAG, "${commandId.name} command failure", ex)
        }

        override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
          Log.e(TAG, "onResponse : $invokeElement, Code : $successCode")
          showMessage("${commandId.name} command success")
        }
      },
      devicePointer,
      invokeElement,
      0,
      0
    )
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread { binding.commandStatusTv.text = msg }
  }

  private fun showReportMessage(msg: String) {
    requireActivity().runOnUiThread { binding.reportStatusTv.text = msg }
  }

  override fun onResume() {
    super.onResume()
    addressUpdateFragment.endpointId = ON_OFF_CLUSTER_ENDPOINT
  }

  companion object {
    private const val TAG = "OnOffClientFragment"

    private const val ON_OFF_CLUSTER_ENDPOINT = 1
    private const val LEVEL_CONTROL_CLUSTER_ENDPOINT = 1

    fun newInstance(): OnOffClientFragment = OnOffClientFragment()
  }
}
