package com.matter.tv.server;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.PixelFormat;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import androidx.annotation.Nullable;

public class PromptDialogManager {
  private static final String TAG = "PromptDialogManager";
  /** normal type,like a standard AlertDialog */
  public static final int DIALOG_STYLE_1 = 1;
  /** input type,include an EditText */
  public static final int DIALOG_STYLE_2 = 2;
  /** normal type,no action area */
  public static final int DIALOG_STYLE_3 = 3;

  public static final int DIALOG_STYLE_4 = 4;
  private final int width;
  private final int height;
  public final float dimAmount;
  public final int style;
  private final boolean cancelable;

  private PromptDialogManager(Builder builder) {
    this.cancelable = builder.cancelable;
    this.height = builder.height;
    this.width = builder.width;
    this.dimAmount = builder.dimAmount;
    this.style = builder.style;
  }

  @SuppressLint("ClickableViewAccessibility")
  public void addView(View view, @Nullable onBackPressListener onBackPressListener) {
    WindowManager.LayoutParams params = new WindowManager.LayoutParams();
    params.type = WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY;
    params.format = PixelFormat.TRANSLUCENT;
    params.gravity = Gravity.CENTER;
    params.width = this.width;
    params.height = this.height;
    params.windowAnimations = android.R.style.Animation_Dialog;
    params.dimAmount = this.dimAmount;
    params.flags = convert2Flag(this.style);
    WindowManager windowManager =
        (WindowManager)
            MatterTvServerApplication.getApplication()
                .getApplicationContext()
                .getSystemService(Context.WINDOW_SERVICE);

    view.setFocusable(true);
    view.setFocusableInTouchMode(true);
    view.requestFocus();

    View.OnKeyListener onKeyListener =
        (view1, keyCode, keyEvent) -> {
          Log.d(TAG, "addView: setOnKeyListener keyCode = " + keyCode);
          if (keyCode == KeyEvent.KEYCODE_BACK && keyEvent.getAction() == KeyEvent.ACTION_UP) {
            windowManager.removeView(view);
            if (onBackPressListener != null) {
              onBackPressListener.onBackPress();
            }
            return true;
          }
          return false;
        };
    view.setOnKeyListener(onKeyListener);
    view.setOnTouchListener(
        (v, motionEvent) -> {
          if (motionEvent.getAction() == MotionEvent.ACTION_OUTSIDE) {
            Log.d(TAG, "handleCommissionOkPermission: setOnTouchListener ACTION_OUTSIDE");
            windowManager.removeView(view);
            return true;
          }
          return false;
        });

    windowManager.addView(view, params);
  }

  public void removeView(View view) {
    WindowManager windowManager =
        (WindowManager)
            MatterTvServerApplication.getApplication()
                .getApplicationContext()
                .getSystemService(Context.WINDOW_SERVICE);
    try {
      windowManager.removeView(view);
    } catch (Exception ex) {
      Log.w(TAG, "removeView: error = " + ex);
    }
  }

  public void removeViewImmediate(View view) {
    WindowManager windowManager =
        (WindowManager)
            MatterTvServerApplication.getApplication()
                .getApplicationContext()
                .getSystemService(Context.WINDOW_SERVICE);
    try {
      windowManager.removeViewImmediate(view);
    } catch (Exception ex) {
      Log.w(TAG, "removeViewImmediate: error = " + ex);
    }
  }

  private static int convert2Flag(int style) {
    int flags = WindowManager.LayoutParams.FLAG_DIM_BEHIND;
    if (style == DIALOG_STYLE_1) {
      return flags
          | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
          | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL;
    } else if (style == DIALOG_STYLE_2) {
      return flags;
    } else if (style == DIALOG_STYLE_3) {
      return flags
          | WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
          | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH;
    } else {
      return flags;
    }
  }

  public static class Builder {

    private int width = WindowManager.LayoutParams.WRAP_CONTENT;
    private int height = WindowManager.LayoutParams.WRAP_CONTENT;
    private float dimAmount = 0.6f;

    private int style = DIALOG_STYLE_1;
    private boolean cancelable = false;

    public Builder setWidth(int width) {
      this.width = width;
      return this;
    }

    public Builder setHeight(int height) {
      this.height = height;
      return this;
    }

    public Builder setCancelable(boolean mCancelable) {
      this.cancelable = mCancelable;
      return this;
    }

    public Builder setDimAmount(float dimAmount) {
      this.dimAmount = dimAmount;
      return this;
    }

    public Builder setStyle(int style) {
      this.style = style;
      return this;
    }

    public PromptDialogManager build() {
      return new PromptDialogManager(this);
    }
  }

  public interface onBackPressListener {
    void onBackPress();
  }
}
