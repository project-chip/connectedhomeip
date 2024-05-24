/*
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
 *
 */

#pragma once

#include <commands/common/CHIPCommand.h>

class FabricSyncAddDeviceCommand : public CHIPCommand
{
public:
    FabricSyncAddDeviceCommand(CredentialIssuerCommands * credIssuerCommands) : CHIPCommand("add-device", credIssuerCommands)
    {
        AddArgument("nodeid", 0, UINT64_MAX, &mNodeId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override { return RunCommand(mNodeId); }

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

private:
    chip::NodeId mNodeId;

    CHIP_ERROR RunCommand(NodeId remoteId);
};
