/*
 *   Copyright (c) 2020 Project CHIP Authors
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

#include "../../config/PersistentStorage.h"
#include "../common/CHIPCommand.h"
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <setup_payload/SetupPayload.h>


class MulticastCommand : public CHIPCommand
{
public:
    MulticastCommand(const char * commandName) : CHIPCommand(commandName)
    {
        AddArgument("group-id", 0, UINT16_MAX, &mGroupId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }
    void Shutdown() override;

private:
    NodeId mGroupId;
};
