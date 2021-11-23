package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.AutoCompleteTextView
import android.widget.LinearLayout
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.view.forEach
import androidx.fragment.app.Fragment
import chip.clusterinfo.ClusterCommandCallback
import chip.clusterinfo.ClusterInfo
import chip.clusterinfo.InteractionInfo
import chip.clusterinfo.CommandResponseInfo
import chip.clusterinfo.DelegatedClusterCallback
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterInfoMapping
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
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

/**
 * ClusterDetailFragment allows user to pick cluster, command, specify parameters and see
 * the callback result.
 */
class ClusterDetailFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())
  private lateinit var clusterMap: Map<String, ClusterInfo>
  private lateinit var selectedClusterInfo: ClusterInfo
  private lateinit var selectedCluster: ChipClusters.BaseChipCluster
  private lateinit var selectedCommandCallback: DelegatedClusterCallback
  private lateinit var selectedInteractionInfo: InteractionInfo
  private var devicePtr = 0L
  private var endpointId = 0

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    clusterMap = ClusterInfoMapping().clusterMap
    devicePtr = checkNotNull(requireArguments().getLong(DEVICE_PTR_KEY))
    endpointId = checkNotNull(requireArguments().getInt(ENDPOINT_ID_KEY))
    return inflater.inflate(R.layout.cluster_detail_fragment, container, false).apply {
      deviceController.setCompletionListener(GenericChipDeviceListener())
      commandAutoCompleteTv.visibility = View.GONE
      clusterAutoCompleteSetup(
        clusterAutoCompleteTv,
        commandAutoCompleteTv,
        parameterList,
        callbackList
      )
      commandAutoCompleteSetup(commandAutoCompleteTv, inflater, parameterList, callbackList)
      invokeCommand.setOnClickListener {
        callbackList.removeAllViews()
        val commandArguments = HashMap<String, Any>()
        parameterList.forEach {
          val type =
            selectedInteractionInfo.commandParameters[it.clusterParameterNameTv.text.toString()]!!.type
          val data = castStringToType(it.clusterParameterData.text.toString(), type)!!

          commandArguments[it.clusterParameterNameTv.text.toString()] = data
        }
        selectedInteractionInfo.getCommandFunction()
          .invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
      }
    }
  }

  private fun castStringToType(data: String, type: Class<*>): Any? {
    return when (type) {
      Int::class.java -> data.toInt()
      Boolean::class.java -> data.toBoolean()
      ByteArray::class.java -> data.encodeToByteArray()
      Long::class.java -> data.toLong()
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
      selectedCommandCallback.setCallbackDelegate(object : ClusterCommandCallback {
        override fun onSuccess(responseValues: Map<CommandResponseInfo, Any>) {
          showMessage("Command success")
          // Populate UI based on response values. We know the types from CommandInfo.getCommandResponses().
          requireActivity().runOnUiThread {
            populateCallbackResult(
              responseValues,
              inflater,
              callbackList
            )
          }
          responseValues.forEach { Log.d(TAG, it.toString()) }
        }

        override fun onFailure(exception: Exception) {
          showMessage("Command failed")
          Log.e(TAG, exception.toString())
        }
      })
    }
  }

  private fun populateCommandParameter(inflater: LayoutInflater, parameterList: LinearLayout) {
    selectedInteractionInfo.commandParameters.forEach { (paramName, paramInfo) ->
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      param.clusterParameterNameTv.text = "${paramName}"
      // byte[].class will be output as class [B, which is not readable, so dynamically change it
      // to Byte[]. If more custom logic is required, should add a className field in
      // commandParameterInfo
      if (paramInfo.type == ByteArray::class.java) {
        param.clusterParameterTypeTv.text = "Byte[]"
      } else {
        param.clusterParameterTypeTv.text = "${paramInfo.type}"
      }
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
    private const val ENDPOINT_ID_KEY = "endpoint_id"
    private const val DEVICE_PTR_KEY = "device_ptr"

    fun newInstance(
      deviceId: Long,
      endpointId: Int
    ): ClusterDetailFragment {
      return ClusterDetailFragment().apply {
        arguments = Bundle(2).apply {
          putLong(DEVICE_PTR_KEY, deviceId)
          putInt(ENDPOINT_ID_KEY, endpointId)
        }
      }
    }
  }
}