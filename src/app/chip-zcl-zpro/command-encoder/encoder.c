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
#include <stdio.h>
#include <string.h>

uint32_t encodeApsFrame(uint8_t * buffer, uint32_t buf_length, uint16_t profileID, uint16_t clusterId, uint8_t sourceEndpoint,
                        uint8_t destinationEndpoint, EmberApsOption options, uint16_t groupId, uint8_t sequence, uint8_t radius)
{
    uint32_t size = 0;
    if (buffer == NULL || buf_length == 0)
    {
        return size;
    }
    size_t nextOutByte = 0;

    if (nextOutByte >= buf_length)
    {
        return size;
    }

    // Simulated APS "frame control" byte.
    buffer[nextOutByte] = 0;
    ++nextOutByte;

    if (nextOutByte >= buf_length)
    {
        return size;
    }

    memcpy(buffer + nextOutByte, &profileID, sizeof(uint16_t));
    nextOutByte += sizeof(uint16_t);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &clusterId, sizeof(uint16_t));
    nextOutByte += sizeof(uint16_t);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &sourceEndpoint, sizeof(uint8_t));
    nextOutByte += sizeof(uint8_t);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &destinationEndpoint, sizeof(uint8_t));
    nextOutByte += sizeof(uint8_t);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &options, sizeof(EmberApsOption));
    nextOutByte += sizeof(EmberApsOption);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &groupId, sizeof(uint16_t));
    nextOutByte += sizeof(uint16_t);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &sequence, sizeof(uint8_t));
    nextOutByte += sizeof(uint8_t);

    if (nextOutByte >= buf_length)
    {
        return size;
    }
    memcpy(buffer + nextOutByte, &radius, sizeof(uint8_t));
    nextOutByte += sizeof(uint8_t);

    buf_length = nextOutByte;
    printf("Encoded %d bytes of aps frame\n", buf_length);
    return buf_length;
}

uint32_t _encodeOnOffCommand(uint8_t * buffer, uint32_t buf_length, int command, uint8_t destination_endpoint)
{
    uint32_t result = 0;
    // pick cluster id as 6 for now.
    // pick source and destination end points as 1 for now.
    // Profile is 65535 because that matches our simple generated code, but we
    // should sort out the profile situation.
    result = encodeApsFrame(buffer, buf_length, 65535, 6, 1, destination_endpoint, 0, 0, 0, 0);
    if (result == 0 || result > buf_length)
    {
        printf("Error encoding aps frame result %d", result);
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
        printf("indexToWrite %d\n", indexToWrite);
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
