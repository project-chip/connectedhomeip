/**
 * Copyright (c) 2024 Project CHIP Authors All rights reserved.
 *
 * <p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 * <p>http://www.apache.org/licenses/LICENSE-2.0
 *
 * <p>Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.matter.casting.support;

import android.util.Log;
import java.util.ArrayList;
import java.util.List;

/**
 * This class contains the optional parameters used in the IdentificationDeclaration Message, sent
 * by the Commissionee to the Commissioner. The options specify information relating to the
 * requested UDC commissioning session.
 */
public class IdentificationDeclarationOptions {
  static final String TAG = IdentificationDeclarationOptions.class.getSimpleName();
  private static final int CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS = 10;

  public IdentificationDeclarationOptions() {}

  public IdentificationDeclarationOptions(
      boolean noPasscode,
      boolean cdUponPasscodeDialog,
      boolean commissionerPasscode,
      boolean commissionerPasscodeReady,
      boolean cancelPasscode,
      List<TargetAppInfo> targetAppInfos) {
    this.noPasscode = noPasscode;
    this.cdUponPasscodeDialog = cdUponPasscodeDialog;
    this.commissionerPasscode = commissionerPasscode;
    this.commissionerPasscodeReady = commissionerPasscodeReady;
    this.cancelPasscode = cancelPasscode;
    this.targetAppInfos = targetAppInfos != null ? targetAppInfos : new ArrayList<>();
  }

  /**
   * Feature: Target Content Application - Flag to instruct the Commissioner not to display a
   * Passcode input dialog, and instead send a CommissionerDeclaration message if a commissioning
   * Passcode is needed.
   */
  public boolean noPasscode = false;
  /**
   * Feature: Coordinate Passcode Dialogs - Flag to instruct the Commissioner to send a
   * CommissionerDeclaration message when the Passcode input dialog on the Commissioner has been
   * shown to the user.
   */
  public boolean cdUponPasscodeDialog = false;
  /**
   * Feature: Commissioner-Generated Passcode - Flag to instruct the Commissioner to use the
   * Commissioner-generated Passcode for commissioning.
   */
  public boolean commissionerPasscode = false;
  /**
   * Feature: Commissioner-Generated Passcode - Flag to indicate whether or not the Commissionee has
   * obtained the Commissioner Passcode from the user and is therefore ready for commissioning.
   */
  public boolean commissionerPasscodeReady = false;
  /**
   * Feature: Coordinate Passcode Dialogs Flag - to indicate when the Commissionee user has decided
   * to exit the commissioning process.
   */
  public boolean cancelPasscode = false;
  /**
   * Feature: Target Content Application - The set of content app Vendor IDs (and optionally,
   * Product IDs) that can be used for authentication. Also, if TargetAppInfo is passed in,
   * VerifyOrEstablishConnection() will force User Directed Commissioning, in case the desired
   * TargetApp is not found in the on-device CastingStore.
   */
  private List<TargetAppInfo> targetAppInfos = new ArrayList<>();

  public boolean addTargetAppInfo(TargetAppInfo targetAppInfo) {
    Log.d(TAG, "addTargetAppInfo()");
    if (targetAppInfos.size() >= CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS) {
      Log.e(
          TAG,
          "addTargetAppInfo() failed to add TargetAppInfo, max list size is {0}"
              + CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS);
      return false;
    }
    targetAppInfos.add(targetAppInfo);
    return true;
  }

  public List<TargetAppInfo> getTargetAppInfoList() {
    return targetAppInfos;
  }

  @Override
  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append("IdentificationDeclarationOptions::noPasscode:                ")
        .append(noPasscode)
        .append("\n");
    sb.append("IdentificationDeclarationOptions::cdUponPasscodeDialog:      ")
        .append(cdUponPasscodeDialog)
        .append("\n");
    sb.append("IdentificationDeclarationOptions::commissionerPasscode:      ")
        .append(commissionerPasscode)
        .append("\n");
    sb.append("IdentificationDeclarationOptions::commissionerPasscodeReady: ")
        .append(commissionerPasscodeReady)
        .append("\n");
    sb.append("IdentificationDeclarationOptions::cancelPasscode:            ")
        .append(cancelPasscode)
        .append("\n");
    sb.append("IdentificationDeclarationOptions::targetAppInfos list: \n");

    for (TargetAppInfo targetAppInfo : targetAppInfos) {
      sb.append("\t\tTargetAppInfo - Vendor ID: ")
          .append(targetAppInfo.vendorId)
          .append(", Product ID: ")
          .append(targetAppInfo.productId)
          .append("\n");
    }

    return sb.toString();
  }

  public void logDetail() {
    Log.d(TAG, "IdentificationDeclarationOptions::logDetail()\n" + this.toString());
  }
}
