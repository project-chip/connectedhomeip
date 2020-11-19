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

#include "Command.h"
#include <map>

#include <controller/CHIPDeviceController_deprecated.h>

class ChipToolPersistentStorageDelegate : public chip::Controller::PersistentStorageDelegate
{
    void SetDelegate(chip::Controller::PersistentStorageResultDelegate * delegate) override {}
    void GetKeyValue(const char * key) override {}
    CHIP_ERROR GetKeyValue(const char * key, char * value, uint16_t & size) override { return CHIP_NO_ERROR; }
    void SetKeyValue(const char * key, const char * value) override {}
    void DeleteKeyValue(const char * key) override {}
};

class Commands
{
public:
    using ChipDeviceController = ::chip::DeviceController::ChipDeviceController;
    using NodeId               = ::chip::NodeId;
    using CommandsVector       = ::std::vector<std::unique_ptr<Command>>;

    void Register(const char * clusterName, commands_list commandsList);
    int Run(NodeId localId, NodeId remoteId, int argc, char ** argv);

private:
    CHIP_ERROR RunCommand(ChipDeviceController & dc, NodeId remoteId, int argc, char ** argv);
    std::map<std::string, CommandsVector>::iterator GetCluster(std::string clusterName);
    Command * GetCommand(CommandsVector & commands, std::string commandName);
    Command * GetGlobalCommand(CommandsVector & commands, std::string commandName, std::string attributeName);
    bool IsGlobalCommand(std::string commandName) const;

    void ShowClusters(std::string executable);
    void ShowCluster(std::string executable, std::string clusterName, CommandsVector & commands);
    void ShowClusterAttributes(std::string executable, std::string clusterName, std::string commandName, CommandsVector & commands);
    void ShowCommand(std::string executable, std::string clusterName, Command * command);

    std::map<std::string, CommandsVector> mClusters;
};
