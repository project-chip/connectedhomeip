package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.*
import androidx.appcompat.app.AlertDialog
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.view.forEach
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.clusterinfo.ClusterCommandCallback
import chip.clusterinfo.ClusterInfo
import chip.clusterinfo.CommandResponseInfo
import chip.clusterinfo.DelegatedClusterCallback
import chip.clusterinfo.InteractionInfo
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterInfoMapping
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.ChipClient.getConnectedDevicePointer
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.clusterclient.clusterinteraction.ClusterInteractionHistoryFragment.Companion.clusterInteractionHistoryList
import com.google.chip.chiptool.databinding.ClusterDetailFragmentBinding
import java.util.*
import kotlin.collections.HashMap
import kotlin.properties.Delegates
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch

/**
 * ClusterDetailFragment allows user to pick cluster, command, specify parameters and see the
 * callback result.
 */
class ClusterDetailFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope
  private var clusterMap: Map<String, ClusterInfo> = ClusterInfoMapping().clusterMap
  private lateinit var selectedClusterInfo: ClusterInfo
  private lateinit var selectedCluster: ChipClusters.BaseChipCluster
  private lateinit var selectedCommandCallback: DelegatedClusterCallback
  private lateinit var selectedInteractionInfo: InteractionInfo
  private var devicePtr by Delegates.notNull<Long>()
  private var deviceId by Delegates.notNull<Long>()
  private var endpointId by Delegates.notNull<Int>()

  // when user opens detail page from home page of cluster interaction, historyCommand will be
  // null, and nothing will be autocompleted. If the detail page is opened from history page,
  // cluster name, command name and potential parameter list will be filled out based on
  // historyCommand
  private var historyCommand: HistoryCommand? = null
  private var _binding: ClusterDetailFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ClusterDetailFragmentBinding.inflate(inflater, container, false)
    scope = viewLifecycleOwner.lifecycleScope
    deviceId = checkNotNull(requireArguments().getLong(DEVICE_ID))
    scope.launch {
      try {
        devicePtr = getConnectedDevicePointer(requireContext(), deviceId)
      } catch (e: IllegalStateException) {
        Log.d(TAG, "getConnectedDevicePointer exception", e)
        showMessage("Get DevicePointer fail!")
        return@launch
      }
    }
    endpointId = checkNotNull(requireArguments().getInt(ENDPOINT_ID_KEY))
    historyCommand = requireArguments().getSerializable(HISTORY_COMMAND) as HistoryCommand?
    deviceController.setCompletionListener(GenericChipDeviceListener())
    if (historyCommand != null) {
      autoCompleteBasedOnHistoryCommand(
        historyCommand!!,
        binding.clusterAutoCompleteTv,
        binding.commandAutoCompleteTv,
        binding.parameterList,
        inflater,
        binding.callbackList
      )
    } else {
      binding.commandAutoCompleteTv.visibility = View.GONE
      clusterAutoCompleteSetup(
        binding.clusterAutoCompleteTv,
        binding.commandAutoCompleteTv,
        binding.parameterList,
        binding.callbackList
      )
      commandAutoCompleteSetup(
        binding.commandAutoCompleteTv,
        inflater,
        binding.parameterList,
        binding.callbackList
      )
    }

    setInvokeCommandOnClickListener(
      binding.invokeCommand,
      binding.callbackList,
      binding.clusterAutoCompleteTv,
      binding.commandAutoCompleteTv,
      binding.parameterList
    )

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun setInvokeCommandOnClickListener(
    invokeCommand: Button,
    callbackList: LinearLayout,
    clusterAutoCompleteTv: AutoCompleteTextView,
    commandAutoCompleteTv: AutoCompleteTextView,
    parameterList: LinearLayout
  ) {
    invokeCommand.setOnClickListener {
      callbackList.removeAllViews()
      val commandArguments = HashMap<String, Any>()
      clusterInteractionHistoryList.addFirst(
        HistoryCommand(
          clusterAutoCompleteTv.text.toString(),
          commandAutoCompleteTv.text.toString(),
          mutableListOf(),
          null,
          null,
          endpointId,
          deviceId
        )
      )
      parameterList.forEach {
        val clusterParameterNameTv: TextView = it.findViewById(R.id.clusterParameterNameTv)
        val clusterParameterData: EditText = it.findViewById(R.id.clusterParameterData)

        val parameterName = clusterParameterNameTv.text.toString()
        val parameterType = selectedInteractionInfo.commandParameters[parameterName]!!.type
        val parameterUnderlyingType =
          selectedInteractionInfo.commandParameters[parameterName]!!.underlyingType
        val data =
          castStringToType(
            clusterParameterData.text.toString(),
            parameterType,
            parameterUnderlyingType
          )!!
        commandArguments[clusterParameterNameTv.text.toString()] = data
        clusterInteractionHistoryList[0]
          .parameterList
          .add(HistoryParameterInfo(parameterName, data.toString(), parameterUnderlyingType))
      }
      selectedInteractionInfo
        .getCommandFunction()
        .invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
    }
  }

  // Cluster name, command name and parameter list will be autofill based on the given
  // historyCommand
  private fun autoCompleteBasedOnHistoryCommand(
    historyCommand: HistoryCommand,
    clusterAutoComplete: AutoCompleteTextView,
    commandAutoComplete: AutoCompleteTextView,
    parameterList: LinearLayout,
    inflater: LayoutInflater,
    callbackList: LinearLayout
  ) {
    clusterAutoComplete.setText(historyCommand.clusterName)
    commandAutoComplete.visibility = View.VISIBLE
    commandAutoComplete.setText(historyCommand.commandName)
    selectedClusterInfo = clusterMap[historyCommand.clusterName]!!
    selectedCluster = selectedClusterInfo.createClusterFunction.create(devicePtr, endpointId)
    selectedInteractionInfo = selectedClusterInfo.commands[historyCommand.commandName]!!
    selectedCommandCallback = selectedInteractionInfo.commandCallbackSupplier.get()
    setCallbackDelegate(inflater, callbackList)
    historyCommand.parameterList.forEach {
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      val clusterParameterNameTv: TextView = param.findViewById(R.id.clusterParameterNameTv)
      val clusterParameterTypeTv: TextView = param.findViewById(R.id.clusterParameterTypeTv)
      val clusterParameterData: EditText = param.findViewById(R.id.clusterParameterData)

      clusterParameterNameTv.text = "${it.parameterName}"
      clusterParameterTypeTv.text = formatParameterType(it.parameterType)
      clusterParameterData.setText(it.parameterData)
      parameterList.addView(param)
    }
  }

  private fun formatParameterType(castType: Class<*>): String {
    return if (castType == ByteArray::class.java) {
      "Byte[]"
    } else {
      castType.toString()
    }
  }

  private fun castStringToType(data: String, type: Class<*>, underlyingType: Class<*>): Any? {
    return when (type) {
      Int::class.java,
      java.lang.Integer::class.java -> data.toInt()
      Boolean::class.java,
      java.lang.Boolean::class.java -> data.toBoolean()
      ByteArray::class.java -> data.encodeToByteArray()
      Long::class.java,
      java.lang.Long::class.java -> data.toLong()
      Short::class.java,
      java.lang.Short::class.java -> data.toShort()
      Double::class.java,
      java.lang.Double::class.java -> data.toDouble()
      Float::class.java,
      java.lang.Float::class.java -> data.toFloat()
      Optional::class.java ->
        if (data.isEmpty()) Optional.empty()
        else Optional.of(castStringToType(data, underlyingType, underlyingType)!!)
      else -> data
    }
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
    }
  }

  private fun clusterAutoCompleteSetup(
    clusterAutoComplete: AutoCompleteTextView,
    commandAutoComplete: AutoCompleteTextView,
    parameterList: LinearLayout,
    callbackList: LinearLayout
  ) {
    val clusterNameList = constructHint(clusterMap)
    val clusterAdapter =
      ArrayAdapter(requireContext(), android.R.layout.simple_list_item_1, clusterNameList)
    clusterAutoComplete.setAdapter(clusterAdapter)
    clusterAutoComplete.setOnItemClickListener { parent, view, position, id ->
      commandAutoComplete.visibility = View.VISIBLE
      // when new cluster is selected, clear the command text and possible parameterList
      commandAutoComplete.setText("", false)
      parameterList.removeAllViews()
      callbackList.removeAllViews()
      // populate all the commands that belong to the selected cluster
      val selectedCluster: String = clusterAutoComplete.adapter.getItem(position).toString()
      val commandAdapter = getCommandOptions(selectedCluster)
      commandAutoComplete.setAdapter(commandAdapter)
    }
  }

  private fun commandAutoCompleteSetup(
    commandAutoComplete: AutoCompleteTextView,
    inflater: LayoutInflater,
    parameterList: LinearLayout,
    callbackList: LinearLayout
  ) {
    commandAutoComplete.setOnItemClickListener { parent, view, position, id ->
      // when new command is selected, clear all the parameterList and callbackList
      parameterList.removeAllViews()
      callbackList.removeAllViews()
      selectedCluster = selectedClusterInfo.createClusterFunction.create(devicePtr, endpointId)
      val selectedCommand: String = commandAutoComplete.adapter.getItem(position).toString()
      selectedInteractionInfo = selectedClusterInfo.commands[selectedCommand]!!
      selectedCommandCallback = selectedInteractionInfo.commandCallbackSupplier.get()
      populateCommandParameter(inflater, parameterList)
      setCallbackDelegate(inflater, callbackList)
    }
  }

  private fun setCallbackDelegate(inflater: LayoutInflater, callbackList: LinearLayout) {
    selectedCommandCallback.setCallbackDelegate(
      object : ClusterCommandCallback {
        override fun onSuccess(responseValues: Map<CommandResponseInfo, Any>) {
          showMessage("Command success")
          // Populate UI based on response values. We know the types from
          // CommandInfo.getCommandResponses().
          requireActivity().runOnUiThread {
            populateCallbackResult(
              responseValues,
              inflater,
              callbackList,
            )
          }
          clusterInteractionHistoryList[0].responseValue = responseValues
          clusterInteractionHistoryList[0].status = "Success"
          responseValues.forEach { Log.d(TAG, it.toString()) }
        }

        override fun onFailure(exception: Exception) {
          showMessage("Command failed")
          var errorStatus = exception.toString().split(':')
          clusterInteractionHistoryList[0].status =
            errorStatus[errorStatus.size - 2] + " " + errorStatus[errorStatus.size - 1]
          Log.e(TAG, exception.toString())
        }
      }
    )
  }

  private fun populateCommandParameter(inflater: LayoutInflater, parameterList: LinearLayout) {
    selectedInteractionInfo.commandParameters.forEach { (paramName, paramInfo) ->
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      val clusterParameterNameTv: TextView = param.findViewById(R.id.clusterParameterNameTv)
      val clusterParameterTypeTv: TextView = param.findViewById(R.id.clusterParameterTypeTv)

      clusterParameterNameTv.text = "${paramName}"
      // byte[].class will be output as class [B, which is not readable, so dynamically change
      // it
      // to Byte[]. If more custom logic is required, should add a className field in
      // commandParameterInfo
      clusterParameterTypeTv.text = formatParameterType(paramInfo.type)
      parameterList.addView(param)
    }
  }

  private fun populateCallbackResult(
    responseValues: Map<CommandResponseInfo, Any>,
    inflater: LayoutInflater,
    callbackList: LinearLayout
  ) {
    responseValues.forEach { (variableNameType, response) ->
      if (response is List<*>) {
        createListResponseView(response, inflater, callbackList, variableNameType)
      } else {
        createBasicResponseView(response, inflater, callbackList, variableNameType)
      }
    }
  }

  private fun createBasicResponseView(
    response: Any,
    inflater: LayoutInflater,
    callbackList: LinearLayout,
    variableNameType: CommandResponseInfo
  ) {
    val callbackItem =
      inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
    val clusterCallbackNameTv: TextView = callbackItem.findViewById(R.id.clusterCallbackNameTv)
    val clusterCallbackDataTv: TextView = callbackItem.findViewById(R.id.clusterCallbackDataTv)
    val clusterCallbackTypeTv: TextView = callbackItem.findViewById(R.id.clusterCallbackTypeTv)

    clusterCallbackNameTv.text = variableNameType.name
    clusterCallbackDataTv.text =
      if (response == null) {
        "null"
      } else if (response.javaClass == ByteArray::class.java) {
        (response as ByteArray).decodeToString()
      } else {
        response.toString()
      }
    clusterCallbackTypeTv.text = variableNameType.type
    callbackList.addView(callbackItem)
  }

  private fun createListResponseView(
    response: List<*>,
    inflater: LayoutInflater,
    callbackList: LinearLayout,
    variableNameType: CommandResponseInfo
  ) {
    if (response.isEmpty()) {
      val emptyCallback =
        inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
      val clusterCallbackNameTv: TextView = emptyCallback.findViewById(R.id.clusterCallbackNameTv)

      clusterCallbackNameTv.text = "Result is empty"
      callbackList.addView(emptyCallback)
    } else {
      response.forEachIndexed { index, it ->
        val attributeCallbackItem =
          inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
        val clusterCallbackNameTv: TextView =
          attributeCallbackItem.findViewById(R.id.clusterCallbackNameTv)
        val clusterCallbackDataTv: TextView =
          attributeCallbackItem.findViewById(R.id.clusterCallbackDataTv)
        val clusterCallbackTypeTv: TextView =
          attributeCallbackItem.findViewById(R.id.clusterCallbackTypeTv)

        clusterCallbackNameTv.text = variableNameType.name + "[$index]"
        val objectString =
          if (it!!.javaClass == ByteArray::class.java) {
            (it as ByteArray).contentToString()
          } else {
            it.toString()
          }

        var callbackClassName =
          if (it!!.javaClass == ByteArray::class.java) {
            "Byte[]"
          } else {
            it!!.javaClass.toString().split('$').last()
          }

        clusterCallbackDataTv.text = callbackClassName
        clusterCallbackDataTv.setOnClickListener {
          AlertDialog.Builder(requireContext())
            .setTitle(callbackClassName)
            .setMessage(objectString)
            .create()
            .show()
        }

        clusterCallbackTypeTv.text = "List<$callbackClassName>"
        callbackList.addView(attributeCallbackItem)
      }
    }
  }

  private fun getCommandOptions(clusterName: String): ArrayAdapter<String> {
    selectedClusterInfo = clusterMap[clusterName]!!
    val commandNameList = constructHint(selectedClusterInfo.commands)
    return ArrayAdapter(requireContext(), android.R.layout.simple_list_item_1, commandNameList)
  }

  private fun constructHint(clusterMap: Map<String, *>): Array<String> {
    return clusterMap.keys.toTypedArray()
  }

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  companion object {
    private const val TAG = "ClusterDetailFragment"
    private const val ENDPOINT_ID_KEY = "endpointId"
    private const val HISTORY_COMMAND = "historyCommand"
    private const val DEVICE_ID = "deviceId"

    fun newInstance(
      deviceId: Long,
      endpointId: Int,
      historyCommand: HistoryCommand?
    ): ClusterDetailFragment {
      return ClusterDetailFragment().apply {
        arguments =
          Bundle(4).apply {
            putLong(DEVICE_ID, deviceId)
            putSerializable(HISTORY_COMMAND, historyCommand)
            putInt(ENDPOINT_ID_KEY, endpointId)
          }
      }
    }
  }
}
