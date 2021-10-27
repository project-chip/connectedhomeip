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

#include "../../config/PersistentStorage.h"
#include "Command.h"
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <map>

class Commands
{
public:
    using NodeId         = ::chip::NodeId;
    using CommandsVector = ::std::vector<std::unique_ptr<Command>>;

    void Register(const char * clusterName, commands_list commandsList);
    int Run(int argc, char ** argv);

private:
    // *ranCommand will be set to the command we ran if we get as far as running
    // it.  If it's not null, we need to call Shutdown() on the command after we
    // shut down the event loop.
    CHIP_ERROR RunCommand(NodeId localId, NodeId remoteId, int argc, char ** argv, Command ** ranCommand);
    static void RunQueuedCommand(intptr_t commandArg);
    std::map<std::string, CommandsVector>::iterator GetCluster(std::string clusterName);
    Command * GetCommand(CommandsVector & commands, std::string commandName);
    Command * GetGlobalCommand(CommandsVector & commands, std::string commandName, std::string attributeName);
    bool IsGlobalCommand(std::string commandName) const;

    void ShowClusters(std::string executable);
    void ShowCluster(std::string executable, std::string clusterName, CommandsVector & commands);
    void ShowClusterAttributes(std::string executable, std::string clusterName, std::string commandName, CommandsVector & commands);
    void ShowCommand(std::string executable, std::string clusterName, Command * command);

    std::map<std::string, CommandsVector> mClusters;
    chip::Controller::DeviceCommissioner mController;
    chip::Controller::ExampleOperationalCredentialsIssuer mOpCredsIssuer;
    PersistentStorage mStorage;
};
