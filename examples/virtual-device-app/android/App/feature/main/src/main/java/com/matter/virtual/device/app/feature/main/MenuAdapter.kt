package com.matter.virtual.device.app.feature.main

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.DiffUtil
import androidx.recyclerview.widget.ListAdapter
import androidx.recyclerview.widget.RecyclerView
import com.matter.virtual.device.app.feature.main.databinding.ItemMenuBinding
import com.matter.virtual.device.app.feature.main.model.Menu

internal class MenuAdapter(private val itemHandler: ItemHandler) :
  ListAdapter<Menu, MenuAdapter.MenuViewHolder>(
    object : DiffUtil.ItemCallback<Menu>() {
      override fun areItemsTheSame(oldItem: Menu, newItem: Menu): Boolean {
        return oldItem.titleResId == newItem.titleResId
      }

      override fun areContentsTheSame(oldItem: Menu, newItem: Menu): Boolean {
        return oldItem == newItem
      }
    }
  ) {

  inner class MenuViewHolder(private val binding: ItemMenuBinding) :
    RecyclerView.ViewHolder(binding.root) {
    fun bind(item: Menu) {
      binding.item = item
      binding.itemHandler = itemHandler
    }
  }

  override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): MenuViewHolder {
    val layoutInflater = LayoutInflater.from(parent.context)
    val binding = ItemMenuBinding.inflate(layoutInflater, parent, false)
    return MenuViewHolder(binding)
  }

  override fun onBindViewHolder(holder: MenuViewHolder, position: Int) {
    val item = getItem(position)
    holder.bind(item)
  }

  interface ItemHandler {
    fun onClick(item: Menu)
  }
}
