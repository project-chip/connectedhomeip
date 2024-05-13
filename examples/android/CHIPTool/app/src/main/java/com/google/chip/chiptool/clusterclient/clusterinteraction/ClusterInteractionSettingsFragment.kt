package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.google.chip.chiptool.R

/**
 * A simple [Fragment] subclass for cluster interaction settings component. Use the
 * [ClusterInteractionSettingsFragment.newInstance] factory method to create an instance of this
 * fragment.
 */
class ClusterInteractionSettingsFragment : Fragment() {

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.cluster_interaction_settings_fragment, container, false)
  }

  companion object {
    fun newInstance() = ClusterInteractionSettingsFragment()
  }
}
