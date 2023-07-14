package com.samsung.matter.chipstdeviceapp.feature.main.ui.recyclerview

import androidx.recyclerview.widget.DiffUtil

class ItemDiffCallback<T : Any>(
  val onItemsTheSame: (T, T) -> Boolean,
  val onContentsTheSame: (T, T) -> Boolean
) : DiffUtil.ItemCallback<T>() {

  override fun areItemsTheSame(oldItem: T, newItem: T): Boolean = onItemsTheSame(oldItem, newItem)

  override fun areContentsTheSame(oldItem: T, newItem: T): Boolean =
    onContentsTheSame(oldItem, newItem)
}
