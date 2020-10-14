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

#include <atomic>
#include <chrono>
#include <sstream>
#include <string>

using namespace ::chip;
using namespace ::chip::DeviceController;

namespace {
constexpr uint8_t kZCLGlobalCmdFrameControlHeader  = 8;
constexpr uint8_t kZCLClusterCmdFrameControlHeader = 9;
} // namespace

constexpr std::chrono::seconds kWaitingForResponseTimeout(10);

namespace {
bool isValidFrame(uint8_t frameControl)
{
    // Bit 3 of the frame control byte set means direction is server to client.
    return (frameControl == kZCLGlobalCmdFrameControlHeader || frameControl == kZCLClusterCmdFrameControlHeader);
}
} // namespace

void ModelCommand::OnConnect(ChipDeviceController * dc)
{
    if (SendCommand(dc))
    {
        UpdateWaitForResponse(true);
        WaitForResponse();
    }
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

bool ModelCommand::SendCommand(ChipDeviceController * dc)
{
    // Make sure our buffer is big enough, but this will need a better setup!
    static const uint16_t bufferSize = 1024;
    auto * buffer                    = PacketBuffer::NewWithAvailableSize(bufferSize);

    if (buffer == nullptr)
    {
        ChipLogError(chipTool, "Failed to allocate memory for packet data.");
        return false;
    }

    ChipLogProgress(chipTool, "Endpoint id: '0x%02x', Cluster id: '0x%04x', Command id: '0x%02x'", mEndPointId, mClusterId,
                    mCommandId);

    uint16_t dataLength = EncodeCommand(buffer, bufferSize, mEndPointId);
    if (dataLength == 0)
    {
        PacketBuffer::Free(buffer);
        ChipLogError(chipTool, "Error while encoding data for command: %s", GetName());
        return false;
    }

    buffer->SetDataLength(dataLength);
    ChipLogDetail(chipTool, "Encoded data of length %d", dataLength);

#ifdef DEBUG
    PrintBuffer(buffer);
#endif

    dc->SendMessage(NULL, buffer);
    return true;
}

void ModelCommand::ReceiveCommandResponse(ChipDeviceController * dc, PacketBuffer * buffer) const
{
    EmberApsFrame frame;
    uint8_t * message;
    uint16_t messageLen;
    uint8_t frameControl;
    uint8_t sequenceNumber;
    uint8_t commandId;
    bool isGlobalCommand = false;

    if (extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) == 0)
    {
        ChipLogError(chipTool, "APS frame processing failure!");
        PacketBuffer::Free(buffer);
        ExitNow();
    }
    ChipLogDetail(chipTool, "APS frame processing success!");

    messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    CHECK_MESSAGE_LENGTH(messageLen >= 3);

    frameControl   = chip::Encoding::Read8(message);
    sequenceNumber = chip::Encoding::Read8(message);
    commandId      = chip::Encoding::Read8(message);
    messageLen     = static_cast<uint16_t>(messageLen - 3);

    VerifyOrExit(isValidFrame(frameControl), ChipLogError(chipTool, "Unexpected frame control byte: 0x%02x", frameControl));
    VerifyOrExit(sequenceNumber == 1, ChipLogError(chipTool, "Unexpected sequence number: %d", sequenceNumber));
    VerifyOrExit(mEndPointId == frame.sourceEndpoint,
                 ChipLogError(chipTool, "Unexpected endpoint id '0x%02x'", frame.sourceEndpoint));
    VerifyOrExit(mClusterId == frame.clusterId, ChipLogError(chipTool, "Unexpected cluster id '0x%04x'", frame.clusterId));

    isGlobalCommand = (frameControl == kZCLGlobalCmdFrameControlHeader);
    isGlobalCommand ? HandleGlobalResponse(commandId, message, messageLen) : HandleSpecificResponse(commandId, message, messageLen);

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
