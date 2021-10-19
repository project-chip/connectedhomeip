package com.google.chip.chiptool.clusterclient

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import chip.clusterinfo.ClusterCommandCallback
import chip.clusterinfo.ClusterInfo
import chip.clusterinfo.CommandInfo
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.cancel
<<<<<<< HEAD
<<<<<<< HEAD
import chip.devicecontroller.ClusterInfoMapping
import java.lang.Exception
import kotlinx.android.synthetic.main.cluster_interaction_fragment.view.getClusterMappingBtn
import kotlinx.coroutines.launch

=======
import
>>>>>>> 505e97db3 (change package)
=======
import chip.devicecontroller.ClusterInfoMapping
import java.lang.Exception
import kotlinx.android.synthetic.main.cluster_interaction_fragment.view.getClusterMappingBtn
import kotlinx.coroutines.launch
>>>>>>> a4fd0282e (no error code generation)

class ClusterInteractionFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private val scope = CoroutineScope(Dispatchers.Main + Job())
  private lateinit var addressUpdateFragment: AddressUpdateFragment
<<<<<<< HEAD
<<<<<<< HEAD
  private lateinit var clusterMap: Map<String, ClusterInfo>
=======
>>>>>>> a4fd0282e (no error code generation)
=======
  private lateinit var clusterMap: Map<String, ClusterInfo>
>>>>>>> 41b0956c1 (on/off commands working)

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    return inflater.inflate(R.layout.cluster_interaction_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())
      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
      clusterMap = ClusterInfoMapping().clusterMap;
      getClusterMappingBtn.setOnClickListener { scope.launch { getClusterMapping() } }
    }
  }

  private suspend fun getClusterMapping() {
    // In real code: "OnOff" would be selected by the user.
    val methodSelected = "onOff"
    showMessage(methodSelected + " is initialized")
    val selectedClusterInfo = clusterMap[methodSelected]!!
    val devicePtr =
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
    val endpointId = 1
    val selectedCluster = selectedClusterInfo.createClusterFunction.create(devicePtr, endpointId)
    // Imagine user wants to execute the command "OffWithEffect", pass the string here
    val selectedCommandInfo: CommandInfo = selectedClusterInfo.commands["on"]!!

    var selectedCommandCallback =  selectedCommandInfo.commandCallbackSupplier.get()
    selectedCommandCallback?.setCallbackDelegate(object : ClusterCommandCallback {
      override fun onSuccess(responseValues: List<Any>) {
        showMessage("Command success")
        // Populate UI based on response values. We know the types from CommandInfo.getCommandResponses().
        responseValues.forEach { Log.d(TAG, it.toString()) }
      }

      override fun onFailure(exception: Exception) {
        showMessage("Command failed")
        Log.e(TAG, exception.toString())
      }
    })

    var commandArguments: HashMap<String, Any> = HashMap<String, Any>()
    selectedCommandInfo.getCommandFunction().invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
=======
      getClusterMappingBtn.setOnClickListener { getClusterMapping()}
    }
  }

  private fun getClusterMapping() {
    val clusterInfoMapping = ClusterInfoMapping();
    val test = clusterInfoMapping.clusterMap;
    for ((key, value) in test.entries) {
      println("${key}=$value")
>>>>>>> a4fd0282e (no error code generation)
=======
      val clusterMapping = ClusterInfoMapping()
      clusterMap = clusterMapping.clusterMap;
      getClusterMappingBtn.setOnClickListener { scope.launch {getClusterMapping()} }
=======
      clusterMap = ClusterInfoMapping().clusterMap;
      getClusterMappingBtn.setOnClickListener { scope.launch { getClusterMapping() } }
>>>>>>> fde1d13c0 (fix comments)
    }
  }

  private suspend fun getClusterMapping() {
    // In real code: "OnOff" would be selected by the user.
    val methodSelected = "onOff"
    showMessage(methodSelected + " is initialized")
    val selectedClusterInfo = clusterMap[methodSelected]!!
    val devicePtr =
      ChipClient.getConnectedDevicePointer(requireContext(), addressUpdateFragment.deviceId)
    val endpointId = 1
    val selectedCluster = selectedClusterInfo.createClusterFunction.create(devicePtr, endpointId)
    // Imagine user wants to execute the command "OffWithEffect", pass the string here
    val selectedCommandInfo: CommandInfo = selectedClusterInfo.commands["on"]!!

    var selectedCommandCallback =  selectedCommandInfo.commandCallbackSupplier.get()
    selectedCommandCallback?.setCallbackDelegate(object : ClusterCommandCallback {
      override fun onSuccess(responseValues: List<Any>) {
        showMessage("Command success")
        // Populate UI based on response values. We know the types from CommandInfo.getCommandResponses().
        responseValues.forEach { Log.d(TAG, it.toString()) }
      }

      override fun onFailure(exception: Exception?) {
        showMessage("Command failed")
        Log.e(TAG, exception.toString())
      }
    })

    var commandArguments: HashMap<String, Any> = HashMap<String, Any>()
    selectedCommandInfo.getCommandFunction().invokeCommand(selectedCluster, selectedCommandCallback, commandArguments)
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
>>>>>>> 41b0956c1 (on/off commands working)
    }
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
    private const val TAG = "ClusterInteractionFragment"
    fun newInstance(): ClusterInteractionFragment = ClusterInteractionFragment()
  }
}