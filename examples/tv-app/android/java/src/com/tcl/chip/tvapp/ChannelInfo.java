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

public class ChannelInfo {

  public static final int kNoError = -1; // todo: what will be the value of no error?
  public static final int kMultipleMatches = 0;
  public static final int kNoMatches = 1;

  private int errorType;
  private int majorNumber;
  private int minorNumber;
  private String name;
  private String callSign;
  private String affiliateCallSign;

  public ChannelInfo(
      int majorNumber, int minorNumber, String name, String callSign, String affiliateCallSign) {
    this.errorType = kNoError;
    this.majorNumber = majorNumber;
    this.minorNumber = minorNumber;
    this.name = name;
    this.callSign = callSign;
    this.affiliateCallSign = affiliateCallSign;
  }

  public ChannelInfo(int errorType) {
    this.errorType = errorType;
  }

  @Override
  public String toString() {
    return "ChannelInfo{"
        + "errorType="
        + errorType
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
