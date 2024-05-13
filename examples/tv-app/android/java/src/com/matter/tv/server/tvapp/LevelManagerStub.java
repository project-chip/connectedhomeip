package com.matter.tv.server.tvapp;

import android.util.Log;

public class LevelManagerStub implements LevelManager {
  private final String TAG = LevelManagerStub.class.getSimpleName();
  private int mEndpoint;

  public LevelManagerStub(int endpoint) {
    mEndpoint = endpoint;
  }

  @Override
  public void HandleLevelChanged(int value) {
    Log.d(TAG, "HandleOnOffChanged for mEndpoint " + mEndpoint + " = " + value);
  }
}
