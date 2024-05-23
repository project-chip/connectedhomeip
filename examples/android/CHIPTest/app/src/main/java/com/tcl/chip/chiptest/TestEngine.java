package com.tcl.chip.chiptest;

import android.util.Log;

public class TestEngine {
  private static final String TAG = TestEngine.class.getSimpleName();
  private static TestListener mListener;

  public interface TestListener {
    void onTestLog(String log);
  }

  public static void setListener(TestListener listener) {
    mListener = listener;
  }

  public static void onTestLog(String log) {
    Log.i(TAG, log);
    if (mListener != null) {
      mListener.onTestLog(log);
    }
  }

  public static native int runTest();

  static {
    System.loadLibrary("CHIPTest");
  }
}
