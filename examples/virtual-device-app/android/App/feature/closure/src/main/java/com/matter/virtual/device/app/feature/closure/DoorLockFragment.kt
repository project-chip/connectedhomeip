package com.matter.virtual.device.app.feature.closure

import androidx.fragment.app.viewModels
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.ui.BaseFragment
import com.matter.virtual.device.app.core.ui.databinding.LayoutAppbarBinding
import com.matter.virtual.device.app.feature.closure.databinding.FragmentDoorLockBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class DoorLockFragment :
  BaseFragment<FragmentDoorLockBinding, DoorLockViewModel>(R.layout.fragment_door_lock) {

  override val viewModel: DoorLockViewModel by viewModels()

  @OptIn(ExperimentalSerializationApi::class)
  override fun setupNavArgs() {
    val args: DoorLockFragmentArgs by navArgs()
    matterSettings = Json.decodeFromString(args.setting)
  }

  override fun setupAppbar(): LayoutAppbarBinding = binding.appbar

  override fun setupUi() {}

  override fun setupObservers() {}

  override fun onResume() {
    Timber.d("onResume()")
    super.onResume()
  }

  override fun onDestroy() {
    Timber.d("onDestroy()")
    super.onDestroy()
  }
}
