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

#pragma once

#ifdef CONFIG_USE_LOCAL_STORAGE
#include <controller/ExamplePersistentStorage.h>
#endif // CONFIG_USE_LOCAL_STORAGE

#include "Command.h"
#include <map>

class Commands
{
public:
    using CommandsVector = ::std::vector<std::unique_ptr<Command>>;

    void RegisterCluster(const char * clusterName, commands_list commandsList)
    {
        Register(clusterName, commandsList, nullptr, false);
    }
    // Command sets represent chip-tool functionality that is not actually
    // XML-defined clusters.  All command sets should have help text explaining
    // what sort of commands one should expect to find in the set.
    void RegisterCommandSet(const char * clusterName, commands_list commandsList, const char * helpText)
    {
        Register(clusterName, commandsList, helpText, true);
    }
    int Run(int argc, char ** argv);
    int RunInteractive(const char * command, const chip::Optional<char *> & storageDirectory = chip::NullOptional);

private:
    struct ClusterData
    {
        CommandsVector commands;
        bool isSynthetic      = false;
        const char * helpText = nullptr;
    };
    // The tuple contains the commands, whether it's a synthetic cluster, and
    // the help text for the cluster (which may be null).
    using ClusterMap = std::map<std::string, ClusterData>;

    CHIP_ERROR RunCommand(int argc, char ** argv, bool interactive = false,
                          const chip::Optional<char *> & interactiveStorageDirectory = chip::NullOptional);

    ClusterMap::iterator GetCluster(std::string clusterName);
    Command * GetCommand(CommandsVector & commands, std::string commandName);
    Command * GetGlobalCommand(CommandsVector & commands, std::string commandName, std::string attributeName);
    bool IsAttributeCommand(std::string commandName) const;
    bool IsEventCommand(std::string commandName) const;
    bool IsGlobalCommand(std::string commandName) const;

    void ShowClusters(std::string executable);
    static void ShowClusterOverview(std::string clusterName, const ClusterData & clusterData);
    void ShowCluster(std::string executable, std::string clusterName, CommandsVector & commands, const char * helpText);
    void ShowClusterAttributes(std::string executable, std::string clusterName, std::string commandName, CommandsVector & commands);
    void ShowClusterEvents(std::string executable, std::string clusterName, std::string commandName, CommandsVector & commands);
    void ShowCommand(std::string executable, std::string clusterName, Command * command);

    bool DecodeArgumentsFromInteractiveMode(const char * command, std::vector<std::string> & args);
    bool DecodeArgumentsFromBase64EncodedJson(const char * encodedData, std::vector<std::string> & args);
    bool DecodeArgumentsFromStringStream(const char * command, std::vector<std::string> & args);

    // helpText may be null, in which case it's not shown.
    static void ShowHelpText(const char * helpText);

    void Register(const char * clusterName, commands_list commandsList, const char * helpText, bool isSynthetic);

    ClusterMap mClusters;
#ifdef CONFIG_USE_LOCAL_STORAGE
    PersistentStorage mStorage;
#endif // CONFIG_USE_LOCAL_STORAGE
};
