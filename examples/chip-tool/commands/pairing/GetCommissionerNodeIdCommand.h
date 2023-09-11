/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "../common/CHIPCommand.h"

class GetCommissionerNodeIdCommand : public CHIPCommand
{
public:
    GetCommissionerNodeIdCommand(CredentialIssuerCommands * credIssuerCommands) :
        CHIPCommand("get-commissioner-node-id", credIssuerCommands)
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        chip::NodeId id;
        ReturnErrorOnFailure(GetIdentityNodeId(GetIdentity(), &id));
        ChipLogProgress(chipTool, "Commissioner Node Id 0x:" ChipLogFormatX64, ChipLogValueX64(id));

        ReturnErrorOnFailure(RemoteDataModelLogger::LogGetCommissionerNodeId(id));
        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }
};
