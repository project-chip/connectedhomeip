package com.samsung.matter.chipstdeviceapp.feature.main.model

import androidx.annotation.DrawableRes
import androidx.annotation.StringRes
import com.samsung.matter.chipstdeviceapp.core.common.Device
import com.samsung.matter.chipstdeviceapp.feature.main.R

enum class Menu(
    @DrawableRes val iconResId: Int,
    @StringRes val titleResId: Int,
    val isDim: Boolean = false,
    val device: Device = Device.Unknown
) {
    ON_OFF_SWITCH(
        R.drawable.round_toggle_on_24,
        R.string.samsung_on_off_switch,
        device = Device.OnOffSwitch
    )
}
