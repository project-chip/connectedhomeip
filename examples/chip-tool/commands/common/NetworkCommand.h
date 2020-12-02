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
#include "Command.h"

class NetworkCommand : public Command, public chip::Controller::DeviceStatusDelegate
{
public:
    NetworkCommand(const char * commandName) : Command(commandName) {}

    virtual uint16_t Encode(PacketBufferHandle & buffer, uint16_t bufferSize) = 0;
    virtual bool Decode(PacketBufferHandle & buffer) const                    = 0;

    /////////// Command Interface /////////
    CHIP_ERROR Run(PersistentStorage & storage, NodeId localId, NodeId remoteId) override;

    /////////// DeviceStatusDelegate Interface /////////
    void OnMessage(PacketBufferHandle buffer) override;
    void OnStatusChange(void) override;

private:
    CHIP_ERROR RunInternal(NodeId remoteId);
    CHIP_ERROR RunCommandInternal(ChipDevice * device);

    void PrintBuffer(PacketBufferHandle & buffer) const;

    ChipDeviceCommissioner mCommissioner;
};
