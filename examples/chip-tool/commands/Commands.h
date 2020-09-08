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
#include "Off.h"
#include "On.h"
#include "ReadOnOff.h"
#include "Toggle.h"

// The list of available commands
Echo echo;
EchoBle echoBle;
On on;
Off off;
Toggle toggle;
ReadOnOff readOnOff;

Command * Commands[] = {
    &echoBle, &echo, &on, &off, &toggle, &readOnOff,
};

Command * GetCommand(int argc, char * argv[])
{
    size_t commandsCount = ArraySize(Commands);
    for (size_t i = 0; i < commandsCount; i++)
    {
        Command * command = Commands[i];
        if (command->InitArguments(argc - 1, &argv[1]))
        {
            return command;
        }
    }

    return nullptr;
}

void ShowUsage(const char * executable)
{
    std::string arguments  = "";
    std::string attributes = "";

    size_t commandsCount = ArraySize(Commands);
    for (size_t i = 0; i < commandsCount; i++)
    {
        Command * command = Commands[i];

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

CHIP_ERROR RunCommand(ChipDeviceController * dc, NodeId remoteId, int argc, char * argv[])
{
    Command * cmd = GetCommand(argc, argv);
    if (cmd == nullptr)
    {
        ShowUsage(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    CHIP_ERROR err = cmd->Run(dc, Optional<NodeId>::Value(remoteId));
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Run command failure: %s", ErrorStr(err)));

exit:
    return err;
}

#endif // __CHIPTOOL_COMMANDS_H__
