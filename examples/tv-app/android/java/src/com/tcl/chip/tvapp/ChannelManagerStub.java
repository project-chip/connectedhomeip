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
package com.tcl.chip.tvapp;

import android.util.Log;

public class ChannelManagerStub implements ChannelManager {
  private static final String TAG = "ChannelManagerStub";

  @Override
  public ChannelInfo[] getChannelList() {
    ChannelInfo ChannelInfo1 =
        new ChannelInfo(1, 1, "HDMI1", "callSign1", "affiliateCallSign1");
    ChannelInfo ChannelInfo2 =
        new ChannelInfo(2, 2, "HDMI2", "callSign2", "affiliateCallSign2");
    Log.d(TAG, "getChannelList");
    return new ChannelInfo[] {ChannelInfo1, ChannelInfo2};
  }

  @Override
  public ChannelLineupInfo getLineup() {
    ChannelLineupInfo lineupInfo = new ChannelLineupInfo("operator", "lineup", "postalCode");
    Log.d(TAG, "getChannelLineup: " + lineupInfo);
    return lineupInfo;
  }

  @Override
  public ChannelInfo getCurrentChannel() {
    Log.d(TAG, "getCurrentChannel: ");
    return new ChannelInfo(1, 1, "HDMI", "callSign", "affiliateCallSign");
  }

  @Override
  public ChannelInfo changeChannel(String match) {
    Log.d(TAG, "changeChannel: " + match);
    return new ChannelInfo(1, 1, "HDMI", "callSign", "affiliateCallSign");
  }

  @Override
  public boolean changeChannelByNumber(int majorNumber, int minorNumber) {
    Log.d(
        TAG,
        "changeChannelByNumber: majorNumber = " + majorNumber + " minorNumber = " + minorNumber);
    return true;
  }

  @Override
  public boolean skipChannel(int count) {
    Log.d(TAG, "skipChannel: count = " + count);
    return true;
  }
}
