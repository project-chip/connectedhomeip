package com.matter.virtual.device.app.core.matter.model

import com.matter.virtual.device.app.core.common.MatterConstants
import com.matter.virtual.device.app.core.model.OnboardingType
import com.matter.virtual.device.app.core.model.Payload
import matter.onboardingpayload.DiscoveryCapability
import matter.onboardingpayload.OnboardingPayload
import timber.log.Timber

fun Payload.asSetupPayload(): OnboardingPayload {
  val discoveryCapabilities = HashSet<DiscoveryCapability>()
  when (this.onboardingType) {
    OnboardingType.WIFI -> discoveryCapabilities.add(DiscoveryCapability.ON_NETWORK)
    OnboardingType.BLE -> discoveryCapabilities.add(DiscoveryCapability.BLE)
    OnboardingType.WIFI_BLE -> {
      discoveryCapabilities.add(DiscoveryCapability.ON_NETWORK)
      discoveryCapabilities.add(DiscoveryCapability.BLE)
    }
    else -> {
      Timber.e("Unknown Type")
    }
  }

  return OnboardingPayload(
    MatterConstants.DEFAULT_VERSION,
    MatterConstants.DEFAULT_VENDOR_ID,
    MatterConstants.DEFAULT_PRODUCT_ID,
    MatterConstants.DEFAULT_COMMISSIONING_FLOW,
    discoveryCapabilities,
    discriminator,
    MatterConstants.DEFAULT_SETUP_PINCODE
  )
}
