/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef CONFIG_USE_LOCAL_STORAGE
#include "../../config/PersistentStorage.h"
#endif // CONFIG_USE_LOCAL_STORAGE

#include "Command.h"
#include <map>

class Commands
{
public:
    using CommandsVector = ::std::vector<std::unique_ptr<Command>>;

    void Register(const char * clusterName, commands_list commandsList);
    int Run(int argc, char ** argv);
    int RunInteractive(int argc, char ** argv);

private:
    CHIP_ERROR RunCommand(int argc, char ** argv, bool interactive = false);

    std::map<std::string, CommandsVector>::iterator GetCluster(std::string clusterName);
    Command * GetCommand(CommandsVector & commands, std::string commandName);
    Command * GetGlobalCommand(CommandsVector & commands, std::string commandName, std::string attributeName);
    bool IsAttributeCommand(std::string commandName) const;
    bool IsEventCommand(std::string commandName) const;
    bool IsGlobalCommand(std::string commandName) const;

    void ShowClusters(std::string executable);
    void ShowCluster(std::string executable, std::string clusterName, CommandsVector & commands);
    void ShowClusterAttributes(std::string executable, std::string clusterName, std::string commandName, CommandsVector & commands);
    void ShowClusterEvents(std::string executable, std::string clusterName, std::string commandName, CommandsVector & commands);
    void ShowCommand(std::string executable, std::string clusterName, Command * command);

    std::map<std::string, CommandsVector> mClusters;
#ifdef CONFIG_USE_LOCAL_STORAGE
    PersistentStorage mStorage;
#endif // CONFIG_USE_LOCAL_STORAGE
};
