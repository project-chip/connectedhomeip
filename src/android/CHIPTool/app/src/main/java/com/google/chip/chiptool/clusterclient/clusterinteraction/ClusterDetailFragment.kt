package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import chip.devicecontroller.ChipDeviceController
import com.google.chip.chiptool.ChipClient
import com.google.chip.chiptool.GenericChipDeviceListener
import com.google.chip.chiptool.R
import kotlinx.coroutines.CoroutineScope

/**
 * ClusterDetailFragment allows user to pick cluster, command, specify parameters and see
 * the callback result.
 */
class ClusterDetailFragment : Fragment(){
  private val deviceController: ChipDeviceController
    get() = ChipClient.getDeviceController(requireContext())

  private lateinit var scope: CoroutineScope

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    scope = viewLifecycleOwner.lifecycleScope

    return inflater.inflate(R.layout.cluster_detail_fragment, container, false).apply {
      deviceController.setCompletionListener(GenericChipDeviceListener())
    }
  }

  private fun showMessage(msg: String) {
    requireActivity().runOnUiThread {
      Toast.makeText(requireContext(), msg, Toast.LENGTH_SHORT).show()
    }
  }

  companion object {
    private const val TAG = "ClusterDetailFragment"
    fun newInstance(): ClusterDetailFragment = ClusterDetailFragment()
  }
}