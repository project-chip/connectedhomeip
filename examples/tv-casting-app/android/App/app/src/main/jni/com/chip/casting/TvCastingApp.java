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

public class TvCastingApp {
  private static final String TAG = TvCastingApp.class.getSimpleName();

  public native void setDACProvider(DACProvider provider);

  public native boolean openBasicCommissioningWindow(
      int duration, Object commissioningCompleteHandler);

  public native boolean sendUserDirectedCommissioningRequest(String address, int port);

  public native boolean discoverCommissioners();

  public native void init();

  public native boolean contentLauncherLaunchURL(
      String contentUrl, String contentDisplayStr, Object launchURLHandler);

  static {
    System.loadLibrary("TvCastingApp");
  }
}
