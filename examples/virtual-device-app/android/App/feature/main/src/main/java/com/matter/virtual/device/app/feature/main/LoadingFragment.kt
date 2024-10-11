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
import androidx.fragment.app.activityViewModels
import androidx.fragment.app.viewModels
import androidx.navigation.NavOptions
import androidx.navigation.fragment.findNavController
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.common.DeepLink
import com.matter.virtual.device.app.core.common.EventObserver
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.ui.SharedViewModel
import com.matter.virtual.device.app.feature.main.databinding.FragmentLoadingBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class LoadingFragment : Fragment() {

  private lateinit var binding: FragmentLoadingBinding
  private val viewModel by viewModels<LoadingViewModel>()
  private val sharedViewModel by activityViewModels<SharedViewModel>()

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

    val args: LoadingFragmentArgs by navArgs()
    this.matterSettings = Json.decodeFromString(args.setting)

    viewModel.uiState.observe(
      viewLifecycleOwner,
      EventObserver { uiStete ->
        when (uiStete) {
          is LoadingUiState.Complete -> {
            matterSettings.device = uiStete.device

            try {
              val navOptions = NavOptions.Builder().setPopUpTo(R.id.mainFragment, true).build()

              findNavController()
                .navigate(
                  DeepLink.getDeepLinkRequestFromDevice(
                    uiStete.device,
                    Json.encodeToString(matterSettings)
                  ),
                  navOptions
                )
            } catch (e: Exception) {
              Timber.e(e, "navigate failure")
            }
          }
          LoadingUiState.Failure -> {
            Timber.e("Failure or Timeout")
            sharedViewModel.requestFactoryReset(
              messageResId = R.string.dialog_timeout_factory_reset_message,
              isCancelable = false
            )
          }
          LoadingUiState.Loading -> {}
        }
      }
    )
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
          sharedViewModel.requestFactoryReset(
            messageResId = R.string.dialog_user_factory_reset_message,
            isCancelable = true
          )
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
