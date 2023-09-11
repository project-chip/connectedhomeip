/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#import <Matter/Matter.h>

#import "logging/logging.h"

#include "commands/common/Commands.h"
#include "commands/delay/Commands.h"
#include "commands/discover/Commands.h"
#include "commands/interactive/Commands.h"
#include "commands/pairing/Commands.h"
#include "commands/payload/Commands.h"
#include "commands/provider/Commands.h"
#include "commands/storage/Commands.h"

#include <zap-generated/cluster/Commands.h>
#include <zap-generated/test/Commands.h>

int main(int argc, const char * argv[])
{
    @autoreleasepool {
        dft::logging::Setup();

        Commands commands;
        registerCommandsPairing(commands);
        registerCommandsDelay(commands);
        registerCommandsDiscover(commands);
        registerCommandsInteractive(commands);
        registerCommandsPayload(commands);
        registerClusterOtaSoftwareUpdateProviderInteractive(commands);
        registerCommandsStorage(commands);
        registerCommandsTests(commands);
        registerClusters(commands);
        return commands.Run(argc, (char **) argv);
    }
}
