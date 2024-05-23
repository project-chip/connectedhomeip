package com.matter.tv.server.tvapp;

import android.util.Log;

public class LowPowerManagerStub implements LowPowerManager {

  private final String TAG = LowPowerManagerStub.class.getSimpleName();

  private int endpoint;

  public LowPowerManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public boolean sleep() {
    Log.d(TAG, "sleep at " + endpoint);
    return true;
  }
}
