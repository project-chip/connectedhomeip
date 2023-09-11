/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#include "commands/common/Commands.h"
#include "commands/example/ExampleCredentialIssuerCommands.h"

#include "commands/clusters/SubscriptionsCommands.h"
#include "commands/delay/Commands.h"
#include "commands/discover/Commands.h"
#include "commands/group/Commands.h"
#include "commands/interactive/Commands.h"
#include "commands/pairing/Commands.h"
#include "commands/payload/Commands.h"
#include "commands/session-management/Commands.h"
#include "commands/storage/Commands.h"

#include <zap-generated/cluster/Commands.h>

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    ExampleCredentialIssuerCommands credIssuerCommands;
    Commands commands;
    registerCommandsDelay(commands, &credIssuerCommands);
    registerCommandsDiscover(commands, &credIssuerCommands);
    registerCommandsInteractive(commands, &credIssuerCommands);
    registerCommandsPayload(commands);
    registerCommandsPairing(commands, &credIssuerCommands);
    registerCommandsGroup(commands, &credIssuerCommands);
    registerClusters(commands, &credIssuerCommands);
    registerCommandsSubscriptions(commands, &credIssuerCommands);
    registerCommandsStorage(commands);
    registerCommandsSessionManagement(commands, &credIssuerCommands);

    return commands.Run(argc, argv);
}
