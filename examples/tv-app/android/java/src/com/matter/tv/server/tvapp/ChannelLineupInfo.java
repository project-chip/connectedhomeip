/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
