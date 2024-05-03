/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "Types.h"
#include <vector>

namespace matter {
namespace casting {
namespace core {

/**
 * This class contains the optional parameters used in the IdentificationDeclaration Message, sent by the Commissionee to the
 * Commissioner. The options specify information relating to the requested UDC commissioning session.
 */
class IdentificationDeclarationOptions
{
public:
    /**
     * Feature: Target Content Application
     * Flag to instruct the Commissioner not to display a Passcode input dialog, and instead send a CommissionerDeclaration message
     * if a commissioning Passcode is needed.
     */
    bool mNoPasscode = false;
    /**
     * Feature: Coordinate Passcode Dialogs
     * Flag to instruct the Commissioner to send a CommissionerDeclaration message when the Passcode input dialog on the
     * Commissioner has been shown to the user.
     */
    bool mCdUponPasscodeDialog = false;
    /**
     * Feature: Commissioner-Generated Passcode
     * Flag to instruct the Commissioner to use the Commissioner-generated Passcode for commissioning.
     */
    bool mCommissionerPasscode = false;
    /**
     * Feature: Commissioner-Generated Passcode
     * Flag to indicate whether or not the Commissionee has obtained the Commissioner Passcode from the user and is therefore ready
     * for commissioning.
     */
    bool mCommissionerPasscodeReady = false;
    /**
     * Feature: Coordinate Passcode Dialogs
     * Flag to indicate when the Commissionee user has decided to exit the commissioning process.
     */
    bool mCancelPasscode = false;

    CHIP_ERROR addTargetAppInfo(const chip::Protocols::UserDirectedCommissioning::TargetAppInfo & targetAppInfo)
    {
        if (mTargetAppInfos.size() >= kMaxTargetAppInfos)
        {
            ChipLogError(AppServer,
                         "IdentificationDeclarationOptions::addTargetAppInfo() failed to add TargetAppInfo, max vector size is %zu",
                         kMaxTargetAppInfos);
            return CHIP_ERROR_NO_MEMORY;
        }

        mTargetAppInfos.push_back(targetAppInfo);
        return CHIP_NO_ERROR;
    }

    std::vector<chip::Protocols::UserDirectedCommissioning::TargetAppInfo> getTargetAppInfoList() const { return mTargetAppInfos; }

private:
    // TVs can handle the memory impact of supporting a larger list. See examples/tv-app/tv-common/include/CHIPProjectAppConfig.h
    constexpr static size_t kMaxTargetAppInfos = 10;
    /**
     * Feature: Target Content Application
     * The set of content app Vendor IDs (and optionally, Product IDs) that can be used for authentication.
     * Also, if TargetAppInfo is passed in, VerifyOrEstablishConnection() will force User Directed Commissioning, in case the
     * desired TargetApp is not found in the on-device CastingStore.
     */
    std::vector<chip::Protocols::UserDirectedCommissioning::TargetAppInfo> mTargetAppInfos;
};

}; // namespace core
}; // namespace casting
}; // namespace matter
