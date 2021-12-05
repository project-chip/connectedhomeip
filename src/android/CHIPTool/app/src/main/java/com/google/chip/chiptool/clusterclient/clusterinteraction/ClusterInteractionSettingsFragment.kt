package com.google.chip.chiptool.clusterclient.clusterinteraction

import android.os.Bundle
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.google.chip.chiptool.R

/**
 * A simple [Fragment] subclass for cluster interaction settings component.
 * Use the [ClusterInteractionSettingsFragment.newInstance] factory method to
 * create an instance of this fragment.
 */
class ClusterInteractionSettingsFragment : Fragment() {
  // TODO: Rename and change types of parameters
  private var param1: String? = null
  private var param2: String? = null

  override fun onCreateView(
    inflater: LayoutInflater, container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View? {
    // Inflate the layout for this fragment
    return inflater.inflate(R.layout.cluster_interaction_settings_fragment, container, false)
  }

  companion object {
    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment ClusterInteractionSettingsFragment.
     */
    // TODO: Rename and change types and number of parameters
    @JvmStatic
    fun newInstance() =
      ClusterInteractionSettingsFragment().apply {
        arguments = Bundle().apply {
        }
      }
  }
}