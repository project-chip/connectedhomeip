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

#ifndef __CHIPTOOL_ONOFF_COMMANDS_H__
#define __CHIPTOOL_ONOFF_COMMANDS_H__

#include "../../common/ModelCommand.h"

class Off : public ModelCommand
{
public:
    Off(const uint16_t clusterId) : ModelCommand("off", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeOffCommand(buffer->Start(), bufferSize, endPointId);
    }
};

class On : public ModelCommand
{
public:
    On(const uint16_t clusterId) : ModelCommand("on", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeOnCommand(buffer->Start(), bufferSize, endPointId);
    }
};

class ReadOnOff : public ModelCommand
{
public:
    ReadOnOff(const uint16_t clusterId) : ModelCommand("read", clusterId) { AddArgument("attr-name", "onoff"); }

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeReadOnOffCommand(buffer->Start(), bufferSize, endPointId);
    }
};

class Toggle : public ModelCommand
{
public:
    Toggle(const uint16_t clusterId) : ModelCommand("toggle", clusterId) {}

    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeToggleCommand(buffer->Start(), bufferSize, endPointId);
    }
};

void registerClusterOnOff(Commands & commands)
{
    const char * clusterName = "OnOff";
    const uint16_t clusterId = 0x0006;

    commands_list clusterCommands = {
        make_unique<Off>(clusterId),
        make_unique<On>(clusterId),
        make_unique<ReadOnOff>(clusterId),
        make_unique<Toggle>(clusterId),
    };

    commands.Register(clusterName, clusterCommands);
}

#endif // __CHIPTOOL_ONOFF_COMMANDS_H__
