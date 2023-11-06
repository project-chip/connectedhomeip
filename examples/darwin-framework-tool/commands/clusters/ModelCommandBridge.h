/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include "../common/CHIPCommandBridge.h"
#include <lib/core/CHIPEncoding.h>

class ModelCommand : public CHIPCommandBridge
{
public:
    ModelCommand(const char * _Nonnull commandName) : CHIPCommandBridge(commandName) {}

    void AddArguments()
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("endpoint-id", 0, UINT16_MAX, &mEndPointId);
    }

    void Shutdown() override;

    void Cleanup() override { ModelCommand::Shutdown(); }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

    virtual CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endPointId) = 0;

private:
    chip::NodeId mNodeId;
    chip::EndpointId mEndPointId;
};
