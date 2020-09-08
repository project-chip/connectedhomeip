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

#include <chrono>
#include <sstream>
#include <string>

using namespace ::chip;
using namespace ::chip::DeviceController;

constexpr std::chrono::seconds kWaitingForResponseTimeout(1);

void ModelCommand::OnConnect(ChipDeviceController * dc)
{
    mWaitingForResponse = true;
    SendCommand(dc);
}

void ModelCommand::OnError(ChipDeviceController * dc, CHIP_ERROR err)
{
    mWaitingForResponse = false;
}

void ModelCommand::OnMessage(ChipDeviceController * dc, System::PacketBuffer * buffer)
{
    mWaitingForResponse = false;
    ReceiveCommandResponse(dc, buffer);
}

CHIP_ERROR ModelCommand::Run(ChipDeviceController * dc, Optional<NodeId> remoteId)
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
    auto * buffer                  = System::PacketBuffer::NewWithAvailableSize(bufferSize);
    uint16_t dataLength            = EncodeCommand(buffer, bufferSize, mEndPointId);
    buffer->SetDataLength(dataLength);
    ChipLogProgress(chipTool, "Encoded data of length %d", dataLength);

#ifdef DEBUG
    const size_t data_len = buffer->DataLength();

    fprintf(stderr, "SENDING: %zu ", data_len);
    for (size_t i = 0; i < data_len; ++i)
    {
        fprintf(stderr, "%d ", buffer->Start()[i]);
    }
    fprintf(stderr, "\n");
#endif

    dc->SendMessage(NULL, buffer);

    // FIXME: waitingForResponse is being written on other threads, presumably.
    // We probably need some more synchronization here.
    auto start = std::chrono::system_clock::now();
    while (mWaitingForResponse &&
           std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - start) < kWaitingForResponseTimeout)
    {
        // Just poll for the response.
        sleep(1);
    }

    if (mWaitingForResponse)
    {
        fprintf(stderr, "No response from device.");
    }
}

void ModelCommand::ReceiveCommandResponse(ChipDeviceController * dc, System::PacketBuffer * buffer)
{
    // A data model message has a first byte whose value is always one of  0x00,
    // 0x01, 0x02, 0x03.
    if (buffer->DataLength() == 0 || buffer->Start()[0] >= 0x04)
    {
        return;
    }

    EmberApsFrame frame;
    if (extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) == 0)
    {
        printf("APS frame processing failure!\n");
        System::PacketBuffer::Free(buffer);
        return;
    }

    printf("APS frame processing success!\n");
    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);

    VerifyOrExit(messageLen >= 3, printf("Unexpected response length: %d\n", messageLen));
    // Bit 3 of the frame control byte set means direction is server to client.
    // We expect no other bits to be set.
    VerifyOrExit(message[0] == 8, printf("Unexpected frame control byte: 0x%02x\n", message[0]));
    VerifyOrExit(message[1] == 1, printf("Unexpected sequence number: %d\n", message[1]));

    // message[2] is the command id.
    switch (message[2])
    {
    case 0x0b: {
        // Default Response command.  Remaining bytes are the command id of the
        // command that's being responded to and a status code.
        VerifyOrExit(messageLen == 5, printf("Unexpected response length: %d\n", messageLen));
        printf("Got default response to command '0x%02x' for cluster '0x%02x'.  Status is '0x%02x'.\n", message[3], frame.clusterId,
               message[4]);
        break;
    }
    case 0x01: {
        // Read Attributes Response command.  Remaining bytes are a list of
        // (attr id, 0, attr type, attr value) or (attr id, failure status)
        // tuples.
        //
        // But for now we only support one attribute value, and that value is a
        // boolean.
        VerifyOrExit(messageLen >= 6, printf("Unexpected response length for Read Attributes command: %d\n", messageLen));
        uint16_t attr_id;
        memcpy(&attr_id, message + 3, sizeof(attr_id));
        if (message[5] == 0)
        {
            // FIXME: Should we have a mapping of type ids to types, based on
            // table 2.6.2.2 in Rev 8 of the ZCL spec?  0x10 is "Boolean".
            VerifyOrExit(messageLen == 8,
                         printf("Unexpected response length for successful Read Attributes command: %d\n", messageLen));
            printf("Read attribute '0x%04x' for cluster '0x%02x'.  Type is '0x%02x', value is '0x%02x'.\n", attr_id,
                   frame.clusterId, message[6], message[7]);
        }
        else
        {
            VerifyOrExit(messageLen == 6,
                         printf("Unexpected response length for failed Read Attributes command: %d\n", messageLen));
            printf("Reading attribute '0x%04x' for cluster '0x%02x' failed with status '0x%02x'.\n", attr_id, frame.clusterId,
                   message[5]);
        }
        break;
    }
    default: {
        printf("Unexpected command '0x%02x'.\n", message[2]);
        break;
    }
    }

exit:
    return;
}
