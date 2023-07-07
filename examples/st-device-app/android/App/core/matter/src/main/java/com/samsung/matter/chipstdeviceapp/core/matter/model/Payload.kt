package com.samsung.matter.chipstdeviceapp.core.matter.model

import chip.onboardingpayload.DiscoveryCapability
import chip.onboardingpayload.OnboardingPayload
import com.samsung.matter.chipstdeviceapp.core.common.MatterConstants
import com.samsung.matter.chipstdeviceapp.core.model.OnboardingType
import com.samsung.matter.chipstdeviceapp.core.model.Payload
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
        productId,
        MatterConstants.DEFAULT_COMMISSIONING_FLOW,
        discoveryCapabilities,
        discriminator,
        MatterConstants.DEFAULT_SETUP_PINCODE
    )
}