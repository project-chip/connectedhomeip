/*
 *   Copyright (c) 2022 Project CHIP Authors
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

#include "commands/common/Commands.h"
#include "commands/dcl/DCLCommands.h"

void registerCommandsDCL(Commands & commands)
{
    const char * clusterName      = "DCL";
    commands_list clusterCommands = {
        make_unique<DCLModelCommand>(),              //
        make_unique<DCLModelByPayloadCommand>(),     //
        make_unique<DCLTCCommand>(),                 //
        make_unique<DCLTCByPayloadCommand>(),        //
        make_unique<DCLTCDisplayCommand>(),          //
        make_unique<DCLTCDisplayByPayloadCommand>(), //
    };

    commands.RegisterCommandSet(clusterName, clusterCommands, "Commands to interact with the DCL.");
}
