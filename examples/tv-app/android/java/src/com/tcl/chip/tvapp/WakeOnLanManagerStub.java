package com.tcl.chip.tvapp;

import android.util.Log;

public class WakeOnLanManagerStub implements WakeOnLanManager {

  private final String TAG = WakeOnLanManagerStub.class.getSimpleName();

  @Override
  public String getMac(int endpoint) {
    Log.d(TAG, "getMac for endpoint=" + endpoint);
    return "AA:BB:CC:DD:EE";
  }
}
