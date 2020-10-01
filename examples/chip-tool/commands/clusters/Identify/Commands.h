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

#ifndef __CHIPTOOL_IDENTIFY_COMMANDS_H__
#define __CHIPTOOL_IDENTIFY_COMMANDS_H__

#include "../../common/ModelCommand.h"

class Identify : public ModelCommand
{
public:
    Identify(const uint16_t clusterId) : ModelCommand("identify", clusterId) {}
    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint8_t endpointId, uint16_t clusterId) override
    {
        uint16_t duration = 10;
        return encodeCommand(buffer->Start(), bufferSize, endpointId, clusterId, 0x00, duration);
    }
};

class IdentifyQuery : public ModelCommand
{
public:
    IdentifyQuery(const uint16_t clusterId) : ModelCommand("identify-query", clusterId) {}
    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint8_t endpointId, uint16_t clusterId) override
    {
        return encodeCommand(buffer->Start(), bufferSize, endpointId, clusterId, 0x01);
    }

    bool HandleClusterResponse(uint8_t * message, uint16_t messageLen) const override
    {
        uint16_t identify_time = 0;

        CHECK_MESSAGE_LENGTH(messageLen == 2);

        identify_time = chip::Encoding::LittleEndian::Read16(message);

        ChipLogProgress(chipTool, "Identify query response %d", identify_time);

    exit:
        return true;
    }
};

void registerClusterIdentify(Commands & commands)
{
    const uint16_t clusterId = 0x0003;

    commands.Register(make_unique<Identify>(clusterId));
    commands.Register(make_unique<IdentifyQuery>(clusterId));
}

#endif // __CHIPTOOL_IDENTIFY_COMMANDS_H__
