/*
 *   Copyright (c) 2021 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

  public native void setMessagesManager(int endpoint, MessagesManager manager);

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
