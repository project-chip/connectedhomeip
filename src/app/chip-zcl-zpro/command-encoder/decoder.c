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
#include <stdio.h>
#include <string.h>

uint16_t extractApsFrame(uint8_t * buffer, uint32_t buf_length, EmberApsFrame * outApsFrame)
{

    if (buffer == NULL || buf_length == 0 || outApsFrame == NULL)
    {
        return 0;
    }
    // Skip first byte, because that's the always-0 frame control.
    uint8_t nextByteToRead = 1;

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->profileId, buffer + nextByteToRead, sizeof(outApsFrame->profileId));
    nextByteToRead += sizeof(outApsFrame->profileId);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->clusterId, buffer + nextByteToRead, sizeof(outApsFrame->clusterId));
    nextByteToRead += sizeof(outApsFrame->clusterId);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->sourceEndpoint, buffer + nextByteToRead, sizeof(outApsFrame->sourceEndpoint));
    nextByteToRead += sizeof(outApsFrame->sourceEndpoint);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->destinationEndpoint, buffer + nextByteToRead, sizeof(outApsFrame->destinationEndpoint));
    nextByteToRead += sizeof(outApsFrame->destinationEndpoint);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->options, buffer + nextByteToRead, sizeof(outApsFrame->options));
    nextByteToRead += sizeof(outApsFrame->options);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->groupId, buffer + nextByteToRead, sizeof(outApsFrame->groupId));
    nextByteToRead += sizeof(outApsFrame->groupId);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->sequence, buffer + nextByteToRead, sizeof(outApsFrame->sequence));
    nextByteToRead += sizeof(outApsFrame->sequence);

    if (nextByteToRead >= buf_length)
    {
        return 0;
    }
    memcpy(&outApsFrame->radius, buffer + nextByteToRead, sizeof(outApsFrame->radius));
    nextByteToRead += sizeof(outApsFrame->radius);

    return nextByteToRead;
}

void printApsFrame(EmberApsFrame * frame)
{
    printf("\n<EmberApsFrame %p> profileID %d, clusterID %d, sourceEndpoint %d, destinationEndPoint %d, options %d, groupID %d, "
           "sequence %d, radius %d\n",
           frame, frame->profileId, frame->clusterId, frame->sourceEndpoint, frame->destinationEndpoint, frame->options,
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
        result = buffer_length - apsFrameSize;
    }
    else if (msg)
    {
        *msg = NULL;
    }
    return result;
}
