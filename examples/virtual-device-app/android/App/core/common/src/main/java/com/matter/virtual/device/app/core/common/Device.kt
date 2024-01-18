package com.matter.virtual.device.app.core.common

import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import kotlinx.serialization.Serializable

@Serializable
sealed class Device(
  val title: String,
  @StringRes val deviceNameResId: Int,
  @DrawableRes val deviceIconResId: Int,
  val deviceTypeId: Long,
  val discriminator: Int
) {
  @Serializable
  object OnOffSwitch :
    Device(
      "onoffswitch",
      R.string.matter_on_off_switch,
      R.drawable.round_toggle_on_24,
      0x0103,
      3841
    )

  @Serializable
  object Unknown :
    Device("unknown", R.string.matter_device, R.drawable.round_device_unknown_24, 65535, 3840)

  companion object {
    fun map(title: String): Device {
      return Device::class
        .sealedSubclasses
        .firstOrNull { it.objectInstance?.title == title }
        ?.objectInstance
        ?: Unknown
    }
  }
}
