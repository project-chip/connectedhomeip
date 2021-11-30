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

public class TvChannelInfo {
  public int majorNumber;
  public int minorNumber;
  public String name;
  public String callSign;
  public String affiliateCallSign;

  public TvChannelInfo() {}

  public TvChannelInfo(
      int majorNumber, int minorNumber, String name, String callSign, String affiliateCallSign) {
    this.majorNumber = majorNumber;
    this.minorNumber = minorNumber;
    this.name = name;
    this.callSign = callSign;
    this.affiliateCallSign = affiliateCallSign;
  }

  @Override
  public String toString() {
    return "TvChannelInfo{"
        + "majorNumber="
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
