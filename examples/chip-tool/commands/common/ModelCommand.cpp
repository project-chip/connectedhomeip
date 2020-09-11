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

#include "ModelCommand.h"

#include <core/CHIPEncoding.h>

#include <atomic>
#include <chrono>
#include <sstream>
#include <string>

using namespace ::chip;
using namespace ::chip::DeviceController;

#define CHECK_MESSAGE_LENGTH(rv)                                                                                                   \
    VerifyOrExit(rv, ChipLogError(chipTool, "%s: Unexpected response length: %d", __FUNCTION__, messageLen));

constexpr std::chrono::seconds kWaitingForResponseTimeout(10);

void ModelCommand::OnConnect(ChipDeviceController * dc)
{
    SendCommand(dc);
    UpdateWaitForResponse(true);
    WaitForResponse();
}

void ModelCommand::OnError(ChipDeviceController * dc, CHIP_ERROR err)
{
    UpdateWaitForResponse(false);
}

void ModelCommand::OnMessage(ChipDeviceController * dc, PacketBuffer * buffer)
{
    UpdateWaitForResponse(false);
    ReceiveCommandResponse(dc, buffer);
}

CHIP_ERROR ModelCommand::Run(ChipDeviceController * dc, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = NetworkCommand::Run(dc, remoteId);
    SuccessOrExit(err);

    err = dc->ServiceEventSignal();
    SuccessOrExit(err);

exit:
    return err;
}

void ModelCommand::SendCommand(ChipDeviceController * dc)
{
    // Make sure our buffer is big enough, but this will need a better setup!
    static const size_t bufferSize = 1024;
    auto * buffer                  = PacketBuffer::NewWithAvailableSize(bufferSize);
    uint16_t dataLength            = EncodeCommand(buffer, bufferSize, mEndPointId);
    buffer->SetDataLength(dataLength);
    ChipLogProgress(chipTool, "Encoded data of length %d", dataLength);

#ifdef DEBUG
    PrintBuffer(buffer);
#endif

    dc->SendMessage(NULL, buffer);
}

void ModelCommand::ReceiveCommandResponse(ChipDeviceController * dc, PacketBuffer * buffer) const
{
    EmberApsFrame frame;
    uint8_t * message;
    uint16_t messageLen;
    uint8_t frameControl;
    uint8_t sequenceNumber;
    uint8_t commandId;

    // Bit 3 of the frame control byte set means direction is server to client.
    if (extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) == 0)
    {
        ChipLogError(chipTool, "APS frame processing failure!");
        PacketBuffer::Free(buffer);
        ExitNow();
    }
    ChipLogProgress(chipTool, "APS frame processing success!");

    // Bit 3 of the frame control byte set means direction is server to client.
    // We expect no other bits to be set.
    messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    CHECK_MESSAGE_LENGTH(messageLen >= 3);

    frameControl   = chip::Encoding::Read8(message);
    sequenceNumber = chip::Encoding::Read8(message);
    commandId      = chip::Encoding::Read8(message);
    messageLen -= 3;

    VerifyOrExit(frameControl == 8, ChipLogError(chipTool, "Unexpected frame control byte: 0x%02x", frameControl));
    VerifyOrExit(sequenceNumber == 1, ChipLogError(chipTool, "Unexpected sequence number: %d", sequenceNumber));

    switch (commandId)
    {
    case 0x0b:
        ParseDefaultResponseCommand(frame.clusterId, message, messageLen);
        break;
    case 0x01:
        ParseReadAttributeResponseCommand(frame.clusterId, message, messageLen);
        break;
    default:
        ChipLogError(chipTool, "Unexpected command '0x%02x'", commandId);
        break;
    }

exit:
    return;
}

void ModelCommand::ParseDefaultResponseCommand(uint16_t clusterId, uint8_t * message, uint16_t messageLen) const
{
    uint8_t commandId;
    uint8_t status;

    CHECK_MESSAGE_LENGTH(messageLen == 2);

    commandId = chip::Encoding::Read8(message);
    status    = chip::Encoding::Read8(message);

    ChipLogProgress(chipTool, "Got default response to command '0x%02x' for cluster '0x%02x'.  Status is '0x%02x'", commandId,
                    clusterId, status);

exit:
    return;
}

void ModelCommand::ParseReadAttributeResponseCommand(uint16_t clusterId, uint8_t * message, uint16_t messageLen) const
{
    uint16_t attrId;
    uint8_t status;

    // Remaining bytes are a list of (attr id, 0, attr type, attr value) or (attr id, failure status)
    // tuples.
    //
    // But for now we only support one attribute value, and that value is a boolean.
    CHECK_MESSAGE_LENGTH(messageLen >= 3);

    attrId = chip::Encoding::LittleEndian::Read16(message);
    status = chip::Encoding::Read8(message);
    messageLen -= 3;

    if (status == 0)
    {
        ParseReadAttributeResponseCommandSuccess(clusterId, attrId, message, messageLen);
    }
    else
    {
        ParseReadAttributeResponseCommandFailure(clusterId, attrId, status, messageLen);
    }

exit:
    return;
}

void ModelCommand::ParseReadAttributeResponseCommandSuccess(uint16_t clusterId, uint16_t attrId, uint8_t * message,
                                                            uint16_t messageLen) const
{
    uint8_t type;
    uint8_t value;

    // FIXME: Should we have a mapping of type ids to types, based on
    // table 2.6.2.2 in Rev 8 of the ZCL spec?  0x10 is "Boolean".
    CHECK_MESSAGE_LENGTH(messageLen == 2);

    type  = chip::Encoding::Read8(message);
    value = chip::Encoding::Read8(message);

    ChipLogProgress(chipTool, "Read attribute '0x%04x' for cluster '0x%02x'.  Type is '0x%02x', value is '0x%02x'", attrId,
                    clusterId, type, value);

exit:
    return;
}

void ModelCommand::ParseReadAttributeResponseCommandFailure(uint16_t clusterId, uint16_t attrId, uint8_t status,
                                                            uint16_t messageLen) const
{
    CHECK_MESSAGE_LENGTH(messageLen == 0);
    ChipLogProgress(chipTool, "Reading attribute '0x%04x' for cluster '0x%02x' failed with status '0x%02x'", attrId, clusterId,
                    status);

exit:
    return;
}

void ModelCommand::UpdateWaitForResponse(bool value)
{
    {
        std::lock_guard<std::mutex> lk(cvWaitingForResponseMutex);
        mWaitingForResponse = value;
    }
    cvWaitingForResponse.notify_all();
}

void ModelCommand::WaitForResponse()
{
    std::unique_lock<std::mutex> lk(cvWaitingForResponseMutex);
    auto waitingUntil = std::chrono::system_clock::now() + kWaitingForResponseTimeout;
    if (!cvWaitingForResponse.wait_until(lk, waitingUntil, [this]() { return !this->mWaitingForResponse; }))
    {
        ChipLogError(chipTool, "No response from device");
    }
}

void ModelCommand::PrintBuffer(PacketBuffer * buffer) const
{
    const size_t data_len = buffer->DataLength();

    fprintf(stderr, "SENDING: %zu ", data_len);
    for (size_t i = 0; i < data_len; ++i)
    {
        fprintf(stderr, "%d ", buffer->Start()[i]);
    }
    fprintf(stderr, "\n");
}
