/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef __CHIPTOOL_COMMANDS_H__
#define __CHIPTOOL_COMMANDS_H__

#include <string>

#include "Echo.h"
#include "EchoBle.h"
#include "Identify-Cluster-Commands.h"
#include "Off.h"
#include "On.h"
#include "ReadOnOff.h"
#include "Toggle.h"

void ShowUsage(Command * commands[], size_t commandsCount, const char * executable)
{
    std::string arguments  = "";
    std::string attributes = "";

    for (size_t i = 0; i < commandsCount; i++)
    {
        const Command * command = commands[i];

        arguments += "    ";
        arguments += command->GetName();

        size_t argumentsCount = command->GetArgumentsCount();
        for (size_t j = 0; j < argumentsCount; j++)
        {
            arguments += " ";
            arguments += command->GetArgumentName(j);
        }

        arguments += "\n";

        if (strcmp("read", command->GetName()) == 0)
        {
            attributes += "    " + std::string(command->GetAttribute()) + "\n";
        }
    }

    fprintf(stderr,
            "Usage: \n"
            "  %s command [params]\n\n"
            "  Supported commands and their parameters:\n%s\n"
            "  Supported attribute names for the 'read' command:\n%s",
            executable, arguments.c_str(), attributes.c_str());
}

CHIP_ERROR RunCommand(chip::DeviceController::ChipDeviceController * dc, chip::NodeId remoteId, int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Start list of available commands
    Echo echo;
    EchoBle echoBle;
    On on;
    Off off;
    Toggle toggle;
    ReadOnOff readOnOff;
    IdentifyQuery identifyQuery;
    Identify identify;

    Command * commands[] = { &echoBle, &echo, &on, &off, &toggle, &readOnOff, &identify, &identifyQuery };
    // End list of available commands

    VerifyOrExit(argc > 1, err = CHIP_ERROR_INVALID_ARGUMENT);

    for (size_t i = 0; i < ArraySize(commands); i++)
    {
        Command * cmd = commands[i];
        if (strcmp(cmd->GetName(), argv[1]) == 0)
        {
            VerifyOrExit(cmd->InitArguments(argc - 2, &argv[2]), err = CHIP_ERROR_INVALID_ARGUMENT);

            err = cmd->Run(dc, remoteId);
            SuccessOrExit(err);

            return err;
        }
    }

    // No command found.
    ChipLogError(chipTool, "Unknown command: %s", argv[1]);
    ShowUsage(commands, ArraySize(commands), argv[0]);
    return CHIP_ERROR_NOT_IMPLEMENTED;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err));
        ShowUsage(commands, ArraySize(commands), argv[0]);
    }

    return err;
}

#endif // __CHIPTOOL_COMMANDS_H__
