/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

import android.util.Log;

public class TvApp {
  private TvAppCallback mCallback;
  private static final String TAG = "TvApp";

  public TvApp(TvAppCallback callback) {
    mCallback = callback;
    nativeInit();
  }

  private void postClusterInit(long clusterId, int endpoint) {
    Log.d(TAG, "postClusterInit for " + clusterId + " at " + endpoint);
    if (mCallback != null) {
      mCallback.onClusterInit(this, clusterId, endpoint);
    }
  }

  public native void nativeInit();

  public native void initializeCommissioner(UserPrompter userPrompter);

  // called before Matter server is inited
  public native void preServerInit();

  // called after Matter server is inited
  public native void postServerInit(ContentAppEndpointManager manager);

  public native void setKeypadInputManager(int endpoint, KeypadInputManager manager);

  public native void setWakeOnLanManager(int endpoint, WakeOnLanManager manager);

  public native void setMediaInputManager(int endpoint, MediaInputManager manager);

  public native void setContentLaunchManager(int endpoint, ContentLaunchManager manager);

  public native void setLowPowerManager(int endpoint, LowPowerManager manager);

  public native void setMediaPlaybackManager(int endpoint, MediaPlaybackManager manager);

  public native void setChannelManager(int endpoint, ChannelManager manager);

  public native void setOnOffManager(int endpoint, OnOffManager manager);

  public native boolean setOnOff(int endpoint, boolean value);

  public native void setLevelManager(int endpoint, LevelManager manager);

  public native boolean setCurrentLevel(int endpoint, int value);

  public native void setDACProvider(DACProvider provider);

  public native void setChipDeviceEventProvider(DeviceEventProvider provider);

  static {
    System.loadLibrary("TvApp");
  }
}
