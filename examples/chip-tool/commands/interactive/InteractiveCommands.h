/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "../common/CHIPCommand.h"
#include "../common/Commands.h"

#include "InteractiveCommands.h"

class Commands;

class InteractiveStartCommand : public CHIPCommand
{
public:
    InteractiveStartCommand(Commands * commandsHandler, CredentialIssuerCommands * credsIssuerConfig) :
        CHIPCommand("start", credsIssuerConfig), mHandler(commandsHandler)
    {}

    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }

private:
    bool ParseCommand(char * command);
    Commands * mHandler = nullptr;
};
