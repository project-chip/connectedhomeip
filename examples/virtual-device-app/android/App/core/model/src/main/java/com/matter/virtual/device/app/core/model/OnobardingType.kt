package com.matter.virtual.device.app.core.model

enum class OnboardingType {
  WIFI,
  BLE,
  WIFI_BLE,
  NFC, /* Indicates if NFC-based Commissioning is supported */
  UNKNOWN
}

fun Int.asOnboardingType() =
  when (this) {
    0 -> OnboardingType.WIFI
    1 -> OnboardingType.BLE
    2 -> OnboardingType.WIFI_BLE
    else -> OnboardingType.UNKNOWN
  }
