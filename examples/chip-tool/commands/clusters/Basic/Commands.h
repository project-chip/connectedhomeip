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

#ifndef __CHIPTOOL_BASIC_COMMANDS_H__
#define __CHIPTOOL_BASIC_COMMANDS_H__

#include "../../common/ModelCommand.h"

class ResetToFactory : public ModelCommand
{
public:
    ResetToFactory(const uint16_t clusterId) : ModelCommand("reset-to-factory", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeResetToFactoryCommand(buffer->Start(), bufferSize, endPointId);
    }
};

void registerClusterBasic(Commands & commands)
{
    const char * clusterName = "Basic";
    const uint16_t clusterId = 0x0000;

    commands_list clusterCommands = {
        make_unique<ResetToFactory>(clusterId),
    };

    commands.Register(clusterName, clusterCommands);
}

#endif // __CHIPTOOL_BASIC_COMMANDS_H__
