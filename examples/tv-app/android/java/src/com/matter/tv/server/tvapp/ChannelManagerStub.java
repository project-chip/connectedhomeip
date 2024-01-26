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
import com.matter.tv.server.tvapp.ChannelInfo.ChannelType;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class ChannelManagerStub implements ChannelManager {
  private static final String TAG = ChannelManagerStub.class.getSimpleName();

  private int endpoint;
  private int currentChannelIndex = 0;
  private ChannelInfo currentChannel;

  ChannelInfo chanAbc = new ChannelInfo(6, 0, "ABC", "KAAL-TV", "KAAL", "id-1", ChannelType.Cable);
  ChannelInfo chanPbs =
      new ChannelInfo(9, 1, "PBS", "KCTS-TV", "KCTS", "id-2", ChannelType.Satellite);
  ChannelInfo chanWor =
      new ChannelInfo(9, 3, "World Channel", "KCTS-TV", "KCTS", "id-4", ChannelType.Terrestrial);

  private ChannelInfo[] channelList = {
    chanAbc,
    chanPbs,
    chanWor,
    new ChannelInfo(9, 2, "PBS Kids", "KCTS-TV", "KCTS", "id-3", ChannelType.OTT)
  };

  private ChannelProgramInfo[] programList = {
    new ChannelProgramInfo(
        "progid-abc1", chanAbc, 0, 30 * 60, "First Show", "First subtitle", "First Description"),
    new ChannelProgramInfo(
        "progid-pbs1", chanPbs, 0, 30 * 60, "Show 2", "subtitle 2", "Description 2"),
    new ChannelProgramInfo(
        "progid-abc2", chanAbc, 30 * 60, 60 * 60, "Show 3", "subtitle 3", "Description 3"),
    new ChannelProgramInfo(
        "progid-abc3", chanAbc, 30 * 60, 60 * 60, "Show 4", "subtitle 4", "Description 4"),
    new ChannelProgramInfo(
        "id-5", chanWor, 60 * 60, 90 * 60, "Show 5", "subtitle 5", "Description 5"),
  };

  public ChannelManagerStub(int endpoint) {
    this.endpoint = endpoint;
    this.currentChannel = channelList[currentChannelIndex];
  }

  @Override
  public ChannelInfo[] getChannelList() {
    return channelList;
  }

  @Override
  public ChannelLineupInfo getLineup() {
    ChannelLineupInfo lineupInfo = new ChannelLineupInfo("Comcast", "Comcast King County", "98052");
    Log.d(TAG, "getChannelLineup: " + lineupInfo + " at " + endpoint);
    return lineupInfo;
  }

  @Override
  public ChannelInfo getCurrentChannel() {
    Log.d(TAG, "getCurrentChannel: at " + endpoint);
    return channelList[currentChannelIndex];
  }

  boolean isChannelMatched(ChannelInfo channel, String match) {
    String number = channel.majorNumber + "." + channel.minorNumber;
    boolean nameMatch = channel.name.equals(match);
    boolean affiliateCallSignMatch = channel.affiliateCallSign.equals(match);
    boolean callSignMatch = channel.callSign.equals(match);
    boolean numberMatch = number.equals(match);

    return affiliateCallSignMatch || callSignMatch || nameMatch || numberMatch;
  }

  @Override
  public ChannelInfo changeChannel(String match) {
    Log.d(TAG, "changeChannel: " + match + " at " + endpoint);
    List<ChannelInfo> matchedList = new ArrayList<>();
    int index = 0;

    for (ChannelInfo channel : channelList) {
      // verify if CharSpan matches channel name
      // or callSign or affiliateCallSign or majorNumber.minorNumber
      if (isChannelMatched(channel, match)) {
        matchedList.add(channel);
        break;
      }
      ;
      // use index to set current channel at the end
      ++index;
    }

    if (matchedList.size() > 1) {
      // Error: Multiple matches
      Log.d(TAG, "multiple matches");
      return new ChannelInfo(ChannelInfo.kMultipleMatches);
    } else if (matchedList.size() == 0) {
      // Error: No match
      Log.d(TAG, "no matches");
      return new ChannelInfo(ChannelInfo.kNoMatches);
    } else {
      Log.d(TAG, "success 1 match");
      // Success: 1 match
      currentChannel = channelList[index];
      currentChannelIndex = index;
      return channelList[index];
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

    boolean channelChanged = false;
    int index = 0;

    for (ChannelInfo channel : channelList) {
      // verify if major & minor matches one of the channel from the list
      if (channel.majorNumber == majorNumber && channel.minorNumber == minorNumber) {
        // verify if channel changed by comparing values of current channel with the requested
        // channel
        if (channel.majorNumber != currentChannel.majorNumber
            || channel.minorNumber != currentChannel.minorNumber) {
          channelChanged = true;
          currentChannelIndex = index;
          currentChannel = channelList[index];
        }
      }
      ++index;
    }
    return channelChanged;
  }

  @Override
  public boolean skipChannel(int count) {
    Log.d(TAG, "skipChannel: count = " + count + " at " + endpoint);

    int newChannelIndex = (count + currentChannelIndex) % channelList.length;
    currentChannelIndex = newChannelIndex;
    currentChannel = channelList[newChannelIndex];
    return true;
  }

  @Override
  public ChannelProgramResponse getProgramGuide(
      long startTime,
      long endTime,
      ChannelInfo[] channels,
      String pageToken,
      boolean series,
      Map.Entry<String, String>[] externalIDList,
      String data) {
    ChannelProgramResponse resp = new ChannelProgramResponse();
    resp.programs = programList;
    return resp;
  }

  @Override
  public boolean recordProgram(
      String identifier, boolean series, Map.Entry<String, String>[] externalIDList, String data) {
    for (ChannelProgramInfo program : programList) {
      if (program.identifier.equals(identifier)) {
        program.recordFlagSeries = series;
        program.recordFlagScheduled = true;
        return true;
      }
    }
    return false;
  }

  @Override
  public boolean cancelRecordProgram(
      String identifier, boolean series, Map.Entry<String, String>[] externalIDList, String data) {
    for (ChannelProgramInfo program : programList) {
      if (program.identifier.equals(identifier)) {
        if (series) {
          program.recordFlagSeries = false;
        }
        program.recordFlagScheduled = false;
        return true;
      }
    }
    return false;
  }
}
