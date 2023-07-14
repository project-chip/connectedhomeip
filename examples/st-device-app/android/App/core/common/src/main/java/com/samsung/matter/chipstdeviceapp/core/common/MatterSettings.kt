package com.samsung.matter.chipstdeviceapp.core.common

import com.samsung.matter.chipstdeviceapp.core.model.OnboardingType
import kotlinx.serialization.Serializable

@Serializable
data class MatterSettings(
  var device: Device = Device.Unknown,
  var onboardingType: OnboardingType = OnboardingType.UNKNOWN,
  var discriminator: Int = MatterConstants.DEFAULT_DISCRIMINATOR
)
