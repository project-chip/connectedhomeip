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

void Commands::Register(const char * clusterName, commands_list commandsList)
{
    for (auto & command : commandsList)
    {
        clusters[clusterName].push_back(std::move(command));
    }
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

    for (auto & cluster : clusters)
    {
        for (auto & command : cluster.second)
        {
            if (strcmp(command->GetName(), argv[1]) == 0)
            {
                // If the command is a read command, ensure the target attribute value matches with the last argument
                if (strcmp(command->GetName(), "read") == 0 && strcmp(command->GetAttribute(), argv[argc - 1]) != 0)
                {
                    continue;
                }

                VerifyOrExit(command->InitArguments(argc - 2, &argv[2]), err = CHIP_ERROR_INVALID_ARGUMENT);

                err = command->Run(&dc, remoteId);
                SuccessOrExit(err);

                return err;
            }
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
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s command [params]\n\n", executable);
    fprintf(stderr, "  Supported commands and their parameters:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Misc. Commands:                                                                     |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    PrintMiscCommands();
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Clusters:                                                                           |\n");
    fprintf(stderr, "  |                                                                                     |\n");
    fprintf(stderr, "  |   Usage:                                                                            |\n");
    fprintf(stderr, "  |    command_name remote-ip remote-port endpoint-id                                   |\n");
    fprintf(stderr, "  |                                                                                     |\n");
    fprintf(stderr, "  | Available command names:                                                            |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    PrintClustersCommands();
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Read Attributes:                                                                    |\n");
    fprintf(stderr, "  |                                                                                     |\n");
    fprintf(stderr, "  |   Usage:                                                                            |\n");
    fprintf(stderr, "  |    read remote-ip remote-port endpoint-id attribute-name                            |\n");
    fprintf(stderr, "  |                                                                                     |\n");
    fprintf(stderr, "  | Available attribute names:                                                          |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    PrintClustersAttributes();
}

void Commands::PrintMiscCommands()
{
    for (auto & cluster : clusters)
    {
        if (cluster.first[0] == '\0')
        {
            for (auto & command : cluster.second)
            {
                std::string arguments = "";
                arguments += command->GetName();

                size_t argumentsCount = command->GetArgumentsCount();
                for (size_t j = 0; j < argumentsCount; j++)
                {
                    arguments += " ";
                    arguments += command->GetArgumentName(j);
                }

                fprintf(stderr, "  | %-84s|\n", arguments.c_str());
            }
        }
    }
}

void PrintClusterHeader(const char * name)
{
    fprintf(stderr, "  | * Cluster: %-73s|\n", name);
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void PrintClusterFooter()
{
    fprintf(stderr, "  | %-84s|\n", "");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::PrintClustersCommands()
{
    for (auto & cluster : clusters)
    {
        bool hasCommands = false;

        if (cluster.first[0] != '\0')
        {
            for (auto & command : cluster.second)
            {
                if (strcmp("read", command->GetName()) != 0)
                {
                    if (!hasCommands)
                    {
                        PrintClusterHeader(cluster.first);
                        hasCommands = true;
                    }

                    fprintf(stderr, "  | %-84s|\n", command->GetName());
                }
            }
        }

        if (hasCommands)
        {
            PrintClusterFooter();
        }
    }
}

void Commands::PrintClustersAttributes()
{
    for (auto & cluster : clusters)
    {
        bool hasAttributes = false;

        if (cluster.first[0] != '\0')
        {
            for (auto & command : cluster.second)
            {
                if (strcmp("read", command->GetName()) == 0)
                {
                    if (!hasAttributes)
                    {
                        PrintClusterHeader(cluster.first);
                        hasAttributes = true;
                    }

                    fprintf(stderr, "  | %-84s|\n", command->GetAttribute());
                }
            }
        }

        if (hasAttributes)
        {
            PrintClusterFooter();
        }
    }
}
