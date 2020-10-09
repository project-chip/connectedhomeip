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

#ifndef __CHIPTOOL_BARRIERCONTROL_COMMANDS_H__
#define __CHIPTOOL_BARRIERCONTROL_COMMANDS_H__

#include "../../common/ModelCommand.h"

class MoveToPercent : public ModelCommand
{
public:
    MoveToPercent(const uint16_t clusterId) : ModelCommand("move-to-percent", clusterId)
    {
        AddArgument("percent", 0, UINT8_MAX, &mPercent);
    }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeMoveToPercentCommand(buffer->Start(), bufferSize, endPointId, mPercent);
    }

private:
    uint8_t mPercent;
};

class StopMoveToPercent : public ModelCommand
{
public:
    StopMoveToPercent(const uint16_t clusterId) : ModelCommand("stop-move-to-percent", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeStopMoveToPercentCommand(buffer->Start(), bufferSize, endPointId);
    }
};

void registerClusterBarrierControl(Commands & commands)
{
    const char * clusterName = "BarrierControl";
    const uint16_t clusterId = 0x0103;

    commands_list clusterCommands = {
        make_unique<MoveToPercent>(clusterId),
        make_unique<StopMoveToPercent>(clusterId),
    };

    commands.Register(clusterName, clusterCommands);
}

#endif // __CHIPTOOL_BARRIERCONTROL_COMMANDS_H__
