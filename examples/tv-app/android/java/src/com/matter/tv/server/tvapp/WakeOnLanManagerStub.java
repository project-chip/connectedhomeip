package com.matter.tv.server.tvapp;

import android.util.Log;

public class WakeOnLanManagerStub implements WakeOnLanManager {

  private final String TAG = WakeOnLanManagerStub.class.getSimpleName();

  private int endpoint;

  public WakeOnLanManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public String getMac() {
    Log.d(TAG, "getMac for endpoint=" + endpoint);
    return "AA:BB:CC:DD:EE";
  }
}
