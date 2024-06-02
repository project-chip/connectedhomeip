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

/**
 * Represents the Commissioner Declaration message sent by a User Directed Commissioning server
 * (CastingPlayer/Commissioner) to a UDC client (Casting Client/Commissionee).
 */
public class CommissionerDeclaration {
  static final String TAG = CommissionerDeclaration.class.getSimpleName();

  /** The allowed values for the ErrorCode field are the following */
  public enum CdError {
    kNoError(0),
    kCommissionableDiscoveryFailed(1),
    kPaseConnectionFailed(2),
    kPaseAuthFailed(3),
    kDacValidationFailed(4),
    kAlreadyOnFabric(5),
    kOperationalDiscoveryFailed(6),
    kCaseConnectionFailed(7),
    kCaseAuthFailed(8),
    kConfigurationFailed(9),
    kBindingConfigurationFailed(10),
    kCommissionerPasscodeNotSupported(11),
    kInvalidIdentificationDeclarationParams(12),
    kAppInstallConsentPending(13),
    kAppInstalling(14),
    kAppInstallFailed(15),
    kAppInstalledRetryNeeded(16),
    kCommissionerPasscodeDisabled(17),
    kUnexpectedCommissionerPasscodeReady(18);
    private final int value;

    CdError(int value) {
      this.value = value;
    }

    public int getValue() {
      return value;
    }
  }
  /** Feature: All - Indicates errors incurred during commissioning. */
  private CdError errorCode = CdError.kNoError;
  /**
   * Feature: Coordinate PIN Dialogs - When NoPasscode field set to true, and the Commissioner
   * determines that a Passcode code will be needed for commissioning.
   */
  private boolean needsPasscode = false;
  /**
   * Feature: Target Content Application - No apps with AccountLogin cluster implementation were
   * found for the last IdentificationDeclaration request. Only apps which provide access to the
   * vendor id of the Commissionee will be considered.
   */
  private boolean noAppsFound = false;
  /**
   * Feature: Coordinate PIN Dialogs - A Passcode input dialog is now displayed for the user on the
   * Commissioner.
   */
  private boolean passcodeDialogDisplayed = false;
  /**
   * Feature: Commissioner-Generated Passcode - A Passcode is now displayed for the user by the
   * CastingPlayer/Commissioner.
   */
  private boolean commissionerPasscode = false;
  /**
   * Feature: Commissioner-Generated Passcode - The user experience conveying a Passcode to the user
   * also displays a QR code.
   */
  private boolean qRCodeDisplayed = false;

  public CommissionerDeclaration(
      int errorCode,
      boolean needsPasscode,
      boolean noAppsFound,
      boolean passcodeDialogDisplayed,
      boolean commissionerPasscode,
      boolean qRCodeDisplayed) {
    this.errorCode = CdError.values()[errorCode];
    this.needsPasscode = needsPasscode;
    this.noAppsFound = noAppsFound;
    this.passcodeDialogDisplayed = passcodeDialogDisplayed;
    this.commissionerPasscode = commissionerPasscode;
    this.qRCodeDisplayed = qRCodeDisplayed;
  }

  public void setErrorCode(CdError errorCode) {
    this.errorCode = errorCode;
  }

  public CdError getErrorCode() {
    return this.errorCode;
  }

  public void setNeedsPasscode(boolean needsPasscode) {
    this.needsPasscode = needsPasscode;
  }

  public boolean getNeedsPasscode() {
    return this.needsPasscode;
  }

  public void setNoAppsFound(boolean noAppsFound) {
    this.noAppsFound = noAppsFound;
  }

  public boolean getNoAppsFound() {
    return this.noAppsFound;
  }

  public void setPasscodeDialogDisplayed(boolean passcodeDialogDisplayed) {
    this.passcodeDialogDisplayed = passcodeDialogDisplayed;
  }

  public boolean getPasscodeDialogDisplayed() {
    return this.passcodeDialogDisplayed;
  }

  public void setCommissionerPasscode(boolean commissionerPasscode) {
    this.commissionerPasscode = commissionerPasscode;
  }

  public boolean getCommissionerPasscode() {
    return this.commissionerPasscode;
  }

  public void setQRCodeDisplayed(boolean qRCodeDisplayed) {
    this.qRCodeDisplayed = qRCodeDisplayed;
  }

  public boolean getQRCodeDisplayed() {
    return this.qRCodeDisplayed;
  }

  @Override
  public String toString() {
    return "CommissionerDeclaration::errorCode:               "
        + errorCode.name()
        + "\n"
        + "CommissionerDeclaration::needsPasscode:           "
        + needsPasscode
        + "\n"
        + "CommissionerDeclaration::noAppsFound:             "
        + noAppsFound
        + "\n"
        + "CommissionerDeclaration::passcodeDialogDisplayed: "
        + passcodeDialogDisplayed
        + "\n"
        + "CommissionerDeclaration:commissionerPasscode:     "
        + commissionerPasscode
        + "\n"
        + "CommissionerDeclaration::qRCodeDisplayed:         "
        + qRCodeDisplayed;
  }

  public void logDetail() {
    Log.d(TAG, "CommissionerDeclaration::logDetail()\n" + this.toString());
  }
}
