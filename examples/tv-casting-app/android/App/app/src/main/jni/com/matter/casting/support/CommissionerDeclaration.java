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
 * (Casting Player) to a UDC client (tv-casting-app).
 */
public class CommissionerDeclaration {
  static final String TAG = CommissionerDeclaration.class.getSimpleName();

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

  private CdError mErrorCode = CdError.kNoError;
  private boolean mNeedsPasscode = false;
  private boolean mNoAppsFound = false;
  private boolean mPasscodeDialogDisplayed = false;
  private boolean mCommissionerPasscode = false;
  private boolean mQRCodeDisplayed = false;

  public CommissionerDeclaration(
      int errorCode,
      boolean needsPasscode,
      boolean noAppsFound,
      boolean passcodeDialogDisplayed,
      boolean commissionerPasscode,
      boolean qrCodeDisplayed) {
    mErrorCode = CdError.values()[errorCode];
    mNeedsPasscode = needsPasscode;
    mNoAppsFound = noAppsFound;
    mPasscodeDialogDisplayed = passcodeDialogDisplayed;
    mCommissionerPasscode = commissionerPasscode;
    mQRCodeDisplayed = qrCodeDisplayed;
  }

  public void setErrorCode(CdError newValue) {
    mErrorCode = newValue;
  }

  public CdError getErrorCode() {
    return mErrorCode;
  }

  public void setNeedsPasscode(boolean newValue) {
    mNeedsPasscode = newValue;
  }

  public boolean getNeedsPasscode() {
    return mNeedsPasscode;
  }

  public void setNoAppsFound(boolean newValue) {
    mNoAppsFound = newValue;
  }

  public boolean getNoAppsFound() {
    return mNoAppsFound;
  }

  public void setPasscodeDialogDisplayed(boolean newValue) {
    mPasscodeDialogDisplayed = newValue;
  }

  public boolean getPasscodeDialogDisplayed() {
    return mPasscodeDialogDisplayed;
  }

  public void setCommissionerPasscode(boolean newValue) {
    mCommissionerPasscode = newValue;
  }

  public boolean getCommissionerPasscode() {
    return mCommissionerPasscode;
  }

  public void setQRCodeDisplayed(boolean newValue) {
    mQRCodeDisplayed = newValue;
  }

  public boolean getQRCodeDisplayed() {
    return mQRCodeDisplayed;
  }

  public void logDetail() {
    Log.d(TAG, "CommissionerDeclaration::logDetail() - java");
    Log.d(TAG, "CommissionerDeclaration::mErrorCode:               " + mErrorCode.name());
    Log.d(TAG, "CommissionerDeclaration::mNeedsPasscode:           " + mNeedsPasscode);
    Log.d(TAG, "CommissionerDeclaration::mNoAppsFound:             " + mNoAppsFound);
    Log.d(TAG, "CommissionerDeclaration::mPasscodeDialogDisplayed: " + mPasscodeDialogDisplayed);
    Log.d(TAG, "CommissionerDeclaration::mCommissionerPasscode:    " + mCommissionerPasscode);
    Log.d(TAG, "CommissionerDeclaration::mQRCodeDisplayed:         " + mQRCodeDisplayed);
  }
}
