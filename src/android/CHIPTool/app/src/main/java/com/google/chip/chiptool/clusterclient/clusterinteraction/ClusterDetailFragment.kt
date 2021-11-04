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
<<<<<<< HEAD
<<<<<<< HEAD
    clusterMap = ClusterInfoMapping().clusterMap
    devicePtr = checkNotNull(requireArguments().getLong(DEVICE_PTR_KEY))
    endpointId = checkNotNull(requireArguments().getInt(ENDPOINT_ID_KEY))
=======
    clusterMap =
      checkNotNull(requireArguments().getSerializable(CLUSTER_MAP_INFO)) as HashMap<String, ClusterInfo>
    devicePtr = checkNotNull(requireArguments().getLong(DEVICE_PTR))
>>>>>>> 7c783e227 (add back class description)
    return inflater.inflate(R.layout.cluster_detail_fragment, container, false).apply {
      deviceController.setCompletionListener(GenericChipDeviceListener())
<<<<<<< HEAD
      commandAutoCompleteTv.visibility = View.GONE
      clusterAutoCompleteSetup(clusterAutoCompleteTv, commandAutoCompleteTv, parameterList)
      commandAutoCompleteSetup(commandAutoCompleteTv, inflater, parameterList, callbackList)
=======
      commandAutoComplete.visibility = View.GONE
      clusterAutoCompleteSetup(clusterAutoComplete, commandAutoComplete, parameterList)
      commandAutoCompleteSetup(commandAutoComplete, inflater, parameterList, callbackList)
>>>>>>> d3d83a0bc (select different cluster, command will remove previous displayed parameter)
=======
    clusterMap = ClusterInfoMapping().clusterMap
    devicePtr = checkNotNull(requireArguments().getLong(DEVICE_PTR_KEY))
    endpointId = checkNotNull(requireArguments().getInt(ENDPOINT_ID_KEY))
    return inflater.inflate(R.layout.cluster_detail_fragment, container, false).apply {
      deviceController.setCompletionListener(GenericChipDeviceListener())
      commandAutoCompleteTv.visibility = View.GONE
      clusterAutoCompleteSetup(clusterAutoCompleteTv, commandAutoCompleteTv, parameterList)
      commandAutoCompleteSetup(commandAutoCompleteTv, inflater, parameterList, callbackList)
>>>>>>> cd1c99028 (resolve comments)
      invokeCommand.setOnClickListener {
        val commandArguments = HashMap<String, Any>()
        parameterList.forEach {
          val type =
<<<<<<< HEAD
<<<<<<< HEAD
            selectedCommandInfo.commandParameters[it.clusterParameterNameTv.text.toString()]!!.type
          val data = castStringToType(it.clusterParameterData.text.toString(), type)!!
=======
            selectedCommandInfo.commandParameters[it.parameterName.text.toString()]!!.type!!
          val data = castCorrectType(type, it.parameterData.text.toString())!!
          commandArguments[it.parameterName.text.toString()] = data
        }
>>>>>>> d3d83a0bc (select different cluster, command will remove previous displayed parameter)

<<<<<<< HEAD
          commandArguments[it.clusterParameterNameTv.text.toString()] = data
        }
=======
>>>>>>> 7c783e227 (add back class description)
=======
            selectedCommandInfo.commandParameters[it.clusterParameterNameTv.text.toString()]!!.type
          val data = castStringToType(it.clusterParameterData.text.toString(), type)!!

          commandArguments[it.clusterParameterNameTv.text.toString()] = data
        }
>>>>>>> cd1c99028 (resolve comments)
        selectedCommandInfo.getCommandFunction()
          .invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
      }
    }
  }

