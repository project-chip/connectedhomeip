/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// bool emberAfProcessMessage(EmberApsFrame *apsFrame,
//                            EmberIncomingMessageType type,
//                            uint8_t *message,
//                            uint16_t msgLen,
//                            EmberNodeId source,
//                            InterPanHeader *interPanHeader)

#include "chip-zcl-zpro-codec.h"

#include <app/message-reader.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <stdio.h>
#include <string.h>

uint16_t extractApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * outApsFrame)
{
    if (buffer == nullptr || buf_length == 0 || outApsFrame == nullptr)
    {
        ChipLogError(Zcl, "Error extracting APS frame. invalid inputs");
        return 0;
    }

    chip::DataModelReader reader(buffer, buf_length);

    // Skip first byte, because that's the always-0 frame control.
    uint8_t ignored;
    const CHIP_ERROR err = reader.ReadOctet(&ignored)
                               .ReadClusterId(&outApsFrame->clusterId)
                               .ReadEndpointId(&outApsFrame->sourceEndpoint)
                               .ReadEndpointId(&outApsFrame->destinationEndpoint)
                               .Read16(&outApsFrame->options)
                               .ReadGroupId(&outApsFrame->groupId)
                               .ReadOctet(&outApsFrame->sequence)
                               .ReadOctet(&outApsFrame->radius)
                               .StatusCode();

    return err == CHIP_NO_ERROR ? static_cast<uint16_t>(reader.OctetsRead()) : 0;
}

void printApsFrame(EmberApsFrame * frame)
{
    ChipLogProgress(Zcl,
                    "\n<EmberApsFrame %p> clusterID " ChipLogFormatMEI
                    ", sourceEndpoint %d, destinationEndPoint %d, options %d, groupID %d, "
                    "sequence %d, radius %d\n",
                    frame, ChipLogValueMEI(frame->clusterId), frame->sourceEndpoint, frame->destinationEndpoint, frame->options,
                    frame->groupId, frame->sequence, frame->radius);
}

uint16_t extractMessage(uint8_t * buffer, uint16_t buffer_length, uint8_t ** msg)
{
    // The message starts after the EmberApsFrame.
    uint16_t result = 0;
    EmberApsFrame frame;
    uint16_t apsFrameSize = extractApsFrame(buffer, buffer_length, &frame);
    if (msg && apsFrameSize > 0)
    {
        *msg   = buffer + apsFrameSize;
        result = static_cast<uint16_t>(buffer_length - apsFrameSize);
    }
    else if (msg)
    {
        *msg = nullptr;
    }
    return result;
}
