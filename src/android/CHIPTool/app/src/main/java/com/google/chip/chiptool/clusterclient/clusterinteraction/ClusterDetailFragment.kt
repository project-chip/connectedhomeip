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
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.core.view.forEach
import androidx.fragment.app.Fragment
import chip.clusterinfo.ClusterCommandCallback
import chip.clusterinfo.ClusterInfo
import chip.clusterinfo.CommandInfo
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
  private lateinit var selectedCommandInfo: CommandInfo
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
      clusterAutoCompleteSetup(clusterAutoCompleteTv, commandAutoCompleteTv, parameterList)
      commandAutoCompleteSetup(commandAutoCompleteTv, inflater, parameterList, callbackList)
      invokeCommand.setOnClickListener {
        val commandArguments = HashMap<String, Any>()
        parameterList.forEach {
          val type =
            selectedCommandInfo.commandParameters[it.clusterParameterNameTv.text.toString()]!!.type
          val data = castStringToType(it.clusterParameterData.text.toString(), type)!!

          commandArguments[it.clusterParameterNameTv.text.toString()] = data
        }
        selectedCommandInfo.getCommandFunction()
          .invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
      }
    }
  }

  private fun castStringToType(data: String, type: Class<*>): Any? {
    return when (type) {
      Int::class.java -> data.toInt()
      String::class.java -> data
      Boolean::class.java -> data.toBoolean()
      else -> null
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
    parameterList: LinearLayout
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
      // when new command is selected, clear all the parameterList
      parameterList.removeAllViews()
      selectedCluster = selectedClusterInfo.createClusterFunction.create(devicePtr, endpointId)
      val selectedCommand: String = commandAutoComplete.adapter.getItem(position).toString()
      selectedCommandInfo = selectedClusterInfo.commands[selectedCommand]!!
      selectedCommandCallback = selectedCommandInfo.commandCallbackSupplier.get()
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
    selectedCommandInfo.commandParameters.forEach { (paramName, paramInfo) ->
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      param.clusterParameterNameTv.text = "${paramName}"
      param.clusterParameterTypeTv.text = "${paramInfo.type}"
      parameterList.addView(param)
    }
  }

  private fun populateCallbackResult(
    responseValues: Map<CommandResponseInfo, Any>,
    inflater: LayoutInflater,
    callbackList: LinearLayout
  ) {
    responseValues.forEach { (variableNameType, response) ->
      val callback =
        inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
      callback.clusterCallbackNameTv.text = variableNameType.name
      callback.clusterCallbackDataTv.text = response.toString()
      callback.clusterCallbackTypeTv.text = variableNameType.type
      callbackList.addView(callback)
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