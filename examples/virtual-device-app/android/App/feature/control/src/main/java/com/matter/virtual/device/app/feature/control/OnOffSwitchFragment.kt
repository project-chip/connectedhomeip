package com.matter.virtual.device.app.feature.control

import androidx.fragment.app.viewModels
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.model.databinding.ButtonData
import com.matter.virtual.device.app.core.ui.BaseFragment
import com.matter.virtual.device.app.core.ui.databinding.LayoutAppbarBinding
import com.matter.virtual.device.app.feature.control.databinding.FragmentOnOffSwitchBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class OnOffSwitchFragment :
  BaseFragment<FragmentOnOffSwitchBinding, OnOffSwitchViewModel>(R.layout.fragment_on_off_switch) {

  override val viewModel: OnOffSwitchViewModel by viewModels()

  @OptIn(ExperimentalSerializationApi::class)
  override fun setupNavArgs() {
    val args: OnOffSwitchFragmentArgs by navArgs()
    matterSettings = Json.decodeFromString(args.setting)
  }

  override fun setupAppbar(): LayoutAppbarBinding = binding.appbar

  override fun setupUi() {
    /** title text */
    binding.appbar.toolbarTitle.text = getString(matterSettings.device.deviceNameResId)

    /** OnOff layout */
    binding.onOffSwitchOnOffLayout.buttonData =
      ButtonData(
        onOff = viewModel.onOff,
        onText = R.string.on_off_switch_power_on,
        offText = R.string.on_off_switch_power_off
      )

    binding.onOffSwitchOnOffLayout.button.setOnClickListener { viewModel.onClickButton() }
  }

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
