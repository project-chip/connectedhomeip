/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <commands/common/CHIPCommand.h>
#include <commands/common/RemoteDataModelLogger.h>

#include "ToTLVCert.h"

#include <string>

class IssueNOCChainCommand : public CHIPCommand
{
public:
    IssueNOCChainCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("issue-noc-chain", credIssuerCommands,
                    "Returns a base64-encoded NOC, ICAC, RCAC, and IPK prefixed with: 'base64:'"),
        mDeviceNOCChainCallback(OnDeviceNOCChainGeneration, this)
    {
        AddArgument("elements", &mNOCSRElements, "NOCSRElements encoded in hexadecimal");
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId, "The target node id");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        auto & commissioner = CurrentCommissioner();
        ReturnErrorOnFailure(commissioner.IssueNOCChain(mNOCSRElements, mNodeId, &mDeviceNOCChainCallback));
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

    static void OnDeviceNOCChainGeneration(void * context, CHIP_ERROR status, const chip::ByteSpan & noc,
                                           const chip::ByteSpan & icac, const chip::ByteSpan & rcac,
                                           chip::Optional<chip::Crypto::IdentityProtectionKeySpan> ipk,
                                           chip::Optional<chip::NodeId> adminSubject)
    {
        auto command = static_cast<IssueNOCChainCommand *>(context);

        auto err = status;
        VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));

        std::string nocStr;
        err = ToTLVCert(noc, nocStr);
        VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
        ChipLogProgress(NotSpecified, "NOC: %s", nocStr.c_str());

        std::string icacStr;
        err = ToTLVCert(icac, icacStr);
        VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
        ChipLogProgress(NotSpecified, "ICAC: %s", icacStr.c_str());

        std::string rcacStr;
        err = ToTLVCert(rcac, rcacStr);
        VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
        ChipLogProgress(NotSpecified, "RCAC: %s", rcacStr.c_str());

        std::string ipkStr;
        if (ipk.HasValue())
        {
            err = ToBase64(ipk.Value(), ipkStr);
            VerifyOrReturn(CHIP_NO_ERROR == err, command->SetCommandExitStatus(err));
        }
        ChipLogProgress(NotSpecified, "IPK: %s", ipkStr.c_str());

        err = RemoteDataModelLogger::LogIssueNOCChain(nocStr.c_str(), icacStr.c_str(), rcacStr.c_str(), ipkStr.c_str());
        command->SetCommandExitStatus(err);
    }

private:
    chip::Callback::Callback<chip::Controller::OnNOCChainGeneration> mDeviceNOCChainCallback;
    chip::ByteSpan mNOCSRElements;
    chip::NodeId mNodeId;
};