<<<<<<< HEAD
<<<<<<< HEAD
  private fun castStringToType(data: String, type: Class<*>): Any? {
=======
  private fun castCorrectType(type: Class<*>, data: String): Any? {

>>>>>>> 7c783e227 (add back class description)
=======
  private fun castStringToType(data: String, type: Class<*>): Any? {
>>>>>>> cd1c99028 (resolve comments)
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
<<<<<<< HEAD
<<<<<<< HEAD
      selectedCommandCallback.setCallbackDelegate(object : ClusterCommandCallback {
        override fun onSuccess(responseValues: Map<CommandResponseInfo, Any>) {
=======
      selectedCommandCallback!!.setCallbackDelegate(object : ClusterCommandCallback {
<<<<<<< HEAD
        override fun onSuccess(responseValues: Map<String, Any>) {
>>>>>>> d3d83a0bc (select different cluster, command will remove previous displayed parameter)
=======
        override fun onSuccess(responseValues: Map<ResponseValueInfo, Any>) {
>>>>>>> 7a1c5d850 (add responseValueInfo class instead of string split)
=======
      selectedCommandCallback.setCallbackDelegate(object : ClusterCommandCallback {
        override fun onSuccess(responseValues: Map<CommandResponseInfo, Any>) {
>>>>>>> cd1c99028 (resolve comments)
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
<<<<<<< HEAD
<<<<<<< HEAD
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      param.clusterParameterNameTv.text = "${paramName}"
      param.clusterParameterTypeTv.text = "${paramInfo.type}"
=======
      val param = inflater.inflate(R.layout.parameter_item, null, false) as ConstraintLayout
      param.parameterName.text = "${paramName}"
      param.parameterType.text = "${paramInfo.type}"
>>>>>>> 78cf954d9 (fix parameter response ui alignment issue)
=======
      val param = inflater.inflate(R.layout.cluster_parameter_item, null, false) as ConstraintLayout
      param.clusterParameterNameTv.text = "${paramName}"
      param.clusterParameterTypeTv.text = "${paramInfo.type}"
>>>>>>> cd1c99028 (resolve comments)
      parameterList.addView(param)
    }
  }

  private fun populateCallbackResult(
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    responseValues: Map<CommandResponseInfo, Any>,
=======
    responseValues: Map<String, Any>,
>>>>>>> 7c783e227 (add back class description)
=======
    responseValues: Map<ResponseValueInfo, Any>,
>>>>>>> 7a1c5d850 (add responseValueInfo class instead of string split)
=======
    responseValues: Map<CommandResponseInfo, Any>,
>>>>>>> cd1c99028 (resolve comments)
    inflater: LayoutInflater,
    callbackList: LinearLayout
  ) {
    responseValues.forEach { (variableNameType, response) ->
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> cd1c99028 (resolve comments)
      val callback =
        inflater.inflate(R.layout.cluster_callback_item, null, false) as ConstraintLayout
      callback.clusterCallbackNameTv.text = variableNameType.name
      callback.clusterCallbackDataTv.text = response.toString()
      callback.clusterCallbackTypeTv.text = variableNameType.type
<<<<<<< HEAD
=======
      val callback = inflater.inflate(R.layout.callback_item, null, false) as LinearLayout
=======
      val callback = inflater.inflate(R.layout.callback_item, null, false) as ConstraintLayout
>>>>>>> 78cf954d9 (fix parameter response ui alignment issue)
      callback.callbackName.text = variableNameType.name
      callback.callbackData.text = response.toString()
      callback.callbackType.text = variableNameType.type
>>>>>>> 7a1c5d850 (add responseValueInfo class instead of string split)
=======
>>>>>>> cd1c99028 (resolve comments)
      callbackList.addView(callback)
    }
  }

<<<<<<< HEAD
<<<<<<< HEAD
  private fun getCommandOptions(
=======
  private fun getCommand(
>>>>>>> 7c783e227 (add back class description)
=======
  private fun getCommandOptions(
>>>>>>> d3d83a0bc (select different cluster, command will remove previous displayed parameter)
    clusterName: String
  ): ArrayAdapter<String> {
    selectedClusterInfo = clusterMap[clusterName]!!
    val commandNameList = constructHint(selectedClusterInfo.commands)
    return ArrayAdapter(requireContext(), android.R.layout.simple_list_item_1, commandNameList)
  }

<<<<<<< HEAD
<<<<<<< HEAD
  private fun constructHint(clusterMap: Map<String, *>): Array<String> {
    return clusterMap.keys.toTypedArray()
<<<<<<< HEAD
=======
  private fun constructHint(clusterMap: HashMap<String, *>): Array<String> {
=======
  private fun constructHint(clusterMap: Map<String, *>): Array<String> {
>>>>>>> cd1c99028 (resolve comments)
    val clusterName = mutableListOf<String>()
    for ((name, info) in clusterMap) {
      clusterName.add(name)
    }
    return clusterName.toTypedArray()
>>>>>>> d3d83a0bc (select different cluster, command will remove previous displayed parameter)
=======
>>>>>>> d419b4060 (resolve comments)
  }

  override fun onStop() {
    super.onStop()
    scope.cancel()
  }

  companion object {
    private const val TAG = "ClusterDetailFragment"
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
    private const val ENDPOINT_ID_KEY = "endpoint_id"
    private const val DEVICE_PTR_KEY = "device_ptr"

    fun newInstance(
      deviceId: Long,
      endpointId: Int
=======
    private const val CLUSTER_MAP_INFO = "cluster_map_info"
    private const val DEVICE_PTR = "device_ptr"
    fun newInstance(
      clusterMap: HashMap<String, ClusterInfo>,
      deviceId: Long
>>>>>>> 7c783e227 (add back class description)
    ): ClusterDetailFragment {
      return ClusterDetailFragment().apply {
        arguments = Bundle(2).apply {
          putLong(DEVICE_PTR_KEY, deviceId)
          putInt(ENDPOINT_ID_KEY, endpointId)
=======
    private const val ENDPOINT_ID = "endpoint_id"
    private const val DEVICE_PTR = "device_ptr"
=======
    private const val ENDPOINT_ID_KEY = "endpoint_id"
    private const val DEVICE_PTR_KEY = "device_ptr"

>>>>>>> d419b4060 (resolve comments)
    fun newInstance(
      deviceId: Long,
      endpointId: Int
    ): ClusterDetailFragment {
      return ClusterDetailFragment().apply {
        arguments = Bundle(2).apply {
<<<<<<< HEAD
          putLong(DEVICE_PTR, deviceId)
          putInt(ENDPOINT_ID, endpointId)
>>>>>>> cd1c99028 (resolve comments)
=======
          putLong(DEVICE_PTR_KEY, deviceId)
          putInt(ENDPOINT_ID_KEY, endpointId)
>>>>>>> d419b4060 (resolve comments)
        }
      }
    }
  }
}