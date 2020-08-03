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

#include "chip-zcl-zpro-codec.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

static uint16_t doEncodeApsFrame(uint8_t * buffer, uint32_t buf_length, uint16_t profileID, uint16_t clusterId,
                                 uint8_t sourceEndpoint, uint8_t destinationEndpoint, EmberApsOption options, uint16_t groupId,
                                 uint8_t sequence, uint8_t radius)
{
    size_t nextOutByte = 0;

#define TRY_WRITE(dataItem)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        size_t neededSize = nextOutByte + sizeof(dataItem);                                                                        \
        if (buffer)                                                                                                                \
        {                                                                                                                          \
            if (neededSize > buf_length)                                                                                           \
            {                                                                                                                      \
                return 0;                                                                                                          \
            }                                                                                                                      \
            memcpy(buffer + nextOutByte, &dataItem, sizeof(dataItem));                                                             \
        }                                                                                                                          \
        nextOutByte = neededSize;                                                                                                  \
    } while (0) /* No semicolon so callers have to provide it. */

    // Simulated APS "frame control" byte.
    uint8_t controlByte = 0;
    TRY_WRITE(controlByte);
    TRY_WRITE(profileID);
    TRY_WRITE(clusterId);
    TRY_WRITE(sourceEndpoint);
    TRY_WRITE(destinationEndpoint);
    TRY_WRITE(options);
    TRY_WRITE(groupId);
    TRY_WRITE(sequence);
    TRY_WRITE(radius);

#undef TRY_WRITE

    assert(nextOutByte < UINT16_MAX);

    buf_length = nextOutByte;
    printf("Encoded %" PRIu32 " bytes of aps frame\n", buf_length);
    return buf_length;
}

uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame)
{
    return doEncodeApsFrame(buffer, buf_length, apsFrame->profileId, apsFrame->clusterId, apsFrame->sourceEndpoint,
                            apsFrame->destinationEndpoint, apsFrame->options, apsFrame->groupId, apsFrame->sequence,
                            apsFrame->radius);
}

uint32_t _encodeOnOffCommand(uint8_t * buffer, uint32_t buf_length, int command, uint8_t destination_endpoint)
{
    uint32_t result = 0;
    // pick cluster id as 6 for now.
    // pick source and destination end points as 1 for now.
    // Profile is 65535 because that matches our simple generated code, but we
    // should sort out the profile situation.
    result = doEncodeApsFrame(buffer, buf_length, 65535, 6, 1, destination_endpoint, 0, 0, 0, 0);
    if (result == 0 || result > buf_length)
    {
        printf("Error encoding aps frame result %" PRIu32 "\n", result);
        result = 0;
        return result;
    }
    uint32_t indexToWrite = result;
    // This is a cluster-specific command so low two bits are 0b01.  The command
    // is standard, so does not need a manufacturer code, and we're sending
    // client to server, so all the remaining bits are 0.

    uint8_t * val = buffer + indexToWrite++;
    *val          = 0x1;

    if (indexToWrite >= buf_length)
    {
        printf("indexToWrite %" PRIu32 "\n", indexToWrite);
        return 0;
    }
    // Transaction sequence number.  Just pick something.
    val  = buffer + indexToWrite++;
    *val = 0x1;

    if (indexToWrite >= buf_length)
    {
        return 0;
    }

    val  = buffer + indexToWrite++;
    *val = command;

    if (indexToWrite >= buf_length)
    {
        return 0;
    }
    return indexToWrite;
}

uint32_t encodeOffCommand(uint8_t * buffer, uint32_t buf_length, uint8_t destination_endpoint)
{
    return _encodeOnOffCommand(buffer, buf_length, 0, destination_endpoint);
};

uint32_t encodeOnCommand(uint8_t * buffer, uint32_t buf_length, uint8_t destination_endpoint)
{
    return _encodeOnOffCommand(buffer, buf_length, 1, destination_endpoint);
}

uint32_t encodeToggleCommand(uint8_t * buffer, uint32_t buf_length, uint8_t destination_endpoint)
{
    return _encodeOnOffCommand(buffer, buf_length, 2, destination_endpoint);
}
