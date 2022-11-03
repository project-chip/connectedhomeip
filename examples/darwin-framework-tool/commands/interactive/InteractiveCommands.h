/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Matter/Matter.h>

#include "../common/CHIPCommandBridge.h"
#include "commands/common/Commands.h"

#include "InteractiveCommands.h"

class Commands;

class InteractiveStartCommand : public CHIPCommandBridge
{
public:
    InteractiveStartCommand(Commands * commandsHandler) : CHIPCommandBridge("start"), mHandler(commandsHandler) {}

    CHIP_ERROR RunCommand() override;

    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(0); }

private:
    bool ParseCommand(char * command);
    Commands * mHandler = nullptr;
};
