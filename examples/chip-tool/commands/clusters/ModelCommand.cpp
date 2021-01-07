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

using namespace ::chip;

namespace {
constexpr uint16_t kWaitDurationInSeconds                    = 10;
constexpr uint8_t kZCLGlobalCmdFrameControlHeader            = 8;
constexpr uint8_t kZCLClusterCmdFrameControlHeader           = 9;
constexpr uint8_t kZCLGlobalMfgSpecificCmdFrameControlHeader = 12;

bool isValidFrame(uint8_t frameControl)
{
    // Bit 3 of the frame control byte set means direction is server to client.
    return (frameControl == kZCLGlobalCmdFrameControlHeader || frameControl == kZCLClusterCmdFrameControlHeader ||
            kZCLGlobalMfgSpecificCmdFrameControlHeader);
}

bool isGlobalCommand(uint8_t frameControl)
{
    return (frameControl == kZCLGlobalCmdFrameControlHeader || frameControl == kZCLGlobalMfgSpecificCmdFrameControlHeader);
}
} // namespace

CHIP_ERROR ModelCommand::Run(PersistentStorage & storage, NodeId localId, NodeId remoteId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mCommissioner.SetUdpListenPort(storage.GetListenPort());
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mCommissioner.Init(localId, &storage);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Commissioner: %s", chip::ErrorStr(err)));

    err = mCommissioner.ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Controller, "Init failure! Run Loop: %s", chip::ErrorStr(err)));

    err = RunInternal(remoteId);
    SuccessOrExit(err);

    VerifyOrExit(GetCommandExitStatus(), err = CHIP_ERROR_INTERNAL);

exit:
    mCommissioner.ServiceEventSignal();
    mCommissioner.Shutdown();
    return err;
}

CHIP_ERROR ModelCommand::RunInternal(NodeId remoteId)
{
    ChipDevice * device;
    CHIP_ERROR err = mCommissioner.GetDevice(remoteId, &device);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Could not find a paired device. Are you sure it has been paired ?"));

    device->SetDelegate(this);

    err = RunCommandInternal(device);
    SuccessOrExit(err);

    UpdateWaitForResponse(true);
    WaitForResponse(kWaitDurationInSeconds);

exit:
    return err;
}

CHIP_ERROR ModelCommand::RunCommandInternal(ChipDevice * device)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(chipTool, "Endpoint id: '0x%02x', Cluster id: '0x%04x', Command id: '0x%02x'", mEndPointId, mClusterId,
                    mCommandId);

    PacketBufferHandle buffer = EncodeCommand(mEndPointId);
    VerifyOrExit(!buffer.IsNull(), err = CHIP_ERROR_INTERNAL);

#ifdef DEBUG
    PrintBuffer(buffer);
#endif

    err = device->SendMessage(std::move(buffer));
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(chipTool, "Failed to send message: %s", ErrorStr(err)));

exit:
    return err;
}

void ModelCommand::OnMessage(PacketBufferHandle buffer)
{
    ChipLogDetail(chipTool, "OnMessage: Received %zu bytes", buffer->DataLength());

    EmberApsFrame frame;
    uint8_t * message;
    uint16_t messageLen;
    uint16_t mfgCode;
    uint8_t frameControl;
    uint8_t sequenceNumber;
    uint8_t commandId;
    bool success = false;

    if (extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) == 0)
    {
        ChipLogError(chipTool, "APS frame processing failure!");
        ExitNow();
    }
    ChipLogDetail(chipTool, "APS frame processing success!");

    messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
    VerifyOrExit(messageLen >= 1, ChipLogError(chipTool, "Unexpected response length: %d", messageLen));

    frameControl = chip::Encoding::Read8(message);
    if (frameControl & (1u << 2))
    {
        VerifyOrExit(messageLen >= 5, ChipLogError(chipTool, "Unexpected response length: %d", messageLen));
        mfgCode = chip::Encoding::LittleEndian::Read16(message);
        ChipLogDetail(chipTool, "Manufacturer specific code in response: 0x%04x", mfgCode);
        messageLen = static_cast<uint16_t>(messageLen - 2);
    }
    VerifyOrExit(messageLen >= 3, ChipLogError(chipTool, "Unexpected response length: %d", messageLen));
    sequenceNumber = chip::Encoding::Read8(message);
    commandId      = chip::Encoding::Read8(message);
    messageLen     = static_cast<uint16_t>(messageLen - 3);

    VerifyOrExit(isValidFrame(frameControl), ChipLogError(chipTool, "Unexpected frame control byte: 0x%02x", frameControl));
    VerifyOrExit(sequenceNumber == 1, ChipLogError(chipTool, "Unexpected sequence number: %d", sequenceNumber));
    VerifyOrExit(mEndPointId == frame.sourceEndpoint,
                 ChipLogError(chipTool, "Unexpected endpoint id '0x%02x'", frame.sourceEndpoint));
    VerifyOrExit(mClusterId == frame.clusterId, ChipLogError(chipTool, "Unexpected cluster id '0x%04x'", frame.clusterId));

    success = isGlobalCommand(frameControl) ? HandleGlobalResponse(commandId, message, messageLen)
                                            : HandleSpecificResponse(commandId, message, messageLen);
exit:
    SetCommandExitStatus(success);
    UpdateWaitForResponse(false);
}

void ModelCommand::OnStatusChange(void)
{
    ChipLogProgress(chipTool, "DeviceStatusDelegate::OnStatusChange");
}

void ModelCommand::PrintBuffer(const PacketBufferHandle & buffer) const
{
    const size_t dataLen = buffer->DataLength();

    fprintf(stderr, "SENDING: %zu ", dataLen);
    for (size_t i = 0; i < dataLen; ++i)
    {
        fprintf(stderr, "%02x ", buffer->Start()[i]);
    }
    fprintf(stderr, "\n");
}
