package com.matter.virtual.device.app.feature.qrcode

import android.content.Context
import android.graphics.Bitmap
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.activity.OnBackPressedCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.databinding.DataBindingUtil
import androidx.fragment.app.Fragment
import androidx.fragment.app.viewModels
import androidx.navigation.fragment.findNavController
import androidx.navigation.fragment.navArgs
import com.matter.virtual.device.app.core.common.MatterConstants
import com.matter.virtual.device.app.core.common.MatterSettings
import com.matter.virtual.device.app.core.common.QrcodeUtil
import com.matter.virtual.device.app.feature.qrcode.databinding.FragmentQrcodeBinding
import dagger.hilt.android.AndroidEntryPoint
import kotlin.math.abs
import kotlinx.serialization.ExperimentalSerializationApi
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.json.Json
import timber.log.Timber

@AndroidEntryPoint
class QrcodeFragment : Fragment() {

  private lateinit var binding: FragmentQrcodeBinding
  private val viewModel by viewModels<QrcodeViewModel>()

  private lateinit var onBackPressedCallback: OnBackPressedCallback

  override fun onCreateView(
    inflater: LayoutInflater,
    container: ViewGroup?,
    savedInstanceState: Bundle?
  ): View {
    Timber.d("Hit")
    binding = DataBindingUtil.inflate(inflater, R.layout.fragment_qrcode, container, false)
    binding.lifecycleOwner = viewLifecycleOwner
    binding.viewModel = viewModel

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

    val args: QrcodeFragmentArgs by navArgs()
    val matterSettings = Json.decodeFromString<MatterSettings>(args.setting)

    viewModel.uiState.observe(viewLifecycleOwner) {
      when (it) {
        QrcodeUiState.Loading -> {
          binding.progressBar.visibility = View.VISIBLE
        }
        is QrcodeUiState.Qrcode -> {
          binding.progressBar.visibility = View.GONE

          binding.qrTypeImage.setImageResource(matterSettings.device.deviceIconResId)
          binding.qrTypeTitle.text = getString(matterSettings.device.deviceNameResId)

          val qrCodeBitmap: Bitmap? = QrcodeUtil.createQrCodeBitmap(it.qrCode, QR_WIDTH, QR_HEIGHT)
          binding.qrImage.setImageBitmap(qrCodeBitmap)

          binding.qrText.text = getString(R.string.qrcode_qr_text, it.qrCode)
          binding.manualCodeText.text =
            getString(R.string.qrcode_manual_code_text, it.manualPairingCode)
          binding.versionText.text =
            getString(R.string.qrcode_version_text, MatterConstants.DEFAULT_VERSION.toString())
          binding.vendorIdText.text =
            getString(
              R.string.qrcode_vendor_id_text,
              MatterConstants.DEFAULT_VENDOR_ID.toString(),
              MatterConstants.DEFAULT_VENDOR_ID
            )
          binding.productIdText.text =
            getString(
              R.string.qrcode_product_id_text,
              MatterConstants.DEFAULT_PRODUCT_ID.toString(),
              MatterConstants.DEFAULT_PRODUCT_ID
            )
          binding.commissioningFlowText.text =
            getString(
              R.string.qrcode_commissioning_flow_text,
              MatterConstants.DEFAULT_COMMISSIONING_FLOW.toString()
            )
          binding.onboardingTypeText.text =
            getString(R.string.qrcode_onboarding_type_text, matterSettings.onboardingType)
          binding.setupPinCodeText.text =
            getString(
              R.string.qrcode_setup_pin_code_text,
              MatterConstants.DEFAULT_SETUP_PINCODE.toString()
            )
          binding.discriminatorText.text =
            getString(
              R.string.qrcode_discriminator_text,
              matterSettings.discriminator.toString(),
              matterSettings.discriminator
            )
        }
        else -> {}
      }
    }
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

  companion object {
    private const val QR_WIDTH = 450
    private const val QR_HEIGHT = 450
  }
}
