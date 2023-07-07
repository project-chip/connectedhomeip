package com.samsung.matter.chipstdeviceapp.core.model

data class Payload(
    val productId: Int,
    val onboardingType: OnboardingType,
    val discriminator: Int
)