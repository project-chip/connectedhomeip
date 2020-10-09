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

#include "../common/EchoCommand.h"

class Echo : public EchoCommand
{
public:
    Echo() : EchoCommand("echo", NetworkType::UDP) {}
};

class EchoBle : public EchoCommand
{
public:
    EchoBle() : EchoCommand("echo-ble", NetworkType::BLE) {}
};

void registerCommandsEcho(Commands & commands)
{
    const char * clusterName = "";

    commands_list clusterCommands = {
        make_unique<Echo>(),
        make_unique<EchoBle>(),
    };

    commands.Register(clusterName, clusterCommands);
}

