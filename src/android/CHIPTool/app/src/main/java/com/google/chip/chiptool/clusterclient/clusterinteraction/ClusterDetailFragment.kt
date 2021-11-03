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
import androidx.core.view.forEach
import androidx.fragment.app.Fragment
import chip.clusterinfo.ClusterCommandCallback
import chip.clusterinfo.ClusterInfo
import chip.clusterinfo.CommandInfo
import chip.clusterinfo.DelegatedClusterCallback
import chip.devicecontroller.ChipClusters
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.callback_item.view.callbackData
import kotlinx.android.synthetic.main.callback_item.view.callbackName
import kotlinx.android.synthetic.main.callback_item.view.callbackType
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.callbackList
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.clusterAutoComplete
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.commandAutoComplete
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.invokeCommand
import kotlinx.android.synthetic.main.cluster_detail_fragment.view.parameterList
import kotlinx.android.synthetic.main.parameter_item.view.parameterData
import kotlinx.android.synthetic.main.parameter_item.view.parameterName
import kotlinx.android.synthetic.main.parameter_item.view.parameterType
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
  private lateinit var clusterMap: HashMap<String, ClusterInfo>
  private lateinit var selectedClusterInfo: ClusterInfo
  private lateinit var selectedCluster: ChipClusters.BaseChipCluster
  private lateinit var selectedCommandCallback: DelegatedClusterCallback
  private var commandArguments = HashMap<String, Any>()
  private lateinit var selectedCommandInfo: CommandInfo
  private var devicePtr = 0L
  private var endpointId = 1

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    clusterMap =
      checkNotNull(requireArguments().getSerializable(CLUSTER_MAP_INFO)) as HashMap<String, ClusterInfo>
    devicePtr = checkNotNull(requireArguments().getLong(DEVICE_PTR))
    return inflater.inflate(R.layout.cluster_detail_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())
      commandAutoComplete.visibility = View.GONE
      clusterAutoCompleteSetup(clusterAutoComplete, commandAutoComplete)
      commandAutoCompleteSetup(commandAutoComplete, inflater, parameterList, callbackList)
      invokeCommand.setOnClickListener {
        parameterList.forEach {
          val type = selectedCommandInfo.commandParameters[it.parameterName.text.toString()]?.type!!
          val data = castCorrectType(type, it.parameterData.text.toString())!!
          commandArguments.put(it.parameterName.text.toString(), data)
        }

        selectedCommandInfo.getCommandFunction()
          .invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
      }
    }
  }

  private fun castCorrectType(type: Class<*>, data: String): Any? {

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
    commandAutoComplete: AutoCompleteTextView
  ) {
    val clusterNameList = constructHint(clusterMap)
    val clusterAdapter =
      ArrayAdapter(requireContext(), android.R.layout.simple_list_item_1, clusterNameList)
    clusterAutoComplete.setAdapter(clusterAdapter)
    clusterAutoComplete.setOnItemClickListener { parent, view, position, id ->

      commandAutoComplete.visibility = View.VISIBLE
      val selectedCluster: String = clusterAutoComplete.adapter.getItem(position).toString()
      val commandAdapter = getCommand(selectedCluster)
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
      selectedCluster = selectedClusterInfo.createClusterFunction.create(devicePtr, endpointId)
      val selectedCommand: String = commandAutoComplete.adapter.getItem(position).toString()
      selectedCommandInfo = selectedClusterInfo.commands[selectedCommand]!!
      selectedCommandCallback = selectedCommandInfo.commandCallbackSupplier.get()
      populateCommandParameter(inflater, parameterList)
      selectedCommandCallback?.setCallbackDelegate(object : ClusterCommandCallback {
        override fun onSuccess(responseValues: Map<String, Any>) {
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
      val param = inflater.inflate(R.layout.parameter_item, null, false) as LinearLayout
      param.parameterName.text = "${paramName}"
      param.parameterType.text = "${paramInfo.type}"
      parameterList.addView(param)
    }
  }

  private fun populateCallbackResult(
    responseValues: Map<String, Any>,
    inflater: LayoutInflater,
    callbackList: LinearLayout
  ) {
    responseValues.forEach { (variableNameType, response) ->
      val callback = inflater.inflate(R.layout.callback_item, null, false) as LinearLayout
      callback.callbackName.text = variableNameType.split(',')[0]
      callback.callbackData.text = response.toString()
      callback.callbackType.text = variableNameType.split(',')[1]
      callbackList.addView(callback)
    }
  }

  private fun getCommand(
    clusterName: String
  ): ArrayAdapter<String> {
    selectedClusterInfo = clusterMap[clusterName]!!
    val commandNameList = constructHint(selectedClusterInfo.commands as HashMap<String, Any>)
    return ArrayAdapter(requireContext(), android.R.layout.simple_list_item_1, commandNameList)
  }

  private fun constructHint(clusterMap: HashMap<String, *>): Array<String> {
    val clusterName = mutableListOf<String>()
    for ((name, info) in clusterMap) {
      clusterName.add(name)
    }
    return clusterName.toTypedArray()
  }

  inner class ChipControllerCallback : GenericChipDeviceListener() {
    override fun onConnectDeviceComplete() {}

    override fun onCommissioningComplete(nodeId: Long, errorCode: Int) {
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

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  companion object {
    private const val TAG = "ClusterDetailFragment"
    private const val CLUSTER_MAP_INFO = "cluster_map_info"
    private const val DEVICE_PTR = "device_ptr"
    private const val ENDPOINT_ID = "endpoint_id"
    fun newInstance(
      clusterMap: HashMap<String, ClusterInfo>,
      deviceId: Long
    ): ClusterDetailFragment {
      return ClusterDetailFragment().apply {
        arguments = Bundle(1).apply {
          putSerializable(CLUSTER_MAP_INFO, clusterMap)
          putLong(DEVICE_PTR, deviceId)
        }
      }
    }
  }

}