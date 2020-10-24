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

#include "ManualCodeCommand.h"
#include "QRCodeCommand.h"

class ManualCodeParse : public ManualCodeCommand
{
public:
    ManualCodeParse() : ManualCodeCommand("manual") {}
};

class QRCodeParse : public QRCodeCommand
{
public:
    QRCodeParse() : QRCodeCommand("qr") {}
};

void registerCommandsParse(Commands & commands)
{
    const char * clusterName = "Parse";

    commands_list clusterCommands = {
        make_unique<ManualCodeParse>(),
        make_unique<QRCodeParse>(),
    };

    commands.Register(clusterName, clusterCommands);
}
