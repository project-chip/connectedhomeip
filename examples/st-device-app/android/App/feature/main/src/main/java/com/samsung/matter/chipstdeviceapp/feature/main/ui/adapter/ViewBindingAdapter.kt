package com.samsung.matter.chipstdeviceapp.feature.main.ui.adapter

import androidx.annotation.DrawableRes
import androidx.appcompat.widget.AppCompatImageView
import androidx.databinding.BindingAdapter

@BindingAdapter("setSrc")
fun AppCompatImageView.bindSetSrc(@DrawableRes imgResId: Int) {
  this.setImageResource(imgResId)
}
