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
import android.widget.Toast
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
import chip.tlv.TlvReader
import chip.tlv.TlvWriter
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.WildcardFragmentBinding
import com.google.protobuf.ByteString
import java.lang.StringBuilder
import java.util.Optional
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

class WildcardFragment : Fragment() {
  private var _binding: WildcardFragmentBinding? = null
  private val binding get() = _binding!!

  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private val attributePath = ArrayList<ChipAttributePath>()
  private val eventPath = ArrayList<ChipEventPath>()
  private val subscribeIdList = ArrayList<ULong>()

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

    binding.selectTypeRadioGroup.setOnCheckedChangeListener { _, i ->
      val readBtnOn = (i == R.id.readRadioBtn)
      val subscribeBtnOn = (i == R.id.subscribeRadioBtn)
      val writeBtnOn = (i == R.id.writeRadioBtn)
      val invokeBtnOn = (i == R.id.invokeRadioBtn)

      binding.addLayout.visibility = getVisibility(readBtnOn || subscribeBtnOn)
      binding.attributeIdLabel.visibility = getVisibility(readBtnOn || subscribeBtnOn || writeBtnOn)
      binding.attributeIdEd.visibility = getVisibility(readBtnOn || subscribeBtnOn || writeBtnOn)
      binding.eventIdLabel.visibility = getVisibility(readBtnOn || subscribeBtnOn)
      binding.eventIdEd.visibility = getVisibility(readBtnOn || subscribeBtnOn)
      binding.commandIdLabel.visibility = getVisibility(invokeBtnOn)
      binding.commandIdEd.visibility = getVisibility(invokeBtnOn)
      binding.isUrgentLabel.visibility = getVisibility(subscribeBtnOn)
      binding.isUrgentSp.visibility = getVisibility(subscribeBtnOn)
      binding.shutdownSubscriptionBtn.visibility = getVisibility(subscribeBtnOn)
    }

    binding.sendBtn.setOnClickListener {
      if (binding.readRadioBtn.isChecked) {
        showReadDialog()
      } else if (binding.subscribeRadioBtn.isChecked) {
        showSubscribeDialog()
      } else if (binding.writeRadioBtn.isChecked) {
        showWriteDialog()
      } else if (binding.invokeRadioBtn.isChecked) {
        showInvokeDialog()
      }
    }

    binding.shutdownSubscriptionBtn.setOnClickListener { showShutdownSubscriptionDialog() }

