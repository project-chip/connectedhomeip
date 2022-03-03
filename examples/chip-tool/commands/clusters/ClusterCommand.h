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

#include <app/CommandSender.h>

#include "DataModelLogger.h"
#include "ModelCommand.h"

class ClusterCommand : public ModelCommand, public chip::app::CommandSender::Callback
{
public:
    ClusterCommand(CredentialIssuerCommands * credsIssuerConfig) : ModelCommand("command-by-id", credsIssuerConfig)
    {
        AddArgument("cluster-id", 0, UINT32_MAX, &mClusterId);
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        ModelCommand::AddArguments();
    }

    ClusterCommand(chip::ClusterId clusterId, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand("command-by-id", credsIssuerConfig), mClusterId(clusterId)
    {
        AddArgument("command-id", 0, UINT32_MAX, &mCommandId);
        AddArgument("payload", &mPayload);
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
        ModelCommand::AddArguments();
    }

    ClusterCommand(const char * commandName, CredentialIssuerCommands * credsIssuerConfig) :
        ModelCommand(commandName, credsIssuerConfig)
    {
        AddArgument("timedInteractionTimeoutMs", 0, UINT16_MAX, &mTimedInteractionTimeoutMs);
    }

    ~ClusterCommand() {}

    CHIP_ERROR SendCommand(ChipDevice * device, std::vector<chip::EndpointId> endpointIds) override
    {
        return ClusterCommand::SendCommand(device, endpointIds.at(0), mClusterId, mCommandId, mPayload);
    }

    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::NodeId senderNodeId) override
    {
        return ClusterCommand::SendGroupCommand(groupId, fabricIndex, senderNodeId, mClusterId, mCommandId, mPayload);
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
        mCommandSender.reset();
        SetCommandExitStatus(mError);
    }

    template <class T>
    CHIP_ERROR SendCommand(ChipDevice * device, chip::EndpointId endpointId, chip::ClusterId clusterId, chip::CommandId commandId,
                           const T & value)
    {
        chip::app::CommandPathParams commandPath = { endpointId, 0 /* groupId */, clusterId, commandId,
                                                     (chip::app::CommandPathFlags::kEndpointIdValid) };
        mCommandSender =
            std::make_unique<chip::app::CommandSender>(this, device->GetExchangeManager(), mTimedInteractionTimeoutMs.HasValue());
        VerifyOrReturnError(mCommandSender != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(mCommandSender->AddRequestDataNoTimedCheck(commandPath, value, mTimedInteractionTimeoutMs));
        ReturnErrorOnFailure(mCommandSender->SendCommandRequest(device->GetSecureSession().Value()));
        return CHIP_NO_ERROR;
    }

    template <class T>
    CHIP_ERROR SendGroupCommand(chip::GroupId groupId, chip::FabricIndex fabricIndex, chip::NodeId senderNodeId,
                                chip::ClusterId clusterId, chip::CommandId commandId, const T & value)
    {
        chip::app::CommandPathParams commandPath = { 0 /* endpoint */, groupId, clusterId, commandId,
                                                     (chip::app::CommandPathFlags::kGroupIdValid) };

        chip::Messaging::ExchangeManager * exchangeManager = chip::app::InteractionModelEngine::GetInstance()->GetExchangeManager();

        auto commandSender =
            chip::Platform::MakeUnique<chip::app::CommandSender>(this, exchangeManager, mTimedInteractionTimeoutMs.HasValue());
        VerifyOrReturnError(commandSender != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(commandSender->AddRequestDataNoTimedCheck(commandPath, value, mTimedInteractionTimeoutMs));

        chip::Transport::OutgoingGroupSession session(groupId, fabricIndex, senderNodeId);
        ReturnErrorOnFailure(commandSender->SendGroupCommandRequest(chip::SessionHandle(session)));
        commandSender.release();

        return CHIP_NO_ERROR;
    }

private:
    chip::ClusterId mClusterId;
    chip::CommandId mCommandId;
    chip::Optional<uint16_t> mTimedInteractionTimeoutMs;

    CHIP_ERROR mError = CHIP_NO_ERROR;
    CustomArgument mPayload;
    std::unique_ptr<chip::app::CommandSender> mCommandSender;
};
