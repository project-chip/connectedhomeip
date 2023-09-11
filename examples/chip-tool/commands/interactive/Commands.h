/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
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
        make_unique<InteractiveServerCommand>(&commands, credsIssuerConfig),
#endif // CONFIG_USE_INTERACTIVE_MODE
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for starting long-lived interactive modes.");
}
