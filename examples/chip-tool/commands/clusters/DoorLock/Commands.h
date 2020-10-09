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

#include "../../common/ModelCommand.h"

class LockDoor : public ModelCommand
{
public:
    LockDoor(const uint16_t clusterId) : ModelCommand("lock-door", clusterId) { AddArgument("PIN", &mPIN); }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeLockDoorCommand(buffer->Start(), bufferSize, endPointId, mPIN);
    }

private:
    char * mPIN;
};

class UnlockDoor : public ModelCommand
{
public:
    UnlockDoor(const uint16_t clusterId) : ModelCommand("unlock-door", clusterId) { AddArgument("PIN", &mPIN); }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeUnlockDoorCommand(buffer->Start(), bufferSize, endPointId, mPIN);
    }

private:
    char * mPIN;
};

void registerClusterDoorLock(Commands & commands)
{
    const char * clusterName = "DoorLock";
    const uint16_t clusterId = 0x0101;

    commands_list clusterCommands = {
        make_unique<LockDoor>(clusterId),
        make_unique<UnlockDoor>(clusterId),
    };

    commands.Register(clusterName, clusterCommands);
}
