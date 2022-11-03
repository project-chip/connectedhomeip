/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "commands/common/CHIPCommand.h"
#include "commands/common/Commands.h"
#include "commands/interactive/InteractiveCommands.h"

void registerCommandsInteractive(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "interactive";

    commands_list clusterCommands = {
#if CONFIG_USE_INTERACTIVE_MODE
        make_unique<InteractiveStartCommand>(&commands, credsIssuerConfig),
#endif // CONFIG_USE_INTERACTIVE_MODE
    };

    commands.Register(clusterName, clusterCommands);
}
