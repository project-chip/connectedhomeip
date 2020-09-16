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

#ifndef __CHIPTOOL_MODELCOMMAND_H__
#define __CHIPTOOL_MODELCOMMAND_H__

#include "NetworkCommand.h"

#include <condition_variable>
#include <mutex>

#include <app/chip-zcl-zpro-codec.h>

// Limits on endpoint values.  Could be wrong, if we start using endpoint 0 for
// something.
#define CHIP_ZCL_ENDPOINT_MIN 0x01
#define CHIP_ZCL_ENDPOINT_MAX 0xF0

class ModelCommand : public NetworkCommand
{
public:
    ModelCommand(const char * commandName) : NetworkCommand(commandName, NetworkType::UDP)
    {
        AddArgument("endpoint-id", CHIP_ZCL_ENDPOINT_MIN, CHIP_ZCL_ENDPOINT_MAX, &mEndPointId);
    }

    /////////// Command Interface /////////
    CHIP_ERROR Run(ChipDeviceController * dc, NodeId remoteId) override;

    /////////// IPCommand Interface /////////
    void OnConnect(ChipDeviceController * dc) override;
    void OnError(ChipDeviceController * dc, CHIP_ERROR err) override;
    void OnMessage(ChipDeviceController * dc, PacketBuffer * buffer) override;

    virtual size_t EncodeCommand(PacketBuffer * buffer, size_t bufferSize, uint16_t endPointId) = 0;

private:
    void SendCommand(ChipDeviceController * dc);
    void ReceiveCommandResponse(ChipDeviceController * dc, PacketBuffer * buffer) const;

    void ParseDefaultResponseCommand(uint16_t clusterId, uint8_t * message, uint16_t messageLen) const;
    void ParseReadAttributeResponseCommand(uint16_t clusterId, uint8_t * message, uint16_t messageLen) const;
    void ParseReadAttributeResponseCommandSuccess(uint16_t clusterId, uint16_t attrId, uint8_t * message,
                                                  uint16_t messageLen) const;
    void ParseReadAttributeResponseCommandFailure(uint16_t clusterId, uint16_t attrId, uint8_t status, uint16_t messageLen) const;

    void UpdateWaitForResponse(bool value);
    void WaitForResponse(void);
    void PrintBuffer(PacketBuffer * buffer) const;

    std::condition_variable cvWaitingForResponse;
    std::mutex cvWaitingForResponseMutex;
    bool mWaitingForResponse{ false };
    uint32_t mEndPointId;
};

#endif // __CHIPTOOL_MODELCOMMAND_H__
