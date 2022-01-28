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


#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/TestPersistentStorageDelegate.h>

chip::TestPersistentStorageDelegate sDeviceStorage;
chip::Credentials::GroupDataProviderImpl sGroupsProvider(sDeviceStorage);

CHIP_ERROR GroupInit()
{
    ReturnErrorOnFailure(sGroupsProvider.Init());
    chip::Credentials::SetGroupDataProvider(&sGroupsProvider);

    const chip::FabricIndex kFabric1 = 1;
    const chip::GroupId kGroup1 = 0x1234;
    const chip::GroupId kGroup2 = 0x0001;
    const chip::KeysetId kKeySet1 = 0x0101;

    const chip::Credentials::GroupDataProvider::GroupInfo group1(kGroup1, "Group #1");
    ReturnErrorOnFailure(sGroupsProvider.SetGroupInfo(kFabric1, group1));
    ReturnErrorOnFailure(sGroupsProvider.AddEndpoint(kFabric1, group1.group_id, 1));

    const chip::Credentials::GroupDataProvider::GroupInfo group2(kGroup2, "Group #2");
    ReturnErrorOnFailure(sGroupsProvider.SetGroupInfo(kFabric1, group2));
    ReturnErrorOnFailure(sGroupsProvider.AddEndpoint(kFabric1, group2.group_id, 0));

    chip::Credentials::GroupDataProvider::KeySet keyset1(kKeySet1, chip::Credentials::GroupDataProvider::KeySet::SecurityPolicy::kStandard, 3);
    const chip::Credentials::GroupDataProvider::EpochKey epoch_keys[] = {
        { 0xaaaaaaaaaaaaaaaa, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
        { 0xbbbbbbbbbbbbbbbb, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
        { 0xcccccccccccccccc, { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf } },
    };
    memcpy(keyset1.epoch_keys, epoch_keys, sizeof(epoch_keys));
    ReturnErrorOnFailure(sGroupsProvider.SetKeySet(kFabric1, keyset1));

    sGroupsProvider.SetGroupKeyAt(kFabric1, 0, chip::Credentials::GroupDataProvider::GroupKey(kGroup1, kKeySet1));
    sGroupsProvider.SetGroupKeyAt(kFabric1, 1, chip::Credentials::GroupDataProvider::GroupKey(kGroup2, kKeySet1));

    return CHIP_NO_ERROR;
}


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

    err = mStorage.Init();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Storage failure: %s", chip::ErrorStr(err)));

    err = GroupInit();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init Group Data Provider failure: %s", chip::ErrorStr(err)));

    chip::Logging::SetLogFilter(mStorage.GetLoggingLevel());

    err = RunCommand(argc, argv);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Run command failure: %s", chip::ErrorStr(err)));

exit:
    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}

CHIP_ERROR Commands::RunCommand(int argc, char ** argv)
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

    return command->Run();
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
            if (strcmp(command->GetName(), "read") == 0 && readCommand == false)
            {
                readCommand = true;
            }
            else if (strcmp(command->GetName(), "write") == 0 && writeCommand == false)
            {
                writeCommand = true;
            }
            else if (strcmp(command->GetName(), "subscribe") == 0 && subscribeCommand == false)
            {
                subscribeCommand = true;
            }
            else if (strcmp(command->GetName(), "read-event") == 0 && readEventCommand == false)
            {
                readEventCommand = true;
            }
            else if (strcmp(command->GetName(), "subscribe-event") == 0 && subscribeEventCommand == false)
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

    std::string arguments = "";
    arguments += command->GetName();

    size_t argumentsCount = command->GetArgumentsCount();
    for (size_t i = 0; i < argumentsCount; i++)
    {
        arguments += " ";
        bool isOptional = command->GetArgumentIsOptional(i);
        if (isOptional)
        {
            arguments += "[--";
        }
        arguments += command->GetArgumentName(i);
        if (isOptional)
        {
            arguments += "]";
        }
    }
    fprintf(stderr, "  %s %s %s\n", executable.c_str(), clusterName.c_str(), arguments.c_str());
}
