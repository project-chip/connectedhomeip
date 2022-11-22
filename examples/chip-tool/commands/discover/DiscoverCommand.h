/*
 *   Copyright (c) 2021 Project CHIP Authors
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
#include <controller/ExamplePersistentStorage.h>

class DiscoverCommand : public CHIPCommand
{
public:
    DiscoverCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand(commandName, credsIssuerConfig)
    {
        AddArgument("nodeid", 0, UINT64_MAX, &mNodeId);
        AddArgument("fabricid", 0, UINT64_MAX, &mFabricId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(30); }

    virtual CHIP_ERROR RunCommand(NodeId remoteId, uint64_t fabricId) = 0;

private:
    chip::NodeId mNodeId;
    uint64_t mFabricId;
};
