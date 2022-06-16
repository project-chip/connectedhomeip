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

public class ChannelLineupInfo {
  public static final int LINEUP_INFO_TYPE_MSO = 0x00;

  public String operatorName;
  public String lineupName;
  public String postalCode;
  public int lineupInfoType;

  public ChannelLineupInfo(String operatorName, String lineupName, String postalCode) {
    this(operatorName, lineupName, postalCode, LINEUP_INFO_TYPE_MSO);
  }

  public ChannelLineupInfo(
      String operatorName, String lineupName, String postalCode, int lineupInfoType) {
    this.operatorName = operatorName;
    this.lineupName = lineupName;
    this.postalCode = postalCode;
    this.lineupInfoType = lineupInfoType;
  }

  @Override
  public String toString() {
    return "ChannelLineupInfo{"
        + "operatorName='"
        + operatorName
        + '\''
        + ", lineupName='"
        + lineupName
        + '\''
        + ", postalCode='"
        + postalCode
        + '\''
        + ", lineupInfoType="
        + lineupInfoType
        + '}';
  }
}
