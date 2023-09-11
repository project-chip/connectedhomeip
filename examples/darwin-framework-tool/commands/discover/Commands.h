/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#pragma once

#import <Matter/Matter.h>

#include "DiscoverCommissionablesCommand.h"

void registerCommandsDiscover(Commands & commands)
{
    const char * clusterName = "Discover";

    commands_list clusterCommands = {
        make_unique<DiscoverCommissionablesStartCommand>(),
        make_unique<DiscoverCommissionablesStopCommand>(),
        make_unique<DiscoverCommissionablesListCommand>(),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands for device discovery.");
}
