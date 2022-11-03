/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "SetupPayloadParseCommand.h"

void registerCommandsPayload(Commands & commands)
{
    const char * clusterName      = "Payload";
    commands_list clusterCommands = {
        make_unique<SetupPayloadParseCommand>(), //
    };

    commands.Register(clusterName, clusterCommands);
}
