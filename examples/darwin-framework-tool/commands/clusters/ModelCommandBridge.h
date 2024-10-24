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

#define DFT_MODEL_COMMAND_DEFAULT_TIMEOUT 20
#define DFT_STRINGIFY_HELPER(arg) #arg
#define DFT_STRINGIFY(arg) DFT_STRINGIFY_HELPER(arg)

class ModelCommand : public CHIPCommandBridge
{
public:
    ModelCommand(const char * _Nonnull commandName) : CHIPCommandBridge(commandName) {}

    void AddArguments()
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
        AddArgument("endpoint-id", 0, UINT16_MAX, &mEndPointId);
        AddArgument(
            "timeout", 0, UINT16_MAX, &mTimeout,
            "Amount of time to allow the command to run for before considering it to have timed out.  Defaults to " DFT_STRINGIFY(
                DFT_MODEL_COMMAND_DEFAULT_TIMEOUT) " seconds.");
    }

    void Shutdown() override;

    void Cleanup() override { ModelCommand::Shutdown(); }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override;
    chip::System::Clock::Timeout GetWaitDuration() const override
    {
        return chip::System::Clock::Seconds16(mTimeout.ValueOr(DFT_MODEL_COMMAND_DEFAULT_TIMEOUT));
    }

    virtual CHIP_ERROR SendCommand(MTRBaseDevice * _Nonnull device, chip::EndpointId endPointId) = 0;

private:
    chip::NodeId mNodeId;
    chip::EndpointId mEndPointId;
    chip::Optional<uint16_t> mTimeout;
};

#undef DFT_STRINGIFY
#undef DFT_STRINGIFY_HELPER
#undef DFT_MODEL_COMMAND_DEFAULT_TIMEOUT
