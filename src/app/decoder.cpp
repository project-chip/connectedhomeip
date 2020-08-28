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

#include <app/chip-zcl-zpro-codec.h>
#include <core/CHIPEncoding.h>
#include <stdio.h>
#include <string.h>
#include <support/logging/CHIPLogging.h>

template <int N>
struct Reader
{
};

template <>
struct Reader<1>
{
    static uint8_t read(const uint8_t *& p) { return chip::Encoding::Read8(p); }
};

template <>
struct Reader<2>
{
    static uint16_t read(const uint8_t *& p) { return chip::Encoding::LittleEndian::Read16(p); }
};

extern "C" {

uint16_t extractApsFrame(uint8_t * buffer, uint32_t buf_length, EmberApsFrame * outApsFrame)
{
    if (buffer == NULL || buf_length == 0 || outApsFrame == NULL)
    {
        ChipLogError(Zcl, "Error extracting APS frame. invalid inputs");
        return 0;
    }

    const uint8_t * read_ptr = buffer;

    // Skip first byte, because that's the always-0 frame control.
    ++read_ptr;
    --buf_length;

#define TRY_READ(fieldName, fieldSize)                                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        static_assert(sizeof(outApsFrame->fieldName) == fieldSize, "incorrect size for " #fieldName);                              \
        if (buf_length < fieldSize)                                                                                                \
        {                                                                                                                          \
            ChipLogError(Zcl, "Missing " #fieldName " when extracting APS frame");                                                 \
            return 0;                                                                                                              \
        }                                                                                                                          \
        outApsFrame->fieldName = Reader<fieldSize>::read(read_ptr);                                                                \
        buf_length -= fieldSize;                                                                                                   \
    } while (0)

    TRY_READ(profileId, 2);
    TRY_READ(clusterId, 2);
    TRY_READ(sourceEndpoint, 1);
    TRY_READ(destinationEndpoint, 1);
    TRY_READ(options, 2);
    TRY_READ(groupId, 2);
    TRY_READ(sequence, 1);
    TRY_READ(radius, 1);

#undef TRY_READ

    return read_ptr - buffer;
}

void printApsFrame(EmberApsFrame * frame)
{
    ChipLogProgress(
        Zcl,
        "\n<EmberApsFrame %p> profileID %d, clusterID %d, sourceEndpoint %d, destinationEndPoint %d, options %d, groupID %d, "
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

} // extern C
