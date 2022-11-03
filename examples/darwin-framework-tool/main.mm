/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Matter/Matter.h>

#include "commands/common/Commands.h"
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
        Commands commands;
        registerCommandsPairing(commands);
        registerCommandsInteractive(commands);
        registerCommandsPayload(commands);
        registerClusterOtaSoftwareUpdateProviderInteractive(commands);
        registerCommandsStorage(commands);
        registerCommandsTests(commands);
        registerClusters(commands);
        return commands.Run(argc, (char **) argv);
    }
}
