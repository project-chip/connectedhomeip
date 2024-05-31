/**
 *   Copyright (c) 2024 Project CHIP Authors
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
 */

package com.matter.casting.support;

import java.util.List;
import java.util.ArrayList;
import android.util.Log;
import com.matter.casting.support.TargetAppInfo;

/**
 * This class contains the optional parameters used in the IdentificationDeclaration Message, sent by the Commissionee to the
 * Commissioner. The options specify information relating to the requested UDC commissioning session.
 */
public class IdentificationDeclarationOptions {
    static final String TAG = IdentificationDeclarationOptions.class.getSimpleName();
    private static final int CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS = 10;

    public IdentificationDeclarationOptions() {}

    /**
     * Feature: Target Content Application
     * Flag to instruct the Commissioner not to display a Passcode input dialog, and instead send a CommissionerDeclaration message
     * if a commissioning Passcode is needed.
     */
    public boolean mNoPasscode = false;
    /**
     * Feature: Coordinate Passcode Dialogs
     * Flag to instruct the Commissioner to send a CommissionerDeclaration message when the Passcode input dialog on the
     * Commissioner has been shown to the user.
     */
    public boolean mCdUponPasscodeDialog = false;
    /**
     * Feature: Commissioner-Generated Passcode
     * Flag to instruct the Commissioner to use the Commissioner-generated Passcode for commissioning.
     */
    public boolean mCommissionerPasscode = false;
    /**
     * Feature: Commissioner-Generated Passcode
     * Flag to indicate whether or not the Commissionee has obtained the Commissioner Passcode from the user and is therefore ready
     * for commissioning.
     */
    public boolean mCommissionerPasscodeReady = false;
    /**
     * Feature: Coordinate Passcode Dialogs
     * Flag to indicate when the Commissionee user has decided to exit the commissioning process.
     */
    public boolean mCancelPasscode = false;
    /**
     * Feature: Target Content Application
     * The set of content app Vendor IDs (and optionally, Product IDs) that can be used for authentication.
     * Also, if TargetAppInfo is passed in, VerifyOrEstablishConnection() will force User Directed Commissioning, in case the
     * desired TargetApp is not found in the on-device CastingStore.
     */
    private List<TargetAppInfo> mTargetAppInfos = new ArrayList<>();

    public boolean addTargetAppInfo(TargetAppInfo targetAppInfo) {
        Log.d(TAG, "addTargetAppInfo()");
        if (mTargetAppInfos.size() >= CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS) {
            Log.e(TAG, "addTargetAppInfo() failed to add TargetAppInfo, max list size is {0}" + CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS);
            return false;
        }
        mTargetAppInfos.add(targetAppInfo);
        return true;
    }

    public List<TargetAppInfo> getTargetAppInfoList() {
        return mTargetAppInfos;
    }

    public void logDetail() {
        Log.d(TAG, "IdentificationDeclarationOptions::logDetail() - java");
        Log.d(TAG, "IdentificationDeclarationOptions::mNoPasscode:                " + mNoPasscode);
        Log.d(TAG, "IdentificationDeclarationOptions::mCdUponPasscodeDialog:      " + mCdUponPasscodeDialog);
        Log.d(TAG, "IdentificationDeclarationOptions::mCommissionerPasscode:      " + mCommissionerPasscode);
        Log.d(TAG, "IdentificationDeclarationOptions::mCommissionerPasscodeReady: " + mCommissionerPasscodeReady);
        Log.d(TAG, "IdentificationDeclarationOptions::mCancelPasscode:            " + mCancelPasscode);
        Log.d(TAG, "IdentificationDeclarationOptions::mTargetAppInfos list: ");

        for (TargetAppInfo targetAppInfo : mTargetAppInfos) {
            Log.d(TAG, "\t\tTargetAppInfo - Vendor ID: " + targetAppInfo.vendorId + ", Product ID: " + targetAppInfo.productId);
        }
    }
}
