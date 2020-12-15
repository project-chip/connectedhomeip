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

#include "../../config/PersistentStorage.h"
#include "../common/Command.h"
#include <app/chip-zcl-zpro-codec.h>
#include <core/CHIPEncoding.h>

// Limits on endpoint values.  Could be wrong, if we start using endpoint 0 for
// something.
#define CHIP_ZCL_ENDPOINT_MIN 0x01
#define CHIP_ZCL_ENDPOINT_MAX 0xF0

class ModelCommand : public Command, public chip::Controller::DeviceStatusDelegate
{
public:
    ModelCommand(const char * commandName, uint16_t clusterId, uint8_t commandId) :
        Command(commandName), mClusterId(clusterId), mCommandId(commandId)
    {}

    void AddArguments() { AddArgument("endpoint-id", CHIP_ZCL_ENDPOINT_MIN, CHIP_ZCL_ENDPOINT_MAX, &mEndPointId); }

    /////////// Command Interface /////////
    CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) override;

    /////////// DeviceStatusDelegate Interface /////////
    void OnMessage(PacketBufferHandle buffer) override;
    void OnStatusChange(void) override;

    virtual PacketBufferHandle EncodeCommand(uint8_t endPointId) = 0;
    virtual bool HandleGlobalResponse(uint8_t commandId, uint8_t * message, uint16_t messageLen) const { return false; }
    virtual bool HandleSpecificResponse(uint8_t commandId, uint8_t * message, uint16_t messageLen) const { return false; }

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR RunCommandInternal(ChipDevice * device);

    void PrintBuffer(const PacketBufferHandle & buffer) const;

    ChipDeviceCommissioner mCommissioner;
    const uint16_t mClusterId;
    const uint8_t mCommandId;
    uint8_t mEndPointId;
};
