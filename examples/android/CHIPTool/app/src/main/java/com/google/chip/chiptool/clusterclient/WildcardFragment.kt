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
import chip.devicecontroller.ExtendableInvokeCallback
import chip.devicecontroller.ReportCallback
import chip.devicecontroller.ResubscriptionAttemptCallback
import chip.devicecontroller.SubscriptionEstablishedCallback
import chip.devicecontroller.WriteAttributesCallback
import chip.devicecontroller.model.AttributeWriteRequest
import chip.devicecontroller.model.ChipAttributePath
import chip.devicecontroller.model.ChipEventPath
import chip.devicecontroller.model.ChipPathId
import chip.devicecontroller.model.InvokeElement
import chip.devicecontroller.model.InvokeResponseData
import chip.devicecontroller.model.NoInvokeResponseData
import chip.devicecontroller.model.NodeState
import chip.devicecontroller.model.Status
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.WildcardFragmentBinding
import com.google.chip.chiptool.util.toAny
import java.lang.StringBuilder
import java.util.Optional
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch
import matter.jsontlv.putJsonString
import matter.tlv.AnonymousTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class WildcardFragment : Fragment(), AddressUpdateFragment.ICDCheckInMessageCallback {
  private var _binding: WildcardFragmentBinding? = null
  private val binding
    get() = _binding!!

  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  private lateinit var addressUpdateFragment: AddressUpdateFragment

  private val attributePath = ArrayList<ChipAttributePath>()
  private val eventPath = ArrayList<ChipEventPath>()
  private val writePath = ArrayList<AttributeWriteRequest>()
  private val invokePath = ArrayList<InvokeElement>()
  private val subscribeIdList = ArrayList<ULong>()

  data class ReadICDConfig(val isFabricFiltered: Boolean, val eventMin: Long?)

  data class SubscribeICDConfig(
    val minInterval: Int,
    val maxInterval: Int,
    val keepSubscriptions: Boolean,
    val isFabricFiltered: Boolean,
    val eventMin: Long?
  )

  data class WriteInvokeICDConfig(val timedRequestTimeoutMs: Int, val imTimeoutMs: Int)

  private var readICDConfig: ReadICDConfig? = null
  private var subscribeICDConfig: SubscribeICDConfig? = null
  private var writeICDConfig: WriteInvokeICDConfig? = null
  private var invokeICDConfig: WriteInvokeICDConfig? = null

  private val reportCallback =
    object : ReportCallback {
      override fun onError(
        attributePath: ChipAttributePath?,
        eventPath: ChipEventPath?,
        ex: Exception
      ) {
        if (attributePath != null) {
          Log.e(TAG, "Report error for $attributePath: $ex")
        }
        if (eventPath != null) {
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

  private val writeAttributeCallback =
    object : WriteAttributesCallback {
      var viewText = ""

      override fun onError(attributePath: ChipAttributePath?, ex: Exception?) {
        Log.e(TAG, "Report error for $attributePath: $ex")
        viewText += "onError : $attributePath - $ex\n"
      }

      override fun onResponse(attributePath: ChipAttributePath, status: Status) {
        viewText += "$attributePath : Write response: $status\n"
      }

      override fun onDone() {
        requireActivity().runOnUiThread {
          binding.outputTv.text = viewText
          viewText = ""
        }
      }
    }

  private val extendableInvokeCallback =
    object : ExtendableInvokeCallback {
      var viewText = ""

      override fun onError(e: java.lang.Exception?) {
        viewText += "Invoke onError : $e\n"
        Log.e(TAG, "Report error", e)
      }

      override fun onResponse(invokeResponseData: InvokeResponseData?) {
        viewText += "Invoke Response : $invokeResponseData\n"
      }

      override fun onNoResponse(noInvokeResponseData: NoInvokeResponseData?) {
        viewText += "Invoke onNoResponse : $noInvokeResponseData\n"
      }

      override fun onDone() {
        requireActivity().runOnUiThread {
          binding.outputTv.text = viewText
          viewText = ""
        }
      }
    }

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?,
  ): View {
    _binding = WildcardFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope

    val writeTypeSpinnerAdapter =
      ArrayAdapter(
        requireActivity(),
        android.R.layout.simple_spinner_dropdown_item,
        TLV_MAP.keys.toList()
      )
    binding.writeValueTypeSp.adapter = writeTypeSpinnerAdapter

    binding.selectTypeRadioGroup.setOnCheckedChangeListener { _, radioBtnId ->
      setVisibilityEachView(radioBtnId)
    }

    binding.sendBtn.setOnClickListener { showDialog(isICDQueueBtn = false) }

    binding.shutdownSubscriptionBtn.setOnClickListener { showShutdownSubscriptionDialog() }

    binding.addAttributeBtn.setOnClickListener { addPathList(SendType.ATTRIBUTE) }
    binding.addEventBtn.setOnClickListener { addPathList(SendType.EVENT) }
    binding.addListBtn.setOnClickListener { addRequest() }
    binding.resetBtn.setOnClickListener { resetPath() }
    binding.writeInvokeresetBtn.setOnClickListener { resetPath() }
    binding.icdQueueBtn.setOnCheckedChangeListener { _, isChecked ->
      if (isChecked) {
        val isSetting = showDialog(isICDQueueBtn = true)
        if (!isSetting) {
          binding.icdQueueBtn.isChecked = false
        }
      } else {
        resetICDConfig()
      }
    }

    addressUpdateFragment =
      childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment

    return binding.root
  }

  override fun onResume() {
    super.onResume()
    addressUpdateFragment.setNotifyCheckInMessageCallback(this)
  }

  override fun onPause() {
    addressUpdateFragment.setNotifyCheckInMessageCallback(null)
    super.onPause()
  }

  override fun notifyCheckInMessage() {
    Log.d(TAG, "notifyCheckInMessage")
    scope.launch {
      if (attributePath.isNotEmpty() || eventPath.isNotEmpty()) {
        if (binding.readRadioBtn.isChecked && readICDConfig != null) {
          read(readICDConfig!!.isFabricFiltered, readICDConfig!!.eventMin)
        } else if (binding.subscribeRadioBtn.isChecked && subscribeICDConfig != null) {
          subscribe(
            subscribeICDConfig!!.minInterval,
            subscribeICDConfig!!.maxInterval,
            subscribeICDConfig!!.keepSubscriptions,
            subscribeICDConfig!!.isFabricFiltered,
            subscribeICDConfig!!.eventMin
          )
        }
      } else if (
        binding.writeRadioBtn.isChecked && writePath.isNotEmpty() && writeICDConfig != null
      ) {
        write(writeICDConfig!!.timedRequestTimeoutMs, writeICDConfig!!.imTimeoutMs)
      } else if (
        binding.invokeRadioBtn.isChecked && invokePath.isNotEmpty() && invokeICDConfig != null
      ) {
        invoke(invokeICDConfig!!.timedRequestTimeoutMs, invokeICDConfig!!.imTimeoutMs)
      }
      requireActivity().runOnUiThread { binding.icdQueueBtn.isChecked = false }
      resetICDConfig()
    }
  }

  private fun resetICDConfig() {
    readICDConfig = null
    subscribeICDConfig = null
    writeICDConfig = null
    invokeICDConfig = null
  }

  private fun setVisibilityEachView(radioBtnId: Int) {
    val readBtnOn = (radioBtnId == R.id.readRadioBtn)
    val subscribeBtnOn = (radioBtnId == R.id.subscribeRadioBtn)
    val writeBtnOn = (radioBtnId == R.id.writeRadioBtn)
    val invokeBtnOn = (radioBtnId == R.id.invokeRadioBtn)

    binding.addAttributeBtn.visibility = getVisibility(readBtnOn || subscribeBtnOn)
    binding.addEventBtn.visibility = getVisibility(readBtnOn || subscribeBtnOn)
    binding.resetBtn.visibility = getVisibility(readBtnOn || subscribeBtnOn)
    binding.attributeIdLabel.visibility = getVisibility(readBtnOn || subscribeBtnOn || writeBtnOn)
    binding.attributeIdEd.visibility = getVisibility(readBtnOn || subscribeBtnOn || writeBtnOn)
    binding.eventIdLabel.visibility = getVisibility(readBtnOn || subscribeBtnOn)
    binding.eventIdEd.visibility = getVisibility(readBtnOn || subscribeBtnOn)
    binding.addListBtn.visibility = getVisibility(writeBtnOn || invokeBtnOn)
    binding.commandIdLabel.visibility = getVisibility(invokeBtnOn)
    binding.commandIdEd.visibility = getVisibility(invokeBtnOn)
    binding.isUrgentLabel.visibility = getVisibility(subscribeBtnOn)
    binding.isUrgentSp.visibility = getVisibility(subscribeBtnOn)
    binding.writeValueLabel.visibility = getVisibility(writeBtnOn)
    binding.writeValueEd.visibility = getVisibility(writeBtnOn)
    binding.writeValueTypeLabel.visibility = getVisibility(writeBtnOn)
    binding.writeValueTypeSp.visibility = getVisibility(writeBtnOn)
    binding.dataVersionLabel.visibility = getVisibility(writeBtnOn)
    binding.dataVersionEd.visibility = getVisibility(writeBtnOn)
    binding.invokeValueLabel.visibility = getVisibility(invokeBtnOn)
    binding.invokeValueEd.visibility = getVisibility(invokeBtnOn)
    binding.shutdownSubscriptionBtn.visibility = getVisibility(subscribeBtnOn)
    binding.writeInvokeresetBtn.visibility = getVisibility(writeBtnOn || invokeBtnOn)

    resetPath()
  }

  private fun showDialog(isICDQueueBtn: Boolean): Boolean {
    var ret = false
    if (binding.readRadioBtn.isChecked) {
      ret = showReadDialog(isICDQueueBtn)
    } else if (binding.subscribeRadioBtn.isChecked) {
      ret = showSubscribeDialog(isICDQueueBtn)
    } else if (binding.writeRadioBtn.isChecked) {
      showWriteDialog(isICDQueueBtn)
      ret = true
    } else if (binding.invokeRadioBtn.isChecked) {
      showInvokeDialog(isICDQueueBtn)
      ret = true
    }
    return ret
  }

  private fun addRequest() {
    if (binding.writeRadioBtn.isChecked) {
      addWriteRequest()
    } else {
      addInvokeRequest()
    }
  }

  private fun getVisibility(isShowing: Boolean): Int {
    return if (isShowing) {
      View.VISIBLE
    } else {
      View.GONE
    }
  }

  private fun addPathList(type: SendType) {
    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(binding.attributeIdEd.text.toString())
    val eventId = getChipPathIdForText(binding.eventIdEd.text.toString())
    // Only Subscribe used
    val isUrgent =
      (binding.subscribeRadioBtn.isChecked) &&
        (binding.isUrgentSp.selectedItem.toString().toBoolean())

    if (type == SendType.ATTRIBUTE) {
      attributePath.add(ChipAttributePath.newInstance(endpointId, clusterId, attributeId))
    } else if (type == SendType.EVENT) {
      eventPath.add(ChipEventPath.newInstance(endpointId, clusterId, eventId, isUrgent))
    }
    updateAddListView()
  }

  private fun resetPath() {
    attributePath.clear()
    eventPath.clear()
    writePath.clear()
    invokePath.clear()
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
    for (write in writePath) {
      builder.append("WritePath($write)\n")
    }
    for (invoke in invokePath) {
      builder.append("InvokePath($invoke)\n")
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
          val tlv = attributeState.tlv
          stringBuilder.append("\t\t$attributeName: ${TlvReader(tlv).toAny()}\n")
        }
        clusterState.eventStates.forEach { (eventId, events) ->
          for (event in events) {
            stringBuilder.append("\t\teventNumber: ${event.eventNumber}\n")
            stringBuilder.append("\t\tpriorityLevel: ${event.priorityLevel}\n")
            stringBuilder.append("\t\ttimestampType: ${event.timestampType}\n")
            stringBuilder.append("\t\ttimestampValue: ${event.timestampValue}\n")

            val eventName = ChipIdLookup.eventIdToName(clusterId, eventId)
            val tlv = event.tlv
            stringBuilder.append("\t\t$eventName: ${TlvReader(tlv).toAny()}\n")
          }
        }
        stringBuilder.append("\t}\n")
      }
      stringBuilder.append("}\n")
    }
    return stringBuilder.toString()
  }

  private suspend fun subscribe(
    minInterval: Int,
    maxInterval: Int,
    keepSubscriptions: Boolean,
    isFabricFiltered: Boolean,
    eventMin: Long?
  ) {
    val subscriptionEstablishedCallback = SubscriptionEstablishedCallback { subscriptionId ->
      Log.i(TAG, "Subscription to device established : ${subscriptionId.toULong()}")
      subscribeIdList.add(subscriptionId.toULong())
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
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        return
      }
    deviceController.subscribeToPath(
      subscriptionEstablishedCallback,
      resubscriptionAttemptCallback,
      reportCallback,
      devicePtr,
      attributePath.ifEmpty { null },
      eventPath.ifEmpty { null },
      minInterval,
      maxInterval,
      keepSubscriptions,
      isFabricFiltered,
      /* imTimeoutMs= */ 0,
      eventMin
    )
  }

  private suspend fun read(isFabricFiltered: Boolean, eventMin: Long?) {
    val devicePtr =
      try {
        ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        return
      }
    deviceController.readPath(
      reportCallback,
      devicePtr,
      attributePath.ifEmpty { null },
      eventPath.ifEmpty { null },
      isFabricFiltered,
      /* imTimeoutMs= */ 0,
      eventMin
    )
  }

  private fun addWriteRequest() {
    val writeValue = binding.writeValueEd.text.toString()
    val writeValueType = binding.writeValueTypeSp.selectedItem.toString()
    val dataVersion = binding.dataVersionEd.text.toString()

    val endpointId =
      if (!addressUpdateFragment.isGroupId()) {
        getChipPathIdForText(binding.endpointIdEd.text.toString())
      } else {
        null
      }
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val attributeId = getChipPathIdForText(binding.attributeIdEd.text.toString())

    val version =
      if (dataVersion.isEmpty()) {
        Optional.empty()
      } else {
        Optional.of(dataVersion.toInt())
      }

    lateinit var writeRequest: AttributeWriteRequest

    if (writeValueType == "json") {
      writeRequest =
        AttributeWriteRequest.newInstance(endpointId, clusterId, attributeId, writeValue, version)
    } else {
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

      writeRequest =
        AttributeWriteRequest.newInstance(
          endpointId,
          clusterId,
          attributeId,
          tlvWriter.getEncoded(),
          version
        )
    }
    writePath.add(writeRequest)
    updateAddListView()
  }

  private fun addInvokeRequest() {
    val endpointId = getChipPathIdForText(binding.endpointIdEd.text.toString())
    val clusterId = getChipPathIdForText(binding.clusterIdEd.text.toString())
    val commandId = getChipPathIdForText(binding.commandIdEd.text.toString())
    val invokeJson = binding.invokeValueEd.text.toString()

    val jsonString = invokeJson.ifEmpty { "{}" }
    val invokeElement =
      if (addressUpdateFragment.isGroupId()) {
        InvokeElement.newGroupInstance(
          addressUpdateFragment.getGroupId().toInt(),
          clusterId,
          commandId,
          null,
          jsonString
        )
      } else {
        InvokeElement.newInstance(endpointId, clusterId, commandId, null, jsonString)
      }
    invokePath.add(invokeElement)
    updateAddListView()
  }

  private suspend fun write(timedRequestTimeoutMs: Int, imTimeoutMs: Int) {
    val devicePtr =
      try {
        addressUpdateFragment.getDevicePointer(requireContext())
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getDevicePointer exception", e)
        return
      }
    deviceController.write(
      writeAttributeCallback,
      devicePtr,
      writePath,
      timedRequestTimeoutMs,
      imTimeoutMs
    )
  }

  private suspend fun invoke(timedRequestTimeoutMs: Int, imTimeoutMs: Int) {
    val devicePtr =
      try {
        addressUpdateFragment.getDevicePointer(requireContext())
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getDevicePointer exception", e)
        return
      }
    deviceController.extendableInvoke(
      extendableInvokeCallback,
      devicePtr,
      invokePath,
      timedRequestTimeoutMs,
      imTimeoutMs
    )
  }

  private fun showReadDialog(isICDQueueBtn: Boolean): Boolean {
    if (attributePath.isEmpty() && eventPath.isEmpty()) {
      requireActivity().runOnUiThread {
        Toast.makeText(
            requireActivity(),
            R.string.wildcard_empty_error_toast_message,
            Toast.LENGTH_SHORT
          )
          .show()
      }
      return false
    }
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.read_dialog, null)
    val eventMinEd = dialogView.findViewById<EditText>(R.id.eventMinEd)
    eventMinEd.visibility =
      if (eventPath.isNotEmpty()) {
        View.VISIBLE
      } else {
        View.GONE
      }
    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

    val isFabricFilteredEd = dialogView.findViewById<Spinner>(R.id.isFabricFilteredSp)
    dialogView.findViewById<Button>(R.id.readBtn).setOnClickListener {
      scope.launch {
        var eventMin: Long? = null
        if (eventPath.isNotEmpty() && eventMinEd.text.isNotBlank()) {
          eventMin = eventMinEd.text.toString().toULong().toLong()
        }
        if (isICDQueueBtn) {
          readICDConfig =
            ReadICDConfig(isFabricFilteredEd.selectedItem.toString().toBoolean(), eventMin)
        } else {
          read(isFabricFilteredEd.selectedItem.toString().toBoolean(), eventMin)
        }
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
    return true
  }

  private fun showWriteDialog(isICDQueueBtn: Boolean) {
    binding.outputTv.text = ""
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.write_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

    dialogView.findViewById<Button>(R.id.writeBtn).setOnClickListener {
      val timedRequestTimeoutMs =
        dialogView.findViewById<EditText>(R.id.timedRequestTimeoutEd).text.toString()
      val imTimeout = dialogView.findViewById<EditText>(R.id.imTimeoutEd).text.toString()
      scope.launch {
        val timedRequestTimeoutInt =
          if (timedRequestTimeoutMs.isEmpty()) {
            0
          } else {
            timedRequestTimeoutMs.toInt()
          }
        val imTimeoutInt =
          if (imTimeout.isEmpty()) {
            0
          } else {
            imTimeout.toInt()
          }
        if (isICDQueueBtn) {
          writeICDConfig = WriteInvokeICDConfig(timedRequestTimeoutInt, imTimeoutInt)
        } else {
          write(timedRequestTimeoutInt, imTimeoutInt)
        }
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private fun showSubscribeDialog(isICDQueueBtn: Boolean): Boolean {
    if (attributePath.isEmpty() && eventPath.isEmpty()) {
      requireActivity().runOnUiThread {
        Toast.makeText(
            requireActivity(),
            R.string.wildcard_empty_error_toast_message,
            Toast.LENGTH_SHORT
          )
          .show()
      }
      return false
    }
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.subscribe_dialog, null)
    val eventMinEd = dialogView.findViewById<EditText>(R.id.eventMinEd)
    eventMinEd.visibility =
      if (eventPath.isNotEmpty()) {
        View.VISIBLE
      } else {
        View.GONE
      }
    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

    val minIntervalEd = dialogView.findViewById<EditText>(R.id.minIntervalEd)
    val maxIntervalEd = dialogView.findViewById<EditText>(R.id.maxIntervalEd)
    val keepSubscriptionsSp = dialogView.findViewById<Spinner>(R.id.keepSubscriptionsSp)
    val isFabricFilteredSp = dialogView.findViewById<Spinner>(R.id.isFabricFilteredSp)
    dialogView.findViewById<Button>(R.id.subscribeBtn).setOnClickListener {
      scope.launch {
        if (minIntervalEd.text.isNotBlank() && maxIntervalEd.text.isNotBlank()) {
          var eventMin: Long? = null
          if (eventPath.isNotEmpty() && eventMinEd.text.isNotBlank()) {
            eventMin = eventMinEd.text.toString().toULong().toLong()
          }
          if (isICDQueueBtn) {
            subscribeICDConfig =
              SubscribeICDConfig(
                minIntervalEd.text.toString().toInt(),
                maxIntervalEd.text.toString().toInt(),
                keepSubscriptionsSp.selectedItem.toString().toBoolean(),
                isFabricFilteredSp.selectedItem.toString().toBoolean(),
                eventMin
              )
          } else {
            subscribe(
              minIntervalEd.text.toString().toInt(),
              maxIntervalEd.text.toString().toInt(),
              keepSubscriptionsSp.selectedItem.toString().toBoolean(),
              isFabricFilteredSp.selectedItem.toString().toBoolean(),
              eventMin
            )
          }
        } else {
          Log.e(TAG, "minInterval or maxInterval is empty!")
        }
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
    return true
  }

  private fun showInvokeDialog(isICDQueueBtn: Boolean) {
    binding.outputTv.text = ""
    val dialogView = requireActivity().layoutInflater.inflate(R.layout.invoke_dialog, null)
    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

    dialogView.findViewById<Button>(R.id.invokeBtn).setOnClickListener {
      val timedRequestTimeoutMs =
        dialogView.findViewById<EditText>(R.id.timedRequestTimeoutEd).text.toString()
      val imTimeout = dialogView.findViewById<EditText>(R.id.imTimeoutEd).text.toString()
      scope.launch {
        val timedRequestTimeoutInt =
          if (timedRequestTimeoutMs.isEmpty()) {
            0
          } else {
            timedRequestTimeoutMs.toInt()
          }
        val imTimeoutInt =
          if (imTimeout.isEmpty()) {
            0
          } else {
            imTimeout.toInt()
          }
        if (isICDQueueBtn) {
          invokeICDConfig = WriteInvokeICDConfig(timedRequestTimeoutInt, imTimeoutInt)
        } else {
          invoke(timedRequestTimeoutInt, imTimeoutInt)
        }
        requireActivity().runOnUiThread { dialog.dismiss() }
      }
    }
    dialog.show()
  }

  private suspend fun readCurrentFabricIndex(): UInt {
    val context = requireContext()
    val endpointId = 0
    val clusterId = 62L // OperationalCredentials
    val attributeId = 5L // CurrentFabricIndex
    val deviceId = addressUpdateFragment.deviceId
    val devicePointer =
      try {
        ChipClient.getConnectedDevicePointer(context, deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        return 0U
      }
    return suspendCoroutine { cont ->
      deviceController.readAttributePath(
        object : ReportCallback {
          override fun onError(
            attributePath: ChipAttributePath?,
            eventPath: ChipEventPath?,
            e: java.lang.Exception
          ) {
            cont.resume(0u)
          }

          override fun onReport(nodeState: NodeState?) {
            val state =
              nodeState
                ?.getEndpointState(endpointId.toInt())
                ?.getClusterState(clusterId)
                ?.getAttributeState(attributeId)
            if (state == null) {
              cont.resume(0u)
              return
            }
            val ret = TlvReader(state.tlv).getUInt(AnonymousTag)
            cont.resume(ret)
          }
        },
        devicePointer,
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
    val dialogView =
      requireActivity().layoutInflater.inflate(R.layout.shutdown_subscribe_dialog, null)
    val subscriptionIdSp = dialogView.findViewById<Spinner>(R.id.subscribeIdSp)
    val fabricIndexTv = dialogView.findViewById<TextView>(R.id.fabricIndexValue)
    val shutdownBtn = dialogView.findViewById<Button>(R.id.shutdownBtn)
    val shutdownAllBtn = dialogView.findViewById<Button>(R.id.shutdownAllBtn)
    val spinnerAdapter =
      ArrayAdapter(
        requireActivity(),
        android.R.layout.simple_spinner_dropdown_item,
        subscribeIdList
      )
    subscriptionIdSp.adapter = spinnerAdapter
    val dialog = AlertDialog.Builder(requireContext()).apply { setView(dialogView) }.create()

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
    fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag = AnonymousTag)
  }

  companion object {
    private const val TAG = "WildcardFragment"

    private enum class SendType {
      ATTRIBUTE,
      EVENT
    }

    fun newInstance(): WildcardFragment = WildcardFragment()

    private val TLV_MAP =
      mapOf(
        "json" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.putJsonString(tag, value)
            }
          },
        "UnsignedInt" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(tag, value.toULong())
            }
          },
        "Int" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(tag, value.toLong())
            }
          },
        "Boolean" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(tag, value.toBoolean())
            }
          },
        "Float" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(tag, value.toFloat())
            }
          },
        "Double" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(tag, value.toDouble())
            }
          },
        "String" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(tag, value)
            }
          },
        "ByteArray(Hex)" to
          object : TlvWriterInterface {
            override fun generate(writer: TlvWriter, value: String, tag: matter.tlv.Tag) {
              writer.put(
                tag,
                value.chunked(2).map { it.toInt(16) and 0xFF }.map { it.toByte() }.toByteArray()
              )
            }
          },
      )
  }
}
