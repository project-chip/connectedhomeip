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

class GetCommissionerRootCertificateCommand : public CHIPCommand
{
public:
    GetCommissionerRootCertificateCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("get-commissioner-root-certificate", credIssuerCommands,
                    "Returns a base64-encoded RCAC prefixed with: 'base64:'")
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        chip::ByteSpan span;
        ReturnErrorOnFailure(GetIdentityRootCertificate(GetIdentity(), span));

        std::string rcac;
        ReturnErrorOnFailure(ToTLVCert(span, rcac));
        ChipLogProgress(NotSpecified, "RCAC: %s", rcac.c_str());

        ReturnErrorOnFailure(RemoteDataModelLogger::LogGetCommissionerRootCertificate(rcac.c_str()));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }
};
