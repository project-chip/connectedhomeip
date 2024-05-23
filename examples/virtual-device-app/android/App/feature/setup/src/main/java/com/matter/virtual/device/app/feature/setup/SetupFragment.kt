package com.matter.virtual.device.app.feature.setup

import android.os.Bundle
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import androidx.appcompat.app.AlertDialog
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.lifecycle.lifecycleScope
import androidx.navigation.fragment.findNavController
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.common.DeepLink
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.model.OnboardingType
import com.matter.virtual.device.app.feature.setup.databinding.DialogSetupContinueBinding
import com.matter.virtual.device.app.feature.setup.databinding.FragmentSetupBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlinx.coroutines.launch
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class SetupFragment : Fragment() {

  private lateinit var binding: FragmentSetupBinding
  private val viewModel by viewModels<SetupViewModel>()

  private lateinit var matterSettings: MatterSettings
  private var ssid: String = "Unknown"

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    Timber.d("Hit")
    binding = DataBindingUtil.inflate(inflater, R.layout.fragment_setup, container, false)
    binding.lifecycleOwner = viewLifecycleOwner

    return binding.root
  }

  @OptIn(ExperimentalSerializationApi::class)
  override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
    Timber.d("Hit")
    super.onViewCreated(view, savedInstanceState)

    (activity as AppCompatActivity).setSupportActionBar(binding.toolbar)
    (activity as AppCompatActivity).supportActionBar?.setDisplayHomeAsUpEnabled(true)

    val args: SetupFragmentArgs by navArgs()
    this.matterSettings = Json.decodeFromString(args.setting)

    binding.setupDeviceNameValueText.text = getString(matterSettings.device.deviceNameResId)
    binding.setupDiscriminatorEditText.setText(matterSettings.device.discriminator.toString())

    binding.setupSaveButton.setOnClickListener {
      val discriminatorText = binding.setupDiscriminatorEditText.text.toString()
      if (discriminatorText.isEmpty()) {
        Toast.makeText(
            requireActivity(),
            getString(R.string.toast_discriminator),
            Toast.LENGTH_LONG
          )
          .show()
      } else {
        val discriminator = discriminatorText.toInt()
        if (discriminator > DISCRIMINATOR_LIMIT) {
          Toast.makeText(
              requireActivity(),
              getString(R.string.toast_discriminator),
              Toast.LENGTH_LONG
            )
            .show()
        } else {
          lifecycleScope.launch {
            this@SetupFragment.ssid = viewModel.getSSID()
            showConfirmDialog()
          }
        }
      }
    }
  }

  @OptIn(ExperimentalSerializationApi::class)
  private fun showConfirmDialog() {
    Timber.d("Hit")

    val dialogBinding: DialogSetupContinueBinding =
      DataBindingUtil.inflate(
        LayoutInflater.from(requireContext()),
        R.layout.dialog_setup_continue,
        null,
        false
      )

    val confirmDialog =
      AlertDialog.Builder(requireContext(), R.style.Theme_AppCompat_DayNight_Dialog_Alert)
        .setView(dialogBinding.root)
        .setCancelable(false)
        .create()

    confirmDialog.window?.let {
      it.setGravity(Gravity.BOTTOM)
      it.setBackgroundDrawableResource(R.drawable.dialog_bg)
    }

    dialogBinding.dialogSetupContinueTypeImage.setImageResource(
      this.matterSettings.device.deviceIconResId
    )
    dialogBinding.dialogSetupContinueSsidText.text = getString(R.string.setup_ssid_text, this.ssid)
    dialogBinding.dialogSetupContinueDeviceNameText.text =
      getString(this.matterSettings.device.deviceNameResId)

    val discriminator = binding.setupDiscriminatorEditText.text.toString()
    dialogBinding.dialogSetupContinueDiscriminatorText.text =
      getString(
        R.string.setup_discriminator_text,
        getString(R.string.hint_discriminator),
        discriminator
      )

    dialogBinding.dialogSetupCancelButton.setOnClickListener { confirmDialog.dismiss() }

    dialogBinding.dialogSetupStartButton.setOnClickListener {
      confirmDialog.dismiss()

      val onboardingType =
        when (binding.setupOnboardingRadioGroup.checkedRadioButtonId) {
          R.id.setup_onboarding_wifi_only -> OnboardingType.WIFI
          R.id.setup_onboarding_ble_only -> OnboardingType.BLE
          R.id.setup_onboarding_wifi_ble -> OnboardingType.WIFI_BLE
          else -> OnboardingType.WIFI
        }
      Timber.d("MatterOnboardingType:$onboardingType")
      this.matterSettings.onboardingType = onboardingType
      this.matterSettings.discriminator = discriminator.toInt()

      val jsonSettings = Json.encodeToString(this.matterSettings)
      try {
        findNavController().navigate(DeepLink.getDeepLinkRequestForQrcodeFragment(jsonSettings))
      } catch (e: Exception) {
        Timber.e(e, "navigate failure")
      }
    }

    confirmDialog.show()
  }

  companion object {
    private const val DISCRIMINATOR_LIMIT = 4095
  }
}
