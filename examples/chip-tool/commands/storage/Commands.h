/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "StorageManagementCommand.h"
#include <commands/common/Commands.h>

void registerCommandsStorage(Commands & commands)
{
    const char * clusterName = "storage";

    commands_list clusterCommands = { make_unique<StorageClearAll>() };

    commands.Register(clusterName, clusterCommands);
}
