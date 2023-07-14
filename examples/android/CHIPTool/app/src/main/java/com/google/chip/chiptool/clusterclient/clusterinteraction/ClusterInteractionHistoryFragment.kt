package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.chip.chiptool.R
import com.google.chip.chiptool.databinding.ClusterInteractionHistoryFragmentBinding

/**
 * A simple [Fragment] subclass for the cluster interaction history component Use the
 * [ClusterInteractionHistoryFragment.newInstance] factory method to create an instance of this
 * fragment.
 */
class ClusterInteractionHistoryFragment : Fragment() {
  private var _binding: ClusterInteractionHistoryFragmentBinding? = null
  private val binding
    get() = _binding!!

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    _binding = ClusterInteractionHistoryFragmentBinding.inflate(inflater, container, false)
    Log.d(TAG, clusterInteractionHistoryList.toString())
    binding.historyCommandList.adapter =
      HistoryCommandAdapter(clusterInteractionHistoryList, HistoryCommandListener(), inflater)
    binding.historyCommandList.layoutManager = LinearLayoutManager(requireContext())

    return binding.root
  }

  override fun onDestroyView() {
    super.onDestroyView()
    _binding = null
  }

  private fun showFragment(fragment: Fragment, showOnBack: Boolean = true) {
    val fragmentTransaction =
      requireActivity()
        .supportFragmentManager
        .beginTransaction()
        .replace(R.id.fragment_container, fragment, fragment.javaClass.simpleName)

    if (showOnBack) {
      fragmentTransaction.addToBackStack(null)
    }

    fragmentTransaction.commit()
  }

  companion object {
    private const val TAG = "ClusterInteractionHistoryFragment"
    // The history list is a most-recent-first, therefore adding the most recent executed
    // command on the top of the list
    val clusterInteractionHistoryList = ArrayDeque<HistoryCommand>()

    fun newInstance() = ClusterInteractionHistoryFragment()
  }

  inner class HistoryCommandListener : HistoryCommandAdapter.OnItemClickListener {
    override fun onItemClick(position: Int) {
      showFragment(
        ClusterDetailFragment.newInstance(
          clusterInteractionHistoryList[position].deviceId,
          clusterInteractionHistoryList[position].endpointId,
          clusterInteractionHistoryList[position]
        )
      )
    }
  }
}
