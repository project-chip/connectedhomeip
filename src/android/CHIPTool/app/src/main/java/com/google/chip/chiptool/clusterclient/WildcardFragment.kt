package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.EditText
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipIdLookup
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ReportEventCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.NodeState
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import java.lang.StringBuilder
import kotlinx.android.synthetic.main.wildcard_fragment.attributeIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.clusterIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.endpointIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.eventIdEd
import kotlinx.android.synthetic.main.wildcard_fragment.outputTv
import kotlinx.android.synthetic.main.wildcard_fragment.view.readBtn
import kotlinx.android.synthetic.main.wildcard_fragment.view.readEventBtn
import kotlinx.android.synthetic.main.wildcard_fragment.view.subscribeBtn
import kotlinx.android.synthetic.main.wildcard_fragment.view.subscribeEventBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class WildcardFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private val reportCallback = object : ReportCallback {
    override fun onError(attributePath: ChipAttributePath, ex: Exception) {
      Log.e(TAG, "Report error for $attributePath: $ex")
    }

    override fun onReport(nodeState: NodeState) {
      Log.i(TAG, "Received wildcard report")

      val debugString = nodeStateToDebugString(nodeState)
      Log.i(TAG, debugString)
      requireActivity().runOnUiThread { outputTv.text = debugString }
    }

    override fun onDone() {
      Log.i(TAG, "wildcard report Done")
    }
  }

  private val reportEventCallback = object : ReportEventCallback {
    override fun onError(eventPath: ChipEventPath, ex: Exception) {
      Log.e(TAG, "Report error for $eventPath: $ex")
    }

    override fun onReport(nodeState: NodeState) {
      Log.i(TAG, "Received wildcard report")

      val debugString = nodeStateToDebugString(nodeState)
      Log.i(TAG, debugString)
      requireActivity().runOnUiThread { outputTv.text = debugString }
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
    scope = viewLifecycleOwner.lifecycleScope
    return inflater.inflate(R.layout.wildcard_fragment, container, false).apply {
      subscribeBtn.setOnClickListener { scope.launch { showSubscribeDialog(ATTRIBUTE) } }
      readBtn.setOnClickListener { scope.launch { read(ATTRIBUTE) } }
      subscribeEventBtn.setOnClickListener { scope.launch { showSubscribeDialog(EVENT) } }
      readEventBtn.setOnClickListener { scope.launch { read(EVENT) } }

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment
    }
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
        clusterState.eventStates.forEach { (eventId, eventState) ->
          stringBuilder.append("\t\teventNumber: ${eventState.eventNumber}\n")
          stringBuilder.append("\t\tpriorityLevel: ${eventState.priorityLevel}\n")
          stringBuilder.append("\t\tsystemTimeStamp: ${eventState.systemTimeStamp}\n")

          val eventName = ChipIdLookup.eventIdToName(clusterId, eventId)
          stringBuilder.append("\t\t$eventName: ${eventState.value}\n")
        }
        stringBuilder.append("\t}\n")
      }
      stringBuilder.append("}\n")
    }
    return stringBuilder.toString()
  }

  private suspend fun subscribe(type: Int, minInterval: Int, maxInterval: Int) {
    val subscriptionEstablishedCallback =
      SubscriptionEstablishedCallback { Log.i(TAG, "Subscription to device established") }

    val resubscriptionAttemptCallback =
      ResubscriptionAttemptCallback { terminationCause, nextResubscribeIntervalMsec
                                     -> Log.i(TAG, "ResubscriptionAttempt terminationCause:$terminationCause, nextResubscribeIntervalMsec:$nextResubscribeIntervalMsec") }

    val endpointId = getChipPathIdForText(endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(attributeIdEd.text.toString())
    val eventId = getChipPathIdForText(eventIdEd.text.toString())

    if (type == ATTRIBUTE) {
      val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
      deviceController.subscribeToPath(subscriptionEstablishedCallback,
                                       reportCallback,
                                       ChipClient.getConnectedDevicePointer(requireContext(),
                                       addressUpdateFragment.deviceId),
                                       listOf(attributePath),
                                       minInterval,
                                       maxInterval)
    } else if (type == EVENT) {
      val eventPath = ChipEventPath.newInstance(endpointId, clusterId, eventId)
      deviceController.subscribeToEventPath(subscriptionEstablishedCallback,
                                      resubscriptionAttemptCallback,
                                      reportEventCallback,
                                      ChipClient.getConnectedDevicePointer(requireContext(),
                                      addressUpdateFragment.deviceId),
                                      listOf(eventPath),
                                      minInterval,
                                      maxInterval)
    }
  }

  private suspend fun read(type: Int) {
    val endpointId = getChipPathIdForText(endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(attributeIdEd.text.toString())
    val eventId = getChipPathIdForText(eventIdEd.text.toString())

    if (type == ATTRIBUTE) {
      val attributePath = ChipAttributePath.newInstance(endpointId, clusterId, attributeId)
      deviceController.readPath(reportCallback,
                          ChipClient.getConnectedDevicePointer(requireContext(),
                          addressUpdateFragment.deviceId),
                          listOf(attributePath))
    } else if (type == EVENT) {
      val eventPath = ChipEventPath.newInstance(endpointId, clusterId, eventId)
      deviceController.readEventPath(reportEventCallback,
                          ChipClient.getConnectedDevicePointer(requireContext(),
                          addressUpdateFragment.deviceId),
                          listOf(eventPath))
    }
  }

  private fun showSubscribeDialog(type: Int) {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    dialogView.findViewById<Button>(R.id.subscribeBtn).setOnClickListener {
      scope.launch {
        subscribe(type, minIntervalEd.text.toString().toInt(), maxIntervalEd.text.toString().toInt())
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