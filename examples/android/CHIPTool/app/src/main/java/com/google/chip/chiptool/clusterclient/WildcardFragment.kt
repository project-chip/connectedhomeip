package com.google.chip.chiptool.clusterclient

import android.app.AlertDialog
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import android.widget.TextView
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ChipIdLookup
import chip.devicecontroller.InvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.NodeState
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvWriter
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.WildcardFragmentBinding
import com.google.protobuf.ByteString
import java.lang.StringBuilder
import java.util.Optional
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

  private val writeAttributeCallback = object : WriteAttributesCallback {
    override fun onError(attributePath: ChipAttributePath?, ex: Exception?) {
      Log.e(TAG, "Report error for $attributePath: $ex")
    }

    override fun onResponse(attributePath: ChipAttributePath?) {
      val text = "$attributePath : Write Success"
      requireActivity().runOnUiThread { binding.outputTv.text = text }
    }

    override fun onDone() {
      Log.i(TAG, "write attribute Done")
    }
  }

  private val invokeCallback = object : InvokeCallback {
    override fun onError(e: java.lang.Exception?) {
      Log.e(TAG, "Report error", e)
    }

    override fun onResponse(invokeElement: InvokeElement?, successCode: Long) {
      val text = "Invoke Response : $invokeElement, $successCode"
      requireActivity().runOnUiThread { binding.outputTv.text = text }
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
    binding.writeBtn.setOnClickListener { scope.launch { showWriteDialog() } }
    binding.subscribeEventBtn.setOnClickListener { scope.launch { showSubscribeDialog(EVENT) } }
    binding.readEventBtn.setOnClickListener { scope.launch { showReadDialog(EVENT) } }
    binding.invokeBtn.setOnClickListener { scope.launch { showInvokeDialog() } }

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

  private suspend fun subscribe(type: Int, minInterval: Int, maxInterval: Int, keepSubscriptions: Boolean, isFabricFiltered: Boolean, isUrgent: Boolean) {
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
      val eventPath = ChipEventPath.newInstance(endpointId, clusterId, eventId, isUrgent)
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

  private suspend fun write(writeValueType: String, writeValue: String, dataVersion: Int?, timedRequestTimeoutMs: Int, imTimeoutMs: Int) {
    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(binding.attributeIdEd.text.toString())
    val tlvWriter = TlvWriter()
    val values = writeValue.split(",")

    if (values.size > 1) tlvWriter.startArray(AnonymousTag)
    for (value in values) {
      try {
        TLV_MAP[writeValueType]?.generate(tlvWriter, value.trim())
      } catch (ex: Exception) {
        Log.e(TAG, "Invalid Data Type", ex)
        return
      }
    }
    if (values.size > 1) tlvWriter.endArray()

    val version = if (dataVersion == null) { Optional.empty() } else { Optional.of(dataVersion) }

    val writeRequest = AttributeWriteRequest.newInstance(endpointId, clusterId, attributeId, tlvWriter.getEncoded(), version)
    deviceController.write(writeAttributeCallback,
                      ChipClient.getConnectedDevicePointer(requireContext(),
                      addressUpdateFragment.deviceId),
                      listOf(writeRequest),
                      timedRequestTimeoutMs,
                      imTimeoutMs)
  }

  private suspend fun invoke(invokeField: String, timedRequestTimeoutMs: Int, imTimeoutMs: Int) {
    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val commandId = getChipPathIdForText(binding.commandIdEd.text.toString())

    val tlvWriter = TlvWriter()
    val fields = if (invokeField.isEmpty()) { listOf() } else { invokeField.split(",") }
    var count = 0
    tlvWriter.startStructure(AnonymousTag)
    for (field in fields) {
      try {
        val type = field.split(":")[0]
        val value = field.split(":")[1]

        Log.d(TAG, "value : $type - $value")
        TLV_MAP[type]?.generate(tlvWriter, value.trim(), ContextSpecificTag(count++))
      } catch (ex: Exception) {
        Log.e(TAG, "Invalid value", ex)
        return
      }
    }
    tlvWriter.endStructure()
    val invokeElement = InvokeElement.newInstance(endpointId, clusterId, commandId, tlvWriter.getEncoded(), null)
    deviceController.invoke(invokeCallback,
            ChipClient.getConnectedDevicePointer(requireContext(),
                    addressUpdateFragment.deviceId),
            invokeElement,
            timedRequestTimeoutMs,
            imTimeoutMs)
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

  private fun showWriteDialog() {
    binding.outputTv.text = ""
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.write_dialog, null)
    val writeValueTypeSp = dialogView.findViewById<Spinner>(R.id.writeValueTypeSp)
    val spinnerAdapter = ArrayAdapter(requireActivity(), android.R.layout.simple_spinner_item, TLV_MAP.keys.toList())
    writeValueTypeSp.adapter = spinnerAdapter
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    dialogView.findViewById<Button>(R.id.writeBtn).setOnClickListener {
      val writeValue = dialogView.findViewById<EditText>(R.id.writeValueEd).text.toString()
      val dataVersion = dialogView.findViewById<EditText>(R.id.dataVersionEd).text.toString()
      val timedRequestTimeoutMs = dialogView.findViewById<EditText>(R.id.timedRequestTimeoutEd).text.toString()
      val imTimeout = dialogView.findViewById<EditText>(R.id.imTimeoutEd).text.toString()
      scope.launch {
        write(  writeValueTypeSp.selectedItem.toString(),
                writeValue,
                if (dataVersion.isEmpty()) { null } else { dataVersion.toInt() },
                if (timedRequestTimeoutMs.isEmpty()) { 0 } else { timedRequestTimeoutMs.toInt() },
                if (imTimeout.isEmpty()) { 0 } else { imTimeout.toInt() } )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun showSubscribeDialog(type: Int) {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val isUrgentTv = dialogView.findViewById<TextView>(R.id.titleisUrgent)
    val isUrgentSp = dialogView.findViewById<Spinner>(R.id.isUrgentSp)
    if (type == EVENT) {
      isUrgentTv.visibility = View.VISIBLE
      isUrgentSp.visibility = View.VISIBLE
    } else {
      isUrgentTv.visibility = View.GONE
      isUrgentSp.visibility = View.GONE
    }
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
            isFabricFilteredSp.selectedItem.toString().toBoolean(),
            isUrgentSp.selectedItem.toString().toBoolean(),
          )
        } else {
          Log.e(TAG, "minInterval or maxInterval is empty!" )
        }
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun showInvokeDialog() {
    binding.outputTv.text = ""
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.invoke_dialog, null)
    val invokeValueEd = dialogView.findViewById<EditText>(R.id.invokeValueEd)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    dialogView.findViewById<Button>(R.id.invokeBtn).setOnClickListener {
      val invokeValue = invokeValueEd.text.toString()
      val timedRequestTimeoutMs = dialogView.findViewById<EditText>(R.id.timedRequestTimeoutEd).text.toString()
      val imTimeout = dialogView.findViewById<EditText>(R.id.imTimeoutEd).text.toString()
      scope.launch {
        invoke(invokeValue,
                if (timedRequestTimeoutMs.isEmpty()) { 0 } else { timedRequestTimeoutMs.toInt() },
                if (imTimeout.isEmpty()) { 0 } else { imTimeout.toInt() } )
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun getChipPathIdForText(text: String): ChipPathId {
    return if (text.isEmpty()) ChipPathId.forWildcard() else ChipPathId.forId(text.toLong())
  }

  interface TlvWriterInterface {
    fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag = AnonymousTag)
  }

  companion object {
    private const val TAG = "WildcardFragment"
    private const val ATTRIBUTE = 1
    private const val EVENT = 2
    fun newInstance(): WildcardFragment = WildcardFragment()

    private val TLV_MAP = mapOf(
            "UnsignedInt" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                writer.put(tag, value.toUInt())
              }
            },
            "Int" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                writer.put(tag, value.toInt())
              }
            },
            "Boolean" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                writer.put(tag, value.toBoolean())
              }
            },
            "Float" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                writer.put(tag, value.toFloat())
              }
            },
            "Double" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                writer.put(tag, value.toDouble())
              }
            },
            "String" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                writer.put(tag, value)
              }
            },
            "ByteArray(Hex)" to object:TlvWriterInterface {
              override fun generate(writer : TlvWriter, value: String, tag: chip.tlv.Tag) {
                val byteStringValue = ByteString.fromHex(value)
                writer.put(tag, byteStringValue)
              }
            },
    )
  }
}