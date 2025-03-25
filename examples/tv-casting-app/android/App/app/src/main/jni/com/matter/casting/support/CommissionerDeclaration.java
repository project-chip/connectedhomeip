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

  /**
   * The allowed values for the ErrorCode field are the following. Indicates errors incurred during
   * commissioning.
   */
  public enum CdError {
    noError(0, "No error"),
    commissionableDiscoveryFailed(1, "Commissionable Node discovery failed"),
    paseConnectionFailed(2, "PASE connection failed"),
    paseAuthFailed(3, "PASE authentication failed (bad Passcode)"),
    dacValidationFailed(4, "DAC validation failed"),
    alreadyOnFabric(5, "Already on fabric"),
    operationalDiscoveryFailed(6, "Operational Node discovery failed"),
    caseConnectionFailed(7, "CASE connection failed"),
    caseAuthFailed(8, "CASE authentication failed"),
    configurationFailed(9, "Configuration failed"),
    bindingConfigurationFailed(10, "Binding Configuration failed"),
    commissionerPasscodeNotSupported(11, "Commissioner Passcode not supported"),
    invalidIdentificationDeclarationParams(12, "Invalid UDC Identification Declaration parameters"),
    appInstallConsentPending(13, "App Install Consent Pending"),
    appInstalling(14, "App Installing"),
    appInstallFailed(15, "App Install Failed"),
    appInstalledRetryNeeded(16, "App Installed, Retry Needed"),
    commissionerPasscodeDisabled(17, "Commissioner Passcode disabled"),
    unexpectedCommissionerPasscodeReady(18, "Unexpected Commissioner Passcode ready");

    private final int value;
    private final String description;

    private CdError(int value, String description) {
      this.value = value;
      this.description = description;
    }

    private static String getDefaultDescription(int value) {
      for (CdError error : CdError.values()) {
        if (error.value == value) {
          return error.name();
        }
      }
      return "Unknown Error";
    }

    CdError(int value) {
      this(value, getDefaultDescription(value));
    }

    public int getValue() {
      return value;
    }

    public String getDescription() {
      return description;
    }
  }
  /** Feature: All - Indicates errors incurred during commissioning. */
  private CdError errorCode = CdError.noError;
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
  /**
   * Feature: Commissioner-Generated Passcode - Flag to indicate when the CastingplAYER/Commissioner
   * user has decided to exit the commissioning process.
   */
  private boolean cancelPasscode = false;

  public CommissionerDeclaration(
      int errorCode,
      boolean needsPasscode,
      boolean noAppsFound,
      boolean passcodeDialogDisplayed,
      boolean commissionerPasscode,
      boolean qRCodeDisplayed,
      boolean cancelPasscode) {
    this.errorCode = CdError.values()[errorCode];
    this.needsPasscode = needsPasscode;
    this.noAppsFound = noAppsFound;
    this.passcodeDialogDisplayed = passcodeDialogDisplayed;
    this.commissionerPasscode = commissionerPasscode;
    this.qRCodeDisplayed = qRCodeDisplayed;
    this.cancelPasscode = cancelPasscode;
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

  public boolean getCancelPasscode() {
    return this.cancelPasscode;
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
        + qRCodeDisplayed
        + "\n"
        + "CommissionerDeclaration::cancelPasscode:          "
        + cancelPasscode;
  }

  public void logDetail() {
    Log.d(TAG, "CommissionerDeclaration::logDetail()\n" + this.toString());
  }
}
