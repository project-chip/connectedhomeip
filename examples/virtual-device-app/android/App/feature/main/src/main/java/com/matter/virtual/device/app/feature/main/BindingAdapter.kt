package com.matter.virtual.device.app.feature.main

import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.databinding.BindingAdapter

object BindingAdapter {
  @BindingAdapter("imageSrc")
  @JvmStatic
  fun AppCompatImageView.bindImageSrc(@DrawableRes imgResId: Int) {
    this.setImageResource(imgResId)
  }
}
