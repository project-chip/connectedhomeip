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

#ifndef __CHIPTOOL_IDENTIFY_H__
#define __CHIPTOOL_IDENTIFY_H__
#include "common/ModelCommand.h"
#include <core/CHIPEncoding.h>

class IdentifyQuery : public ModelCommand
{
public:
    IdentifyQuery() : ModelCommand("identify-query"){};
    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeIdentifyQueryCommand(buffer->Start(), bufferSize, endPointId);
    }

    void HandleClusterResponse(uint16_t clusterId, uint16_t endPointId, uint16_t commandId, uint8_t * message,
                               uint16_t messageLen) const override
    {
        uint16_t identify_time = 0;
        ChipLogProgress(chipTool, "Parsing identify cluster response id %d", commandId);
        CHECK_MESSAGE_LENGTH(messageLen == 2);
        identify_time = chip::Encoding::LittleEndian::Read16(message);
        ChipLogProgress(chipTool, "Identify query response %d", identify_time);

    exit:
        return;
    }
};

class Identify : public ModelCommand
{
public:
    Identify() : ModelCommand("identify") {}
    size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) override
    {
        return encodeIdentifyCommand(buffer->Start(), bufferSize, endPointId, 10);
    }
};
#endif
