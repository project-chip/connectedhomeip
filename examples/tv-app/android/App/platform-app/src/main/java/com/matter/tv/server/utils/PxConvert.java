package com.matter.tv.server.utils;

import android.content.Context;
import android.util.TypedValue;

public class PxConvert {
  public static int dp2px(Context context, float value) {
    return (int)
        (TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP, value, context.getResources().getDisplayMetrics())
            + 0.5f);
  }
}
