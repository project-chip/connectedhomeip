package com.matter.virtual.device.app.core.ui

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import androidx.coordinatorlayout.widget.CoordinatorLayout
import com.google.android.material.appbar.AppBarLayout

class DisableAppBarLayoutBehavior(context: Context, attributeSet: AttributeSet) :
  AppBarLayout.Behavior(context, attributeSet) {

  override fun onStartNestedScroll(
    parent: CoordinatorLayout,
    child: AppBarLayout,
    directTargetChild: View,
    target: View,
    nestedScrollAxes: Int,
    type: Int
  ): Boolean {
    return false
  }

  override fun onTouchEvent(
    parent: CoordinatorLayout,
    child: AppBarLayout,
    ev: MotionEvent
  ): Boolean {
    return false
  }
}
