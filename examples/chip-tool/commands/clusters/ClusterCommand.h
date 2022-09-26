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

#include <app/tests/suites/commands/interaction_model/InteractionModel.h>

#include "DataModelLogger.h"
#include "ModelCommand.h"

class ClusterCommand : public InteractionModelCommands, public ModelCommand, public chip::app::CommandSender::Callback
{
public:
    ClusterCommand(CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelCommands(this), ModelCommand("command-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload);
        AddArguments();
    }

    ClusterCommand(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelCommands(this), ModelCommand("command-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload);
        AddArguments();
    }

    ~ClusterCommand() {}

    CHIP_ERROR SendCommand(chip::DeviceProxy * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return InteractionModelCommands::SendCommand(device, endpointIds.at(0), mClusterId, mCommandId, mPayload,
                                                     mTimedInteractionTimeoutMs, mSuppressResponse, mRepeatCount, mRepeatDelayInMs);
    }

    template <class T>
    CHIP_ERROR SendCommand(chip::DeviceProxy * device, chip::EndpointId endpointId, chip::ClusterId clusterId,
                           chip::CommandId commandId, const T & value)
    {
        return InteractionModelCommands::SendCommand(device, endpointId, clusterId, commandId, value, mTimedInteractionTimeoutMs,
                                                     mSuppressResponse, mRepeatCount, mRepeatDelayInMs);
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex) override
    {
        return InteractionModelCommands::SendGroupCommand(groupId, fabricIndex, mClusterId, mCommandId, mPayload);
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::ClusterId clusterId,
                                chip::CommandId commandId, const T & value)
    {
        return InteractionModelCommands::SendGroupCommand(groupId, fabricIndex, clusterId, commandId, value);
    }

    /////////// CommandSender Callback Interface /////////
    virtual void OnResponse(chip::app::CommandSender * client, const chip::app::ConcreteCommandPath & path,
                            const chip::app::StatusIB & status, chip::TLV::TLVReader * data) override
    {
        CHIP_ERROR error = status.ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(chipTool, "Response Failure: %s", chip::ErrorStr(error));
            mError = error;
            return;
        }

        if (data != nullptr)
        {
            error = DataModelLogger::LogCommand(path, data);
            if (CHIP_NO_ERROR != error)
            {
                ChipLogError(chipTool, "Response Failure: Can not decode Data");
                mError = error;
                return;
            }
        }
    }

    virtual void OnError(const chip::app::CommandSender * client, CHIP_ERROR error) override
    {
        ChipLogProgress(chipTool, "Error: %s", chip::ErrorStr(error));
        mError = error;
    }

    virtual void OnDone(chip::app::CommandSender * client) override
    {
        if (mCommandSender.size())
        {
            mCommandSender.front().reset();
            mCommandSender.erase(mCommandSender.begin());
        }

        // If the command is repeated N times, wait for all the responses to comes in
        // before exiting.
        bool shouldStop = true;
        if (mRepeatCount.HasValue())
        {
            mRepeatCount.SetValue(static_cast<uint16_t>(mRepeatCount.Value() - 1));
            shouldStop = mRepeatCount.Value() == 0;
        }

        if (shouldStop)
        {
            SetCommandExitStatus(mError);
        }
    }

    void Shutdown() override
    {
        mError = CHIP_NO_ERROR;
        ModelCommand::Shutdown();
    }

protected:
    ClusterCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        InteractionModelCommands(this), ModelCommand(commandName, credsIssuerConfig)
    {
        // Subclasses are responsible for calling AddArguments.
    }

    void AddArguments()
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs,
                    "If provided, do a timed invoke with the given timed interaction timeout.");
        AddArgument("suppressResponse", 0, 1, &mSuppressResponse);
        AddArgument("repeat-count", 1, UINT16_MAX, &mRepeatCount);
        AddArgument("repeat-delay-ms", 0, UINT16_MAX, &mRepeatDelayInMs);
        ModelCommand::AddArguments();
    }

private:
    chip::ClusterId mClusterId;
    chip::CommandId mCommandId;
    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;
    chip::Optional<bool> mSuppressResponse;
    chip::Optional<uint16_t> mRepeatCount;
    chip::Optional<uint16_t> mRepeatDelayInMs;

    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mPayload;
};
