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

public class TvChannelManagerStub implements TvChannelManager {
  private static final String TAG = "ChannelManagerStub";

  @Override
  public TvChannelInfo[] getChannelList() {
    TvChannelInfo tvChannelInfo1 =
        new TvChannelInfo(1, 1, "HDMI1", "callSign1", "affiliateCallSign1");
    TvChannelInfo tvChannelInfo2 =
        new TvChannelInfo(2, 2, "HDMI2", "callSign2", "affiliateCallSign2");
    Log.d(TAG, "getTVChannelList");
    return new TvChannelInfo[] {tvChannelInfo1, tvChannelInfo2};
  }

  @Override
  public TvChannelLineupInfo getLineup() {
    TvChannelLineupInfo lineupInfo = new TvChannelLineupInfo("operator", "lineup", "postalCode");
    Log.d(TAG, "getTVChannelLineup: " + lineupInfo);
    return lineupInfo;
  }

  @Override
  public TvChannelInfo getCurrentChannel() {
    Log.d(TAG, "getCurrentTVChannel: ");
    return new TvChannelInfo(1, 1, "HDMI", "callSign", "affiliateCallSign");
  }

  @Override
  public TvChannelInfo changeChannel(String match) {
    Log.d(TAG, "changeChannel: " + match);
    return new TvChannelInfo(1, 1, "HDMI", "callSign", "affiliateCallSign");
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
