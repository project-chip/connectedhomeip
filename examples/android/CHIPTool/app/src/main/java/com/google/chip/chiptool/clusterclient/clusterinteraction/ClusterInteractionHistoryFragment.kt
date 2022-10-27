package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.LinearLayoutManager
import com.google.chip.chiptool.R
import kotlinx.android.synthetic.main.cluster_interaction_history_fragment.view.historyCommandList

/**
 * A simple [Fragment] subclass for the cluster interaction history component
 * Use the [ClusterInteractionHistoryFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class ClusterInteractionHistoryFragment : Fragment() {

  override fun onCreateView(
    inflater: LayoutInflater, container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    // Inflate the layout for this fragment
    Log.d(TAG, clusterInteractionHistoryList.toString())
    return inflater.inflate(R.layout.cluster_interaction_history_fragment, container, false).apply {
      historyCommandList.adapter =
        HistoryCommandAdapter(clusterInteractionHistoryList, HistoryCommandListener(), inflater)
      historyCommandList.layoutManager = LinearLayoutManager(requireContext())
    }
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

  companion object {
    private const val TAG = "ClusterInteractionHistoryFragment"
    // The history list is a most-recent-first, therefore adding the most recent executed
    // command on the top of the list
    val clusterInteractionHistoryList = ArrayDeque<HistoryCommand>()
    fun newInstance() =
      ClusterInteractionHistoryFragment()
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