/*
 *   Copyright (c) 2023 Project CHIP Authors
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
package com.matter.virtual.device.app;

import android.util.Log;

public class DeviceApp {
  private DeviceAppCallback mCallback = null;
  private static final String TAG = "DeviceApp";

  public DeviceApp() {
    nativeInit();
  }

  public void setCallback(DeviceAppCallback callback) {
    mCallback = callback;
  }

  private void postClusterInit(long clusterId, int endpoint) {
    Log.d(TAG, "postClusterInit for " + clusterId + " at " + endpoint);
    if (mCallback != null) {
      mCallback.onClusterInit(this, clusterId, endpoint);
    }
  }

  private void postEvent(long event) {
    Log.d(TAG, "postEvent : " + event);
    if (mCallback != null) {
      mCallback.onEvent(event);
    }
  }

  public native void nativeInit();

  // called before Matter server is initiated
  public native void preServerInit();

  // called after Matter server is initiated
  public native void postServerInit(int deviceTypeId);

  public native void setDACProvider(DACProvider provider);

  public native void setOnOffManager(int endpoint, OnOffManager manager);

  public native boolean setOnOff(int endpoint, boolean value);

  public native void setDoorLockManager(int endpoint, DoorLockManager manager);

  public native boolean setLockType(int endpoint, int value);

  public native boolean setLockState(int endpoint, int value);

  public native boolean setActuatorEnabled(int endpoint, boolean value);

  public native boolean setAutoRelockTime(int endpoint, int value);

  public native boolean setOperatingMode(int endpoint, int value);

  public native boolean setSupportedOperatingModes(int endpoint, int value);

  public native boolean sendLockAlarmEvent(int endpoint);

  public native void setPowerSourceManager(int endpoint, PowerSourceManager manager);

  public native boolean setBatPercentRemaining(int endpoint, int value);

  static {
    System.loadLibrary("DeviceApp");
  }
}
