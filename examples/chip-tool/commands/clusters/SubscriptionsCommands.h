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

#include <lib/core/CHIPError.h>

#include <commands/common/CHIPCommand.h>
#include <commands/common/Commands.h>

class ShutdownSubscription : public CHIPCommand
{
public:
    ShutdownSubscription(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("shutdown-subscription", credsIssuerConfig)
    {
        AddArgument("subscription-id", 0, UINT64_MAX, &mSubscriptionId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        CHIP_ERROR err = chip::app::InteractionModelEngine::GetInstance()->ShutdownSubscription(mSubscriptionId);
        SetCommandExitStatus(err);
        return CHIP_NO_ERROR;
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

private:
    chip::SubscriptionId mSubscriptionId;
};

class ShutdownSubscriptions : public CHIPCommand
{
public:
    ShutdownSubscriptions(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("shutdown-subscriptions", credsIssuerConfig)
    {
        AddArgument("fabric-index", 0, UINT64_MAX, &mFabricIndex);
        AddArgument("node-id", 0, UINT64_MAX, &mNodeId);
    }

    /////////// CHIPCommand Interface /////////
    CHIP_ERROR RunCommand() override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        chip::app::InteractionModelEngine::GetInstance()->ShutdownSubscriptions(mFabricIndex, mNodeId);

        SetCommandExitStatus(err);
        return CHIP_NO_ERROR;
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(10); }

private:
    chip::FabricIndex mFabricIndex;
    chip::NodeId mNodeId;
};

void registerClusterSubscriptions(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "Subscriptions";

    commands_list clusterCommands = {
        make_unique<ShutdownSubscription>(credsIssuerConfig),  //
        make_unique<ShutdownSubscriptions>(credsIssuerConfig), //
    };

    commands.Register(clusterName, clusterCommands);
}
