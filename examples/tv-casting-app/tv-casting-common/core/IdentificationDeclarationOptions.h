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
#include <app/server/Dnssd.h>
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

    /**
     * Commissionee's (random) DNS-SD instance name. This field is mandatory and will be auto generated if not provided by the
     * client.
     */
    char mCommissioneeInstanceName[chip::Dnssd::Commission::kInstanceNameMaxLength + 1] = "";

    CHIP_ERROR addTargetAppInfo(const chip::Protocols::UserDirectedCommissioning::TargetAppInfo & targetAppInfo)
    {
        ChipLogProgress(AppServer, "IdentificationDeclarationOptions::addTargetAppInfo()");
        if (mTargetAppInfos.size() >= CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS)
        {
            ChipLogError(AppServer,
                         "IdentificationDeclarationOptions::addTargetAppInfo() failed to add TargetAppInfo, max vector size is %d",
                         CHIP_DEVICE_CONFIG_UDC_MAX_TARGET_APPS);
            return CHIP_ERROR_NO_MEMORY;
        }

        mTargetAppInfos.push_back(targetAppInfo);
        return CHIP_NO_ERROR;
    }

    std::vector<chip::Protocols::UserDirectedCommissioning::TargetAppInfo> getTargetAppInfoList() const { return mTargetAppInfos; }

    void resetState()
    {
        mNoPasscode                = false;
        mCdUponPasscodeDialog      = false;
        mCommissionerPasscode      = false;
        mCommissionerPasscodeReady = false;
        mCancelPasscode            = false;
        mTargetAppInfos.clear();
    }

    /**
     * @brief Builds an IdentificationDeclaration message to be sent to a CastingPlayer, given the options state specified in this
     * object.
     */
    chip::Protocols::UserDirectedCommissioning::IdentificationDeclaration buildIdentificationDeclarationMessage()
    {
        ChipLogProgress(AppServer, "IdentificationDeclarationOptions::buildIdentificationDeclarationMessage()");
        chip::Protocols::UserDirectedCommissioning::IdentificationDeclaration id;

        std::vector<chip::Protocols::UserDirectedCommissioning::TargetAppInfo> targetAppInfos = getTargetAppInfoList();
        for (size_t i = 0; i < targetAppInfos.size(); i++)
        {
            id.AddTargetAppInfo(targetAppInfos[i]);
        }
        id.SetNoPasscode(mNoPasscode);
        id.SetCdUponPasscodeDialog(mCdUponPasscodeDialog);
        id.SetCancelPasscode(mCancelPasscode);
        id.SetCommissionerPasscode(mCommissionerPasscode);
        if (mCommissionerPasscodeReady)
        {
            id.SetCommissionerPasscodeReady(true);
            id.SetInstanceName(mCommissioneeInstanceName);
        }
        else
        {
            ChipLogProgress(AppServer,
                            "IdentificationDeclarationOptions::buildIdentificationDeclarationMessage() generating InstanceName");
            CHIP_ERROR err = chip::app::DnssdServer::Instance().GetCommissionableInstanceName(mCommissioneeInstanceName,
                                                                                              sizeof(mCommissioneeInstanceName));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer,
                             "IdentificationDeclarationOptions::buildIdentificationDeclarationMessage() Failed to get mdns "
                             "instance name error: %" CHIP_ERROR_FORMAT,
                             err.Format());
            }
            else
            {
                id.SetInstanceName(mCommissioneeInstanceName);
                ChipLogProgress(AppServer,
                                "IdentificationDeclarationOptions::buildIdentificationDeclarationMessage() InstanceName set to: %s",
                                mCommissioneeInstanceName);
            }
        }
        LogDetail();
        return id;
    }

    void LogDetail()
    {
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::LogDetail() - cpp");
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::mNoPasscode:                %s",
                      mNoPasscode ? "true" : "false");
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::mCdUponPasscodeDialog:      %s",
                      mCdUponPasscodeDialog ? "true" : "false");
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::mCommissionerPasscode:      %s",
                      mCommissionerPasscode ? "true" : "false");
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::mCommissionerPasscodeReady: %s",
                      mCommissionerPasscodeReady ? "true" : "false");
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::mCancelPasscode:            %s",
                      mCancelPasscode ? "true" : "false");
        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::mCommissioneeInstanceName:  %s", mCommissioneeInstanceName);

        ChipLogDetail(AppServer, "IdentificationDeclarationOptions::TargetAppInfos list:");
        for (size_t i = 0; i < mTargetAppInfos.size(); i++)
        {
            const chip::Protocols::UserDirectedCommissioning::TargetAppInfo & info = mTargetAppInfos[i];
            ChipLogDetail(AppServer, "\t\tTargetAppInfo %d, Vendor ID: %u, Product ID: %u", int(i + 1), info.vendorId,
                          info.productId);
        }
    }

private:
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
