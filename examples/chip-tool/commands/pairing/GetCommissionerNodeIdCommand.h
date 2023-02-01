/*
 *   Copyright (c) 2023 Project CHIP Authors
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
