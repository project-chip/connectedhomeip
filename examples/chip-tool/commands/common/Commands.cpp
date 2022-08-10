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

#include <algorithm>
#include <string>

#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

void Commands::Register(const char * clusterName, commands_list commandsList)
{
    for (auto & command : commandsList)
    {
        mClusters[clusterName].push_back(std::move(command));
    }
}

int Commands::Run(int argc, char ** argv)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Memory failure: %s", chip::ErrorStr(err)));

#ifdef CONFIG_USE_LOCAL_STORAGE
    err = mStorage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());
#endif // CONFIG_USE_LOCAL_STORAGE

    err = RunCommand(argc, argv);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err)));

exit:
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int Commands::RunInteractive(int argc, char ** argv)
{
    CHIP_ERROR err = RunCommand(argc, argv, true);
    if (err == CHIP_NO_ERROR)
    {
        return EXIT_SUCCESS;
    }
    ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err));
    return EXIT_FAILURE;
}

CHIP_ERROR Commands::RunCommand(int argc, char ** argv, bool interactive)
{
    std::map<std::string, CommandsVector>::iterator cluster;
    Command * command = nullptr;

    if (argc <= 1)
    {
        ChipLogError(chipTool, "Missing cluster name");
        ShowClusters(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    cluster = GetCluster(argv[1]);
    if (cluster == mClusters.end())
    {
        ChipLogError(chipTool, "Unknown cluster: %s", argv[1]);
        ShowClusters(argv[0]);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (argc <= 2)
    {
        ChipLogError(chipTool, "Missing command name");
        ShowCluster(argv[0], argv[1], cluster->second);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (!IsGlobalCommand(argv[2]))
    {
        command = GetCommand(cluster->second, argv[2]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown command: %s", argv[2]);
            ShowCluster(argv[0], argv[1], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else if (IsEventCommand(argv[2]))
    {
        if (argc <= 3)
        {
            ChipLogError(chipTool, "Missing event name");
            ShowClusterEvents(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(cluster->second, argv[2], argv[3]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown event: %s", argv[3]);
            ShowClusterEvents(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        if (argc <= 3)
        {
            ChipLogError(chipTool, "Missing attribute name");
            ShowClusterAttributes(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        command = GetGlobalCommand(cluster->second, argv[2], argv[3]);
        if (command == nullptr)
        {
            ChipLogError(chipTool, "Unknown attribute: %s", argv[3]);
            ShowClusterAttributes(argv[0], argv[1], argv[2], cluster->second);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    if (!command->InitArguments(argc - 3, &argv[3]))
    {
        ShowCommand(argv[0], argv[1], command);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return interactive ? command->RunAsInteractive() : command->Run();
}

std::map<std::string, Commands::CommandsVector>::iterator Commands::GetCluster(std::string clusterName)
{
    for (auto & cluster : mClusters)
    {
        std::string key(cluster.first);
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        if (key.compare(clusterName) == 0)
        {
            return mClusters.find(cluster.first);
        }
    }

    return mClusters.end();
}

Command * Commands::GetCommand(CommandsVector & commands, std::string commandName)
{
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0)
        {
            return command.get();
        }
    }

    return nullptr;
}

Command * Commands::GetGlobalCommand(CommandsVector & commands, std::string commandName, std::string attributeName)
{
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0 && attributeName.compare(command->GetAttribute()) == 0)
        {
            return command.get();
        }
    }

    return nullptr;
}

bool Commands::IsAttributeCommand(std::string commandName) const
{
    return commandName.compare("read") == 0 || commandName.compare("write") == 0 || commandName.compare("subscribe") == 0;
}

bool Commands::IsEventCommand(std::string commandName) const
{
    return commandName.compare("read-event") == 0 || commandName.compare("subscribe-event") == 0;
}

bool Commands::IsGlobalCommand(std::string commandName) const
{
    return IsAttributeCommand(commandName) || IsEventCommand(commandName);
}

void Commands::ShowClusters(std::string executable)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s cluster_name command_name [param1 param2 ...]\n", executable.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Clusters:                                                                           |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & cluster : mClusters)
    {
        std::string clusterName(cluster.first);
        std::transform(clusterName.begin(), clusterName.end(), clusterName.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        fprintf(stderr, "  | * %-82s|\n", clusterName.c_str());
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowCluster(std::string executable, std::string clusterName, CommandsVector & commands)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s command_name [param1 param2 ...]\n", executable.c_str(), clusterName.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Commands:                                                                           |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    bool readCommand           = false;
    bool writeCommand          = false;
    bool subscribeCommand      = false;
    bool readEventCommand      = false;
    bool subscribeEventCommand = false;
    for (auto & command : commands)
    {
        bool shouldPrint = true;

        if (IsGlobalCommand(command->GetName()))
        {
            if (strcmp(command->GetName(), "read") == 0 && !readCommand)
            {
                readCommand = true;
            }
            else if (strcmp(command->GetName(), "write") == 0 && !writeCommand)
            {
                writeCommand = true;
            }
            else if (strcmp(command->GetName(), "subscribe") == 0 && !subscribeCommand)
            {
                subscribeCommand = true;
            }
            else if (strcmp(command->GetName(), "read-event") == 0 && !readEventCommand)
            {
                readEventCommand = true;
            }
            else if (strcmp(command->GetName(), "subscribe-event") == 0 && !subscribeEventCommand)
            {
                subscribeEventCommand = true;
            }
            else
            {
                shouldPrint = false;
            }
        }

        if (shouldPrint)
        {
            fprintf(stderr, "  | * %-82s|\n", command->GetName());
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowClusterAttributes(std::string executable, std::string clusterName, std::string commandName,
                                     CommandsVector & commands)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s %s attribute-name [param1 param2 ...]\n", executable.c_str(), clusterName.c_str(),
            commandName.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Attributes:                                                                         |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0)
        {
            fprintf(stderr, "  | * %-82s|\n", command->GetAttribute());
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowClusterEvents(std::string executable, std::string clusterName, std::string commandName,
                                 CommandsVector & commands)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s %s %s event-name [param1 param2 ...]\n", executable.c_str(), clusterName.c_str(), commandName.c_str());
    fprintf(stderr, "\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    fprintf(stderr, "  | Events:                                                                             |\n");
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
    for (auto & command : commands)
    {
        if (commandName.compare(command->GetName()) == 0)
        {
            fprintf(stderr, "  | * %-82s|\n", command->GetEvent());
        }
    }
    fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
}

void Commands::ShowCommand(std::string executable, std::string clusterName, Command * command)
{
    fprintf(stderr, "Usage:\n");

    std::string arguments;
    std::string description;
    arguments += command->GetName();

    size_t argumentsCount = command->GetArgumentsCount();
    for (size_t i = 0; i < argumentsCount; i++)
    {
        std::string arg;
        bool isOptional = command->GetArgumentIsOptional(i);
        if (isOptional)
        {
            arg += "[--";
        }
        arg += command->GetArgumentName(i);
        if (isOptional)
        {
            arg += "]";
        }
        arguments += " ";
        arguments += arg;

        const char * argDescription = command->GetArgumentDescription(i);
        if ((argDescription != nullptr) && (strlen(argDescription) > 0))
        {
            description += "\n";
            description += arg;
            description += ":\n  ";
            description += argDescription;
            description += "\n";
        }
    }
    fprintf(stderr, "  %s %s %s\n", executable.c_str(), clusterName.c_str(), arguments.c_str());

    if (command->GetHelpText())
    {
        fprintf(stderr, "\n%s\n", command->GetHelpText());
    }

    if (description.size() > 0)
    {
        fprintf(stderr, "%s\n", description.c_str());
    }
}
