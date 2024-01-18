package com.matter.virtual.device.app.core.model.databinding

import androidx.annotation.StringRes
import androidx.lifecycle.LiveData

data class ButtonData(
  val onOff: LiveData<Boolean>,
  @StringRes val onText: Int,
  @StringRes val offText: Int
)
