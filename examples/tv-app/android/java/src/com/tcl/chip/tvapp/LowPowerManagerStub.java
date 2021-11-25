package com.tcl.chip.tvapp;

import android.util.Log;

public class LowPowerManagerStub implements LowPowerManager {

  private final String TAG = LowPowerManagerStub.class.getSimpleName();

  @Override
  public boolean sleep() {
    Log.d(TAG, "sleep");
    return true;
  }
}
