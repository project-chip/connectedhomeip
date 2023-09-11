/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "commands/common/Commands.h"
#include "commands/delay/SleepCommand.h"
#include "commands/delay/WaitForCommissioneeCommand.h"

void registerCommandsDelay(Commands & commands)
{
    const char * clusterName      = "Delay";
    commands_list clusterCommands = {
        make_unique<SleepCommand>(),               //
        make_unique<WaitForCommissioneeCommand>(), //
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for waiting for something to happen.");
}
