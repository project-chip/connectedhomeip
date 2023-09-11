/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.tv.server.tvapp;

public class ChannelInfo {

  public static final int kSuccess = 0;
  public static final int kMultipleMatches = 1;
  public static final int kNoMatches = 2;

  public int status;
  public int majorNumber;
  public int minorNumber;
  public String name;
  public String callSign;
  public String affiliateCallSign;

  public ChannelInfo(
      int majorNumber, int minorNumber, String name, String callSign, String affiliateCallSign) {
    this.status = kSuccess;
    this.majorNumber = majorNumber;
    this.minorNumber = minorNumber;
    this.name = name;
    this.callSign = callSign;
    this.affiliateCallSign = affiliateCallSign;
  }

  public ChannelInfo(int status) {
    this.status = status;
  }

  @Override
  public String toString() {
    return "ChannelInfo{"
        + "status="
        + status
        + ", majorNumber="
        + majorNumber
        + ", minorNumber="
        + minorNumber
        + ", name='"
        + name
        + '\''
        + ", callSign='"
        + callSign
        + '\''
        + ", affiliateCallSign='"
        + affiliateCallSign
        + '\''
        + '}';
  }
}
