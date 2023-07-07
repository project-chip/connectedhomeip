package com.samsung.matter.chipstdeviceapp.core.common

import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import kotlinx.serialization.Serializable

@Serializable
sealed class Device(
    val title: String,
    val productId: Long,
    @StringRes val deviceNameResId: Int,
    @DrawableRes val deviceIconResId: Int,
    val deviceTypeId: Long,
    val discriminator: Int
) {
    @Serializable
    object OnOffSwitch : Device(
        "onoffswitch",
        0x1001,
        R.string.samsung_on_off_switch,
        R.drawable.round_toggle_on_24,
        0x0103,
        3841
    )

    @Serializable
    object Unknown : Device(
        "unknown",
        0xFFFF,
        R.string.matter_device,
        R.drawable.round_device_unknown_24,
        65535,
        3840
    )

    companion object {
        fun map(title: String): Device {
            return Device::class.sealedSubclasses.firstOrNull {
                it.objectInstance?.title == title
            }?.objectInstance ?: Unknown
        }
    }
}
