package com.matter.virtual.device.app.feature.main

import android.content.Context
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import androidx.navigation.fragment.findNavController
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.feature.main.databinding.FragmentLoadingBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlin.math.abs
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class LoadingFragment : Fragment() {

  private lateinit var binding: FragmentLoadingBinding

  private lateinit var matterSettings: MatterSettings
  private lateinit var onBackPressedCallback: OnBackPressedCallback

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    Timber.d("Hit")
    binding = DataBindingUtil.inflate(inflater, R.layout.fragment_loading, container, false)
    binding.lifecycleOwner = viewLifecycleOwner

    return binding.root
  }

  @OptIn(ExperimentalSerializationApi::class)
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    Timber.d("Hit")
    super.onViewCreated(view, savedInstanceState)

    (activity as AppCompatActivity).setSupportActionBar(binding.toolbar)
    (activity as AppCompatActivity).supportActionBar?.setDisplayHomeAsUpEnabled(true)

    binding.appBarLayout.addOnOffsetChangedListener { appBarLayout, verticalOffset ->
      var ratio = 0F
      if (abs(verticalOffset) != 0) {
        ratio = abs(verticalOffset).toFloat() / appBarLayout.totalScrollRange.toFloat()
      }

      binding.collapseTitle.alpha = 1f - ratio * 2f + 0.1f
      binding.toolbarTitle.alpha = (ratio - 0.5f) * 2f + 0.1f
    }

    val args: LoadingFragmentArgs by navArgs()
    this.matterSettings = Json.decodeFromString(args.setting)
  }

  override fun onResume() {
    Timber.d("Hit")
    super.onResume()
  }

  override fun onAttach(context: Context) {
    Timber.d("Hit")
    super.onAttach(context)

    onBackPressedCallback =
      object : OnBackPressedCallback(true) {
        override fun handleOnBackPressed() {
          Timber.d("handleOnBackPressed()")
          findNavController().popBackStack()
        }
      }

    requireActivity().onBackPressedDispatcher.addCallback(this, onBackPressedCallback)
  }

  override fun onDetach() {
    Timber.d("Hit")
    super.onDetach()
    onBackPressedCallback.remove()
  }

  override fun onDestroy() {
    Timber.d("Hit")
    super.onDestroy()
  }
}
