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

#include <controller/CHIPDeviceController.h>
#include <memory>
#include <vector>

class Command;

class Commands
{
public:
    using ChipDeviceController = ::chip::DeviceController::ChipDeviceController;
    using NodeId               = ::chip::NodeId;

    void Register(std::unique_ptr<Command> command);
    int Run(NodeId localId, NodeId remoteId, int argc, char * argv[]);

private:
    CHIP_ERROR RunCommand(ChipDeviceController & dc, NodeId remoteId, int argc, char * argv[]);
    void ShowUsage(const char * executable);
    std::vector<std::unique_ptr<Command>> commands;
};

#endif // __CHIPTOOL_COMMANDS_H__
