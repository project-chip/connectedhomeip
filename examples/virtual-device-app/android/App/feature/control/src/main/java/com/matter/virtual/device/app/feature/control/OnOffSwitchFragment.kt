package com.matter.virtual.device.app.feature.control

import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import com.matter.virtual.device.app.feature.control.databinding.FragmentOnOffSwitchBinding
import dagger.hilt.android.AndroidEntryPoint
import timber.log.Timber

@AndroidEntryPoint
class OnOffSwitchFragment : Fragment() {

  private lateinit var binding: FragmentOnOffSwitchBinding

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    Timber.d("onCreateView()")
    binding = DataBindingUtil.inflate(inflater, R.layout.fragment_on_off_switch, container, false)
    binding.lifecycleOwner = viewLifecycleOwner

    return binding.root
  }

  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    Timber.d("onViewCreated()")
    super.onViewCreated(view, savedInstanceState)
  }

  override fun onResume() {
    Timber.d("onResume()")
    super.onResume()
  }

  override fun onDestroy() {
    Timber.d("onDestroy()")
    super.onDestroy()
  }
}
