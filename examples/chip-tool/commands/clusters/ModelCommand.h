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

#include "../common/NetworkCommand.h"

#include <app/chip-zcl-zpro-codec.h>
#include <core/CHIPEncoding.h>

// Limits on endpoint values.  Could be wrong, if we start using endpoint 0 for
// something.
#define CHIP_ZCL_ENDPOINT_MIN 0x01
#define CHIP_ZCL_ENDPOINT_MAX 0xF0

class ModelCommand : public NetworkCommand
{
public:
    ModelCommand(const char * commandName, uint16_t clusterId, uint8_t commandId) :
        NetworkCommand(commandName), mClusterId(clusterId), mCommandId(commandId)
    {}

    void AddArguments() { AddArgument("endpoint-id", CHIP_ZCL_ENDPOINT_MIN, CHIP_ZCL_ENDPOINT_MAX, &mEndPointId); }

    /////////// NetworkCommand Interface /////////
    uint16_t Encode(PacketBufferHandle & buffer, uint16_t bufferSize) override;
    bool Decode(PacketBufferHandle & buffer) const override;

    virtual uint16_t EncodeCommand(const PacketBufferHandle & buffer, uint16_t bufferSize, uint8_t endPointId) = 0;
    virtual bool HandleGlobalResponse(uint8_t commandId, uint8_t * message, uint16_t messageLen) const { return false; }
    virtual bool HandleSpecificResponse(uint8_t commandId, uint8_t * message, uint16_t messageLen) const { return false; }

private:
    const uint16_t mClusterId;
    const uint8_t mCommandId;
    uint8_t mEndPointId;
};
