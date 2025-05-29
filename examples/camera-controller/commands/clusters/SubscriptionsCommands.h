/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/CHIPError.h>

#include <commands/common/CHIPCommand.h>
#include <commands/common/Commands.h>

class ShutdownSubscription : public CHIPCommand
{
public:
    ShutdownSubscription(CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand("shutdown-one", credsIssuerConfig,
                    "Shut down a single subscription, identified by its subscription id and target node id.")
    {
        AddArgument("subscription-id", 0, UINT32_MAX, &mSubscriptionId);
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId,
                    "The node id, scoped to the commissioner name the command is running under.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        CHIP_ERROR err = chip::app::InteractionModelEngine::GetInstance()->ShutdownSubscription(
            chip::ScopedNodeId(mNodeId, CurrentCommissioner().GetFabricIndex()), mSubscriptionId);
        SetCommandExitStatus(err);
        return CHIP_NO_ERROR;
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

private:
    chip::SubscriptionId mSubscriptionId;
    chip::NodeId mNodeId;
};

class ShutdownSubscriptionsForNode : public CHIPCommand
{
public:
    ShutdownSubscriptionsForNode(CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand("shutdown-all-for-node", credsIssuerConfig, "Shut down all subscriptions targeting a given node.")
    {
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId,
                    "The node id, scoped to the commissioner name the command is running under.");
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        chip::app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(CurrentCommissioner().GetFabricIndex(), mNodeId);

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

private:
    chip::NodeId mNodeId;
};

class ShutdownAllSubscriptions : public CHIPCommand
{
public:
    ShutdownAllSubscriptions(CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand("shutdown-all", credsIssuerConfig, "Shut down all subscriptions to all nodes.")
    {}

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        chip::app::InteractionModelEngine::GetInstance()->ShutdownAllSubscriptions();

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

private:
};

void registerCommandsSubscriptions(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "Subscriptions";

    commands_list clusterCommands = {
        make_unique<ShutdownSubscription>(credsIssuerConfig),         //
        make_unique<ShutdownSubscriptionsForNode>(credsIssuerConfig), //
        make_unique<ShutdownAllSubscriptions>(credsIssuerConfig),     //
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for shutting down subscriptions.");
}