    binding.addAttributeBtn.setOnClickListener { addPathList(ATTRIBUTE) }
    binding.addEventBtn.setOnClickListener { addPathList(EVENT) }
    binding.resetBtn.setOnClickListener { resetPath() }

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    return binding.root
  }

  private fun getVisibility(isShowing: Boolean) : Int {
    return if (isShowing) { View.VISIBLE } else { View.GONE }
  }

  private fun addPathList(type: Int) {
    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(binding.attributeIdEd.text.toString())
    val eventId = getChipPathIdForText(binding.eventIdEd.text.toString())
    // Only Subscribe used
    val isUrgent = (binding.subscribeRadioBtn.isChecked) && (binding.isUrgentSp.selectedItem.toString().toBoolean())

    if (type == ATTRIBUTE) {
      attributePath.add(ChipAttributePath.newInstance(endpointId, clusterId, attributeId))
    } else if (type == EVENT) {
      eventPath.add(ChipEventPath.newInstance(endpointId, clusterId, eventId, isUrgent))
    }
    updateAddListView()
  }

  private fun resetPath() {
    attributePath.clear()
    eventPath.clear()
    updateAddListView()
  }

  private fun updateAddListView() {
    val builder = StringBuilder()
    for (attribute in attributePath) {
      builder.append("attribute($attribute)\n")
    }
    for (event in eventPath) {
      builder.append("event($event)\n")
    }
    binding.sendListView.text = builder.toString()
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

  private suspend fun subscribe(minInterval: Int, maxInterval: Int, keepSubscriptions: Boolean, isFabricFiltered: Boolean) {
    val subscriptionEstablishedCallback =
      SubscriptionEstablishedCallback {
        subscriptionId ->
        Log.i(TAG, "Subscription to device established : ${subscriptionId.toULong()}")
        subscribeIdList.add(subscriptionId.toULong())
        requireActivity().runOnUiThread {
          Toast.makeText(requireActivity(), "${getString(R.string.wildcard_subscribe_established_toast_message)} : $subscriptionId", Toast.LENGTH_SHORT).show()
        }
      }

    val resubscriptionAttemptCallback =
      ResubscriptionAttemptCallback { terminationCause, nextResubscribeIntervalMsec
                                     -> Log.i(TAG, "ResubscriptionAttempt terminationCause:$terminationCause, nextResubscribeIntervalMsec:$nextResubscribeIntervalMsec") }

    deviceController.subscribeToPath(subscriptionEstablishedCallback,
            resubscriptionAttemptCallback,
            reportCallback,
            ChipClient.getConnectedDevicePointer(requireContext(),
                    addressUpdateFragment.deviceId),
            attributePath.ifEmpty { null },
            eventPath.ifEmpty { null },
            minInterval,
            maxInterval,
            keepSubscriptions,
            isFabricFiltered,
            /* imTimeoutMs= */ 0)
  }

  private suspend fun read(isFabricFiltered: Boolean) {
    deviceController.readPath(reportCallback,
            ChipClient.getConnectedDevicePointer(requireContext(),
                    addressUpdateFragment.deviceId),
            attributePath.ifEmpty { null },
            eventPath.ifEmpty { null },
            isFabricFiltered,
            /* imTimeoutMs= */ 0)
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

  private fun showReadDialog() {
    if (attributePath.isEmpty() && eventPath.isEmpty()) {
      requireActivity().runOnUiThread {
        Toast.makeText(requireActivity(), R.string.wildcard_empty_error_toast_message, Toast.LENGTH_SHORT).show()
      }
      return
    }
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.read_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    val isFabricFilteredEd = dialogView.findViewById<EditText>(R.id.isFabricFilteredSp)
    dialogView.findViewById<Button>(R.id.readBtn).setOnClickListener {
      scope.launch {
        read(isFabricFilteredEd.text.toString().toBoolean())
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun showWriteDialog() {
    binding.outputTv.text = ""
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.write_dialog, null)
    val writeValueTypeSp = dialogView.findViewById<Spinner>(R.id.writeValueTypeSp)
    val spinnerAdapter = ArrayAdapter(requireActivity(), android.R.layout.simple_spinner_dropdown_item, TLV_MAP.keys.toList())
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

  private fun showSubscribeDialog() {
    if (attributePath.isEmpty() && eventPath.isEmpty()) {
      requireActivity().runOnUiThread {
        Toast.makeText(requireActivity(), R.string.wildcard_empty_error_toast_message, Toast.LENGTH_SHORT).show()
      }
      return
    }
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
            minIntervalEd.text.toString().toInt(),
            maxIntervalEd.text.toString().toInt(),
            keepSubscriptionsSp.selectedItem.toString().toBoolean(),
            isFabricFilteredSp.selectedItem.toString().toBoolean(),
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

  private suspend fun readCurrentFabricIndex() : UInt {
    val context = requireContext()
    val endpointId = 0L
    val clusterId = 62L // OperationalCredentials
    val attributeId = 5L // CurrentFabricIndex
    val deviceId = addressUpdateFragment.deviceId
    val devicePointer = ChipClient.getConnectedDevicePointer(context, deviceId)
    return suspendCoroutine { cont ->
      deviceController.readAttributePath(object : ReportCallback {
        override fun onError(attributePath: ChipAttributePath?, eventPath: ChipEventPath?, e: java.lang.Exception?) {
          cont.resume(0u)
        }

        override fun onReport(nodeState: NodeState?) {
          val state = nodeState?.getEndpointState(endpointId.toInt())?.
                                  getClusterState(clusterId)?.
                                  getAttributeState(attributeId)
          if (state == null) {
            cont.resume(0u)
            return
          }
          val ret = TlvReader(state.tlv).getUInt(AnonymousTag)
          cont.resume(ret)
        }
      }, devicePointer,
         listOf(ChipAttributePath.newInstance(endpointId, clusterId, attributeId)),
         0 /* imTimeoutMs */
      )
    }
  }

  private fun shutdownSubscription(fabricIndex: UInt, subscribeId: ULong? = null) {
    val deviceId = addressUpdateFragment.deviceId
    if (subscribeId != null) {
      deviceController.shutdownSubscriptions(fabricIndex.toInt(), deviceId, subscribeId.toLong())
      subscribeIdList.remove(subscribeId)
    } else {
      deviceController.shutdownSubscriptions(fabricIndex.toInt(), deviceId)
    }
  }

  private fun showShutdownSubscriptionDialog() {
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.shutdown_subscribe_dialog, null)
    val subscriptionIdSp = dialogView.findViewById<Spinner>(R.id.subscribeIdSp)
    val fabricIndexTv = dialogView.findViewById<TextView>(R.id.fabricIndexValue)
    val shutdownBtn = dialogView.findViewById<Button>(R.id.shutdownBtn)
    val shutdownAllBtn = dialogView.findViewById<Button>(R.id.shutdownAllBtn)
    val spinnerAdapter = ArrayAdapter(requireActivity(), android.R.layout.simple_spinner_dropdown_item, subscribeIdList)
    subscriptionIdSp.adapter = spinnerAdapter
    val dialog = AlertDialog.Builder(requireContext()).apply {
      setView(dialogView)
    }.create()

    shutdownBtn.setOnClickListener {
      val fabricIndex = fabricIndexTv.text.toString().toUInt()
      val subscribeId = subscriptionIdSp.selectedItem.toString().toULong()
      scope.launch {
        shutdownSubscription(fabricIndex, subscribeId)
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }

    shutdownAllBtn.setOnClickListener {
      scope.launch {
        val fabricIndex = fabricIndexTv.text.toString().toUInt()
        shutdownSubscription(fabricIndex)
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()

    scope.launch {
      val fabricIndex = readCurrentFabricIndex()
      requireActivity().runOnUiThread {
        fabricIndexTv.text = fabricIndex.toString()
        shutdownBtn.isEnabled = true
        shutdownAllBtn.isEnabled = true
      }
    }
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