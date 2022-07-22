package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.AutoCompleteTextView
import android.widget.Button
import android.widget.LinearLayout
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.view.forEach
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.clusterinfo.ClusterCommandCallback
import chip.clusterinfo.ClusterInfo
import chip.clusterinfo.InteractionInfo
import chip.clusterinfo.CommandResponseInfo
import chip.clusterinfo.DelegatedClusterCallback
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterInfoMapping
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.ChipClient.getConnectedDevicePointer
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.clusterclient.clusterinteraction.ClusterInteractionHistoryFragment.Companion.clusterInteractionHistoryList
import kotlin.properties.Delegates
import kotlinx.android.synthetic.main.cluster_callback_item.view.clusterCallbackDataTv
import kotlinx.android.synthetic.main.cluster_callback_item.view.clusterCallbackNameTv
import kotlinx.android.synthetic.main.cluster_callback_item.view.clusterCallbackTypeTv
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.callbackList
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.clusterAutoCompleteTv
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.commandAutoCompleteTv
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.invokeCommand
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.parameterList
import kotlinx.android.synthetic.main.cluster_parameter_item.view.clusterParameterData
import kotlinx.android.synthetic.main.cluster_parameter_item.view.clusterParameterNameTv
import kotlinx.android.synthetic.main.cluster_parameter_item.view.clusterParameterTypeTv
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
import kotlinx.coroutines.launch
import java.util.*
import kotlin.collections.HashMap

/**
 * ClusterDetailFragment allows user to pick cluster, command, specify parameters and see
 * the callback result.
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
  // cluster name, command name and potential parameter list will be filled out based on historyCommand
  private var historyCommand: HistoryCommand? = null

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    scope = viewLifecycleOwner.lifecycleScope
    deviceId = checkNotNull(requireArguments().getLong(DEVICE_ID))
    scope.launch {
      devicePtr = getConnectedDevicePointer(requireContext(), deviceId)
    }
    endpointId = checkNotNull(requireArguments().getInt(ENDPOINT_ID_KEY))
    historyCommand = requireArguments().getSerializable(HISTORY_COMMAND) as HistoryCommand?
    return inflater.inflate(R.layout.cluster_detail_fragment, container, false).apply {
      deviceController.setCompletionListener(GenericChipDeviceListener())
      if (historyCommand != null) {
        autoCompleteBasedOnHistoryCommand(
          historyCommand!!,
          clusterAutoCompleteTv,
          commandAutoCompleteTv,
          parameterList,
          inflater,
          callbackList
        )
      } else {
        commandAutoCompleteTv.visibility = View.GONE
        clusterAutoCompleteSetup(
          clusterAutoCompleteTv,
          commandAutoCompleteTv,
          parameterList,
          callbackList
        )
        commandAutoCompleteSetup(commandAutoCompleteTv, inflater, parameterList, callbackList)
      }
      setInvokeCommandOnClickListener(
        invokeCommand,
        callbackList,
        clusterAutoCompleteTv,
        commandAutoCompleteTv,
        parameterList
      )
    }
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
        val parameterName = it.clusterParameterNameTv.text.toString()
        val parameterType = selectedInteractionInfo.commandParameters[parameterName]!!.type
        val parameterUnderlyingType = selectedInteractionInfo.commandParameters[parameterName]!!.underlyingType
        val data = castStringToType(it.clusterParameterData.text.toString(), parameterType, parameterUnderlyingType)!!
        commandArguments[it.clusterParameterNameTv.text.toString()] = data
        clusterInteractionHistoryList[0].parameterList.add(
          HistoryParameterInfo(
            parameterName,
            data.toString(),
            parameterUnderlyingType
          )
        )
      }
      selectedInteractionInfo.getCommandFunction()
        .invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
    }
  }

  // Cluster name, command name and parameter list will be autofill based on the given historyCommand
  private fun autoCompleteBasedOnHistoryCommand(
    historyCommand: HistoryCommand,
    clusterAutoComplete: AutoCompleteTextView,
    commandAutoComplete: AutoCompleteTextView,
    parameterList: LinearLayout, inflater: LayoutInflater,
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
      param.clusterParameterNameTv.text = "${it.parameterName}"
      param.clusterParameterTypeTv.text = formatParameterType(it.parameterType)
      param.clusterParameterData.setText(it.parameterData)
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
      Int::class.java -> data.toInt()
      Boolean::class.java -> data.toBoolean()
      ByteArray::class.java -> data.encodeToByteArray()
      Long::class.java -> data.toLong()
      Optional::class.java -> if (data.isEmpty()) Optional.empty() else Optional.of(castStringToType(data, underlyingType, underlyingType)!!)
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
    selectedCommandCallback.setCallbackDelegate(object : ClusterCommandCallback {
      override fun onSuccess(responseValues: Map<CommandResponseInfo, Any>) {
        showMessage("Command success")
        // Populate UI based on response values. We know the types from CommandInfo.getCommandResponses().
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
    })
  }

  private fun populateCommandParameter(inflater: LayoutInflater, parameterList: LinearLayout) {
    selectedInteractionInfo.commandParameters.forEach { (paramName, paramInfo) ->
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      param.clusterParameterNameTv.text = "${paramName}"
      // byte[].class will be output as class [B, which is not readable, so dynamically change it
      // to Byte[]. If more custom logic is required, should add a className field in
      // commandParameterInfo
      param.clusterParameterTypeTv.text = formatParameterType(paramInfo.type)
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
    callbackItem.clusterCallbackNameTv.text = variableNameType.name
    callbackItem.clusterCallbackDataTv.text = if (response.javaClass == ByteArray::class.java) {
      (response as ByteArray).decodeToString()
    } else {
      response.toString()
    }
    callbackItem.clusterCallbackTypeTv.text = variableNameType.type
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
      emptyCallback.clusterCallbackNameTv.text = "Result is empty"
      callbackList.addView(emptyCallback)
    } else {
      response.forEachIndexed { index, it ->
        val attributeCallbackItem =
          inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
        attributeCallbackItem.clusterCallbackNameTv.text = variableNameType.name + "[$index]"
        val objectString = if (it!!.javaClass == ByteArray::class.java) {
          (it as ByteArray).contentToString()
        } else {
          it.toString()
        }
        var callbackClassName = if (it!!.javaClass == ByteArray::class.java) {
          "Byte[]"
        } else {
          it!!.javaClass.toString().split('$').last()
        }
        attributeCallbackItem.clusterCallbackDataTv.text = callbackClassName
        attributeCallbackItem.clusterCallbackDataTv.setOnClickListener {
          AlertDialog.Builder(requireContext())
            .setTitle(callbackClassName)
            .setMessage(objectString)
            .create()
            .show()
        }
        attributeCallbackItem.clusterCallbackTypeTv.text = "List<$callbackClassName>"
        callbackList.addView(attributeCallbackItem)
      }
    }
  }

  private fun getCommandOptions(
    clusterName: String
  ): ArrayAdapter<String> {
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
        arguments = Bundle(4).apply {
          putLong(DEVICE_ID, deviceId)
          putSerializable(HISTORY_COMMAND, historyCommand)
          putInt(ENDPOINT_ID_KEY, endpointId)
        }
      }
    }
  }
}
