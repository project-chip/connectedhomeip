/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
