package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import chip.clusterinfo.ClusterInfo
import chip.devicecontroller.ChipDeviceController
import chip.devicecontroller.ClusterInfoMapping
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import com.google.chip.chiptool.clusterclient.AddressUpdateFragment
import kotlinx.android.synthetic.main.cluster_interaction_fragment.view.endpointList
import kotlinx.android.synthetic.main.cluster_interaction_fragment.view.getEndpointListBtn
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.launch

class ClusterInteractionFragment : Fragment() {
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope
  private lateinit var addressUpdateFragment: AddressUpdateFragment
  private lateinit var clusterMap: Map<String, ClusterInfo>

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    scope = viewLifecycleOwner.lifecycleScope

    return inflater.inflate(R.layout.cluster_interaction_fragment, container, false).apply {
      deviceController.setCompletionListener(ChipControllerCallback())
      getEndpointListBtn.setOnClickListener {
        scope.launch {
          showMessage("Retrieving endpoints")
          endpointList.visibility = View.VISIBLE
        }
      }

      addressUpdateFragment =
        childFragmentManager.findFragmentById(R.id.addressUpdateFragment) as AddressUpdateFragment
      clusterMap = ClusterInfoMapping().clusterMap
      var dataList: List<EndpointItem> = ArrayList()
      // TODO: Dynamically retrieve endpoint information using descriptor cluster
      // hardcode the endpoint for now
      for (i in 0 until 2) {
        dataList += EndpointItem(i)
      }
      endpointList.adapter = EndpointAdapter(dataList, EndpointListener())
      endpointList.layoutManager = LinearLayoutManager(requireContext())
    }
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
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

  companion object {
    private const val TAG = "ClusterInteractionFragment"
    fun newInstance(): ClusterInteractionFragment = ClusterInteractionFragment()
  }

  private fun showFragment(fragment: Fragment, showOnBack: Boolean = true) {
    val fragmentTransaction = requireActivity().supportFragmentManager
      .beginTransaction()
      .replace(R.id.fragment_container, fragment, fragment.javaClass.simpleName)

    if (showOnBack) {
      fragmentTransaction.addToBackStack(null)
    }

    fragmentTransaction.commit()
  }

  inner class EndpointListener : EndpointAdapter.OnItemClickListener {
    override fun onItemClick(position: Int) {
      Toast.makeText(requireContext(), "Item $position clicked", Toast.LENGTH_SHORT).show()
      showFragment(ClusterDetailFragment.newInstance())
    }
  }
}