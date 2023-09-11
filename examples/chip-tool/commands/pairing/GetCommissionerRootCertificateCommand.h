/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommand.h"

#include "ToTLVCert.h"

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
        ChipLogProgress(chipTool, "RCAC: %s", rcac.c_str());

        ReturnErrorOnFailure(RemoteDataModelLogger::LogGetCommissionerRootCertificate(rcac.c_str()));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }
};
