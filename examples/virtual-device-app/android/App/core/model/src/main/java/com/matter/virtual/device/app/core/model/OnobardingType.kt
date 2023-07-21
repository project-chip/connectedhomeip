package com.matter.virtual.device.app.core.model

enum class OnboardingType {
  WIFI,
  BLE,
  WIFI_BLE,
  UNKNOWN
}

fun Int.asOnboardingType() =
  when (this) {
    0 -> OnboardingType.WIFI
    1 -> OnboardingType.BLE
    2 -> OnboardingType.WIFI_BLE
    else -> OnboardingType.UNKNOWN
  }
