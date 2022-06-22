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

public class ChannelManagerStub implements ChannelManager {
  private static final String TAG = ChannelManagerStub.class.getSimpleName();

  private int endpoint;
  private int mCurrentChannel = 0;

  public ChannelManagerStub(int endpoint) {
    this.endpoint = endpoint;
  }

  @Override
  public ChannelInfo[] getChannelList() {
    ChannelInfo ChannelInfo1 = new ChannelInfo(1, 11, "HDMI1", "callSign1", "affiliateCallSign1");
    ChannelInfo ChannelInfo2 = new ChannelInfo(2, 22, "HDMI2", null, "");
    Log.d(TAG, "getChannelList at " + endpoint);
    return new ChannelInfo[] {ChannelInfo1, ChannelInfo2};
  }

  @Override
  public ChannelLineupInfo getLineup() {
    // for null lineup test
    if (mCurrentChannel == 100) {
      Log.d(TAG, "getChannelLineup: null at " + endpoint);
      return null;
    }

    ChannelLineupInfo lineupInfo = new ChannelLineupInfo("operator", "lineup", "postalCode");
    Log.d(TAG, "getChannelLineup: " + lineupInfo + " at " + endpoint);
    return lineupInfo;
  }

  @Override
  public ChannelInfo getCurrentChannel() {
    Log.d(TAG, "getCurrentChannel: at " + endpoint);
    // for null channel test
    if (mCurrentChannel == 100) {
      return null;
    }

    return new ChannelInfo(1, 1, "HDMI", "callSign", "affiliateCallSign");
  }

  @Override
  public ChannelInfo changeChannel(String match) {
    Log.d(TAG, "changeChannel: " + match + " at " + endpoint);
    if ("no".equals(match)) {
      return new ChannelInfo(ChannelInfo.kNoMatches);
    } else if ("multiple".equals(match)) {
      return new ChannelInfo(ChannelInfo.kMultipleMatches);
    } else {
      return new ChannelInfo(1, 1, "HDMI", "callSign", "affiliateCallSign");
    }
  }

  @Override
  public boolean changeChannelByNumber(int majorNumber, int minorNumber) {
    Log.d(
        TAG,
        "changeChannelByNumber: majorNumber = "
            + majorNumber
            + " minorNumber = "
            + minorNumber
            + " at "
            + endpoint);

    mCurrentChannel = majorNumber;

    // for failed test
    if (majorNumber == 1 && minorNumber == 1) {
      return false;
    }
    return true;
  }

  @Override
  public boolean skipChannel(int count) {
    Log.d(TAG, "skipChannel: count = " + count + " at " + endpoint);

    // for failed test
    if (count == 100) {
      return false;
    }
    return true;
  }
}
