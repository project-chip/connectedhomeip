/*
 *   Copyright (c) 2022 Project CHIP Authors
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
package com.chip.casting;

import android.util.Log;

public class TvCastingApp {
  private TvCastingAppCallback mCallback;
  private static final String TAG = TvCastingApp.class.getSimpleName();

  public TvCastingApp(TvCastingAppCallback callback) {
    mCallback = callback;
    nativeInit();
  }

  private void postClusterInit(int clusterId, int endpoint) {
    Log.d(TAG, "postClusterInit for " + clusterId + " at " + endpoint);
    if (mCallback != null) {
      mCallback.onClusterInit(this, clusterId, endpoint);
    }
  }

  public native void nativeInit();

  /** TBD: Temp dummy function for testing */
  public native void doSomethingInCpp(int endpoint);

  static {
    System.loadLibrary("TvCastingApp");
  }
}
