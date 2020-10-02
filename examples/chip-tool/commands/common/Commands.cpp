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

#include "Commands.h"

#include "Command.h"
#include <string>

void Commands::Register(std::unique_ptr<Command> command)
{
    commands.push_back(std::move(command));
}

int Commands::Run(NodeId localId, NodeId remoteId, int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipDeviceController dc;

    err = dc.Init(localId);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure: %s", chip::ErrorStr(err)));

    err = dc.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Run Loop failure: %s", chip::ErrorStr(err)));

    err = RunCommand(dc, remoteId, argc, argv);
    SuccessOrExit(err);

exit:
    dc.ServiceEventSignal();
    dc.Shutdown();
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

CHIP_ERROR Commands::RunCommand(ChipDeviceController & dc, NodeId remoteId, int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(argc > 1, err = CHIP_ERROR_INVALID_ARGUMENT);

    for (size_t i = 0; i < commands.size(); i++)
    {
        Command * cmd = commands.at(i).get();
        if (strcmp(cmd->GetName(), argv[1]) == 0)
        {
            // If the command is a read command, ensure the target attribute value matches with the last argument
            if (strcmp(cmd->GetName(), "read") == 0 && strcmp(cmd->GetAttribute(), argv[argc - 1]) != 0)
            {
                continue;
            }

            VerifyOrExit(cmd->InitArguments(argc - 2, &argv[2]), err = CHIP_ERROR_INVALID_ARGUMENT);

            err = cmd->Run(&dc, remoteId);
            SuccessOrExit(err);

            return err;
        }
    }

    // No command found.
    ChipLogError(chipTool, "Unknown command: %s", argv[1]);
    Commands::ShowUsage(argv[0]);
    return CHIP_ERROR_NOT_IMPLEMENTED;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err));
        Commands::ShowUsage(argv[0]);
    }

    return err;
}

void Commands::ShowUsage(const char * executable)
{
    std::string arguments  = "";
    std::string attributes = "";
    size_t commandsCount   = commands.size();

    for (size_t i = 0; i < commandsCount; i++)
    {
        const Command * command = commands.at(i).get();

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
