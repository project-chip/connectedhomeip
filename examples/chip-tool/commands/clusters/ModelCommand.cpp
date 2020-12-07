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
constexpr uint8_t kZCLGlobalCmdFrameControlHeader  = 8;
constexpr uint8_t kZCLClusterCmdFrameControlHeader = 9;

bool isValidFrame(uint8_t frameControl)
{
    // Bit 3 of the frame control byte set means direction is server to client.
    return (frameControl == kZCLGlobalCmdFrameControlHeader || frameControl == kZCLClusterCmdFrameControlHeader);
}

bool isGlobalCommand(uint8_t frameControl)
{
    return (frameControl == kZCLGlobalCmdFrameControlHeader);
}
} // namespace

uint16_t ModelCommand::Encode(PacketBufferHandle & buffer, uint16_t bufferSize)
{
    ChipLogProgress(chipTool, "Endpoint id: '0x%02x', Cluster id: '0x%04x', Command id: '0x%02x'", mEndPointId, mClusterId,
                    mCommandId);

    return EncodeCommand(buffer, bufferSize, mEndPointId);
}

bool ModelCommand::Decode(PacketBufferHandle & buffer) const
{
    EmberApsFrame frame;
    uint8_t * message;
    uint16_t messageLen;
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
    VerifyOrExit(messageLen >= 3, ChipLogError(chipTool, "Unexpected response length: %d", messageLen));

    frameControl   = chip::Encoding::Read8(message);
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
    return success;
}
