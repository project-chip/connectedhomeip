package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipIdLookup
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.WildcardFragmentBinding
import java.lang.StringBuilder
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class WildcardFragment : Fragment() {
  private var _binding: WildcardFragmentBinding? = null
  private val binding get() = _binding!!

  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private val reportCallback = object : ReportCallback {
    override fun onError(attributePath: ChipAttributePath?, eventPath: ChipEventPath?, ex: Exception) {
      if (attributePath != null)
      {
        Log.e(TAG, "Report error for $attributePath: $ex")
      }
      if (eventPath != null)
      {
        Log.e(TAG, "Report error for $eventPath: $ex")
      }
    }

    override fun onReport(nodeState: NodeState) {
      Log.i(TAG, "Received wildcard report")

      val debugString = nodeStateToDebugString(nodeState)
      Log.i(TAG, debugString)
      requireActivity().runOnUiThread { binding.outputTv.text = debugString }
    }

    override fun onDone() {
      Log.i(TAG, "wildcard report Done")
    }
  }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View {
    _binding = WildcardFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope
    binding.subscribeBtn.setOnClickListener { scope.launch { showSubscribeDialog(ATTRIBUTE) } }
    binding.readBtn.setOnClickListener { scope.launch { showReadDialog(ATTRIBUTE) } }
    binding.subscribeEventBtn.setOnClickListener { scope.launch { showSubscribeDialog(EVENT) } }
    binding.readEventBtn.setOnClickListener { scope.launch { showReadDialog(EVENT) } }

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun nodeStateToDebugString(nodeState: NodeState): String {
    val stringBuilder = StringBuilder()
    nodeState.endpointStates.forEach { (endpointId, endpointState) ->
      stringBuilder.append("Endpoint $endpointId: {\n")
      endpointState.clusterStates.forEach { (clusterId, clusterState) ->
        stringBuilder.append("\t${ChipIdLookup.clusterIdToName(clusterId)}Cluster: {\n")
        clusterState.attributeStates.forEach { (attributeId, attributeState) ->
          val attributeName = ChipIdLookup.attributeIdToName(clusterId, attributeId)
          stringBuilder.append("\t\t$attributeName: ${attributeState.value}\n")
        }
        clusterState.eventStates.forEach { (eventId, events) ->
          for (event in events)
          {
            stringBuilder.append("\t\teventNumber: ${event.eventNumber}\n")
            stringBuilder.append("\t\tpriorityLevel: ${event.priorityLevel}\n")
            stringBuilder.append("\t\tsystemTimeStamp: ${event.systemTimeStamp}\n")

            val eventName = ChipIdLookup.eventIdToName(clusterId, eventId)
            stringBuilder.append("\t\t$eventName: ${event.value}\n")
          }
        }
        stringBuilder.append("\t}\n")
      }
      stringBuilder.append("}\n")
    }
    return stringBuilder.toString()
  }

  private suspend fun subscribe(type: Int, minInterval: Int, maxInterval: Int, keepSubscriptions: Boolean, isFabricFiltered: Boolean) {
    val subscriptionEstablishedCallback =
      SubscriptionEstablishedCallback { Log.i(TAG, "Subscription to device established") }

    val resubscriptionAttemptCallback =
      ResubscriptionAttemptCallback { terminationCause, nextResubscribeIntervalMsec
                                     -> Log.i(TAG, "ResubscriptionAttempt terminationCause:$terminationCause, nextResubscribeIntervalMsec:$nextResubscribeIntervalMsec") }

    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(binding.attributeIdEd.text.toString())
    val eventId = getChipPathIdForText(binding.eventIdEd.text.toString())

    if (type == ATTRIBUTE) {
      val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
      deviceController.subscribeToPath(subscriptionEstablishedCallback,
                                       resubscriptionAttemptCallback,
                                       reportCallback,
                                       ChipClient.getConnectedDevicePointer(requireContext(),
                                       addressUpdateFragment.deviceId),
                                       listOf(attributePath),
                                       null,
                                       minInterval,
                                       maxInterval,
                                       keepSubscriptions,
                                       isFabricFiltered)
    } else if (type == EVENT) {
      val eventPath = ChipEventPath.newInstance(endpointId, clusterId, eventId)
      deviceController.subscribeToPath(subscriptionEstablishedCallback,
                                      resubscriptionAttemptCallback,
                                      reportCallback,
                                      ChipClient.getConnectedDevicePointer(requireContext(),
                                      addressUpdateFragment.deviceId),
                                      null,
                                      listOf(eventPath),
                                      minInterval,
                                      maxInterval,
                                      keepSubscriptions,
                                      isFabricFiltered)
    }
  }

  private suspend fun read(type: Int, isFabricFiltered: Boolean) {
    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(binding.attributeIdEd.text.toString())
    val eventId = getChipPathIdForText(binding.eventIdEd.text.toString())

    if (type == ATTRIBUTE) {
      val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
      deviceController.readPath(reportCallback,
                          ChipClient.getConnectedDevicePointer(requireContext(),
                          addressUpdateFragment.deviceId),
                          listOf(attributePath),
                          null,
                          isFabricFiltered)
    } else if (type == EVENT) {
      val eventPath = ChipEventPath.newInstance(endpointId, clusterId, eventId)
      deviceController.readPath(reportCallback,
                          ChipClient.getConnectedDevicePointer(requireContext(),
                          addressUpdateFragment.deviceId),
                          null,
                          listOf(eventPath),
                          isFabricFiltered)
    }
  }

  private fun showReadDialog(type: Int) {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.read_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val isFabricFilteredEd = dialogView.findViewById<EditText>(R.id.isFabricFilteredSp)
    dialogView.findViewById<Button>(R.id.readBtn).setOnClickListener {
      scope.launch {
        read(type, isFabricFilteredEd.text.toString().toBoolean())
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun showSubscribeDialog(type: Int) {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    val keepSubscriptionsSp = dialogView.findViewById<Spinner>(R.id.keepSubscriptionsSp)
    val isFabricFilteredSp = dialogView.findViewById<Spinner>(R.id.isFabricFilteredSp)
    dialogView.findViewById<Button>(R.id.subscribeBtn).setOnClickListener {
      scope.launch {
        if(minIntervalEd.text.isNotBlank() && maxIntervalEd.text.isNotBlank()) {
          subscribe(
            type,
            minIntervalEd.text.toString().toInt(),
            maxIntervalEd.text.toString().toInt(),
            keepSubscriptionsSp.selectedItem.toString().toBoolean(),
            isFabricFilteredSp.selectedItem.toString().toBoolean()
          )
        } else {
          Log.e(TAG, "minInterval or maxInterval is empty!" )
        }
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun getChipPathIdForText(text: String): ChipPathId {
    return if (text.isEmpty()) ChipPathId.forWildcard() else ChipPathId.forId(text.toLong())
  }

  companion object {
    private const val TAG = "WildcardFragment"
    private const val ATTRIBUTE = 1
    private const val EVENT = 2
    fun newInstance(): WildcardFragment = WildcardFragment()
  }
}