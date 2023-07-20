package com.matter.virtual.device.app.feature.main

import android.graphics.Rect
import android.view.View
import androidx.recyclerview.widget.RecyclerView

class VerticalSpaceItemDecoration(private val sideSpace: Int, private val bottomSpace: Int) :
  RecyclerView.ItemDecoration() {

  override fun getItemOffsets(
    outRect: Rect,
    view: View,
    parent: RecyclerView,
    state: RecyclerView.State
  ) {
    parent.adapter?.let { adapter ->
      if (parent.getChildAdapterPosition(view) != adapter.itemCount - 1) {
        outRect.bottom = bottomSpace
      }

      outRect.left = sideSpace
      outRect.right = sideSpace
    }
  }
}
