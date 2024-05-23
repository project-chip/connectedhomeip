package com.matter.virtual.device.app.feature.closure

import android.text.Html
import android.widget.SeekBar
import androidx.appcompat.app.AlertDialog
import androidx.fragment.app.viewModels
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.model.databinding.SeekbarData
import com.matter.virtual.device.app.core.model.matter.LockState
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
  private var lockState: LockState = LockState.LOCKED

  @OptIn(ExperimentalSerializationApi::class)
  override fun setupNavArgs() {
    val args: DoorLockFragmentArgs by navArgs()
    matterSettings = Json.decodeFromString(args.setting)
  }

  override fun setupAppbar(): LayoutAppbarBinding = binding.appbar

  override fun setupUi() {
    /** title text */
    binding.appbar.toolbarTitle.text = getString(R.string.matter_door_lock)

    /** LockState layout */
    binding.doorLockLockStateLayout.titleText.text = getString(R.string.door_lock_lock_state)
    binding.doorLockLockStateLayout.button.setImageResource(R.drawable.round_settings_24)
    binding.doorLockLockStateLayout.button.setOnClickListener { showLockStatePopup() }

    /** Send alarm layout */
    binding.doorLockSendAlarmLayout.valueText.text =
      getString(R.string.door_lock_send_lock_alarm_event)
    binding.doorLockSendAlarmLayout.button.setImageResource(
      R.drawable.round_radio_button_checked_24
    )
    binding.doorLockSendAlarmLayout.button.setOnClickListener {
      viewModel.onClickSendLockAlarmEventButton()
    }

    /** Battery layout */
    binding.doorLockBatteryLayout.titleText.text = getString(R.string.battery)
    binding.doorLockBatteryLayout.seekbarData =
      SeekbarData(progress = viewModel.batteryRemainingPercentage)
    binding.doorLockBatteryLayout.seekbar.setOnSeekBarChangeListener(
      object : SeekBar.OnSeekBarChangeListener {
        override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {
          viewModel.updateBatterySeekbarProgress(progress)
        }

        override fun onStartTrackingTouch(seekBar: SeekBar) {}

        override fun onStopTrackingTouch(seekBar: SeekBar) {
          viewModel.updateBatteryStatusToCluster(seekBar.progress)
        }
      }
    )
  }

  override fun setupObservers() {
    viewModel.lockState.observe(viewLifecycleOwner) {
      this.lockState = it
      binding.doorLockLockStateLayout.valueText.text = it.toString()
    }

    viewModel.batteryRemainingPercentage.observe(viewLifecycleOwner) {
      val text: String = getString(R.string.battery_format, it)
      binding.doorLockBatteryLayout.valueText.text = Html.fromHtml(text, Html.FROM_HTML_MODE_LEGACY)
    }
  }

  override fun onResume() {
    Timber.d("onResume()")
    super.onResume()
  }

  override fun onDestroy() {
    Timber.d("onDestroy()")
    super.onDestroy()
  }

  private fun showLockStatePopup() {
    val list =
      arrayOf(
        LockState.NOT_FULLY_LOCKED.toString(),
        LockState.LOCKED.toString(),
        LockState.UNLOCKED.toString(),
        LockState.UNLATCHED.toString()
      )

    AlertDialog.Builder(requireContext())
      .setTitle(R.string.door_lock_lock_state)
      .setSingleChoiceItems(list, list.indexOf(this.lockState.toString())) { dialog, which ->
        viewModel.setLockState(enumValueOf(list[which]))
        dialog.dismiss()
      }
      .setNegativeButton(R.string.cancel, null)
      .show()
  }
}
