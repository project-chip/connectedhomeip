/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Matter/Matter.h>

#include "../common/CHIPCommandBridge.h"
#include <commands/common/Command.h>

#include "InteractiveCommands.h"

void registerCommandsInteractive(Commands & commands)
{
    const char * clusterName = "interactive";

    commands_list clusterCommands = {
#if CONFIG_USE_INTERACTIVE_MODE
        make_unique<InteractiveStartCommand>(&commands),
#endif // CONFIG_USE_INTERACTIVE_MODE
    };

    commands.Register(clusterName, clusterCommands);
}
