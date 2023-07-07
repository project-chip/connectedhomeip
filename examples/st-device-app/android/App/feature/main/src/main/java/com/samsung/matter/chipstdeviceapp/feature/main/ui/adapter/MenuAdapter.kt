package com.samsung.matter.chipstdeviceapp.feature.main.ui.adapter

import com.samsung.matter.chipstdeviceapp.feature.main.BR
import com.samsung.matter.chipstdeviceapp.feature.main.R
import com.samsung.matter.chipstdeviceapp.feature.main.model.Menu
import com.samsung.matter.chipstdeviceapp.feature.main.ui.recyclerview.DataBindingViewHolder
import com.samsung.matter.chipstdeviceapp.feature.main.ui.recyclerview.ItemDiffCallback
import com.samsung.matter.chipstdeviceapp.feature.main.ui.recyclerview.ListBindingAdapter


internal class MenuAdapter(
    menus: List<Menu>,
    private val itemHandler: ItemHandler
) : ListBindingAdapter<Menu>(ItemDiffCallback(
    onItemsTheSame = { old, new -> old.titleResId == new.titleResId },
    onContentsTheSame = { old, new -> old == new }
)) {

    init {
        submitList(menus)
    }

    override fun getItemViewType(position: Int): Int {
        return R.layout.item_menu
    }

    override fun viewBindViewHolder(holder: DataBindingViewHolder<Menu>, position: Int) {
        super.viewBindViewHolder(holder, position)
        holder.binding.setVariable(BR.itemHandler, itemHandler)
    }

    interface ItemHandler {
        fun onClick(item: Menu)
    }
}
