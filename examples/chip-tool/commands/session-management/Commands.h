/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#include "commands/common/Commands.h"
#include "commands/session-management/CloseSessionCommand.h"

void registerCommandsSessionManagement(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "SessionManagement";

    commands_list clusterCommands = {
        make_unique<SendCloseSessionCommand>(credsIssuerConfig),
        make_unique<EvictLocalCASESessionsCommand>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for managing CASE and PASE session state.");
}
