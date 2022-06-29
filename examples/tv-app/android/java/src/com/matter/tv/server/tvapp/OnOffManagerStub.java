package com.matter.tv.server.tvapp;

import android.util.Log;

public class OnOffManagerStub implements OnOffManager {
  private final String TAG = OnOffManagerStub.class.getSimpleName();
  private int mEndpoint;

  public OnOffManagerStub(int endpoint) {
    mEndpoint = endpoint;
  }

  @Override
  public void HandleOnOffChanged(boolean value) {
    Log.d(TAG, "HandleOnOffChanged for mEndpoint " + mEndpoint + " = " + value);
  }
}
