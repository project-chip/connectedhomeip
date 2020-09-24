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

#include <support/BufBound.h>
#include <support/logging/CHIPLogging.h>

#define CHECK_FRAME_LENGTH(value, name)                                                                                            \
    if (value == 0)                                                                                                                \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding APS Frame: %s", name);                                                                   \
        return value;                                                                                                              \
    }

#define CHECK_COMMAND_LENGTH(value, name)                                                                                          \
    if (value == 0)                                                                                                                \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return value;                                                                                                              \
    }

using namespace chip;
extern "C" {

static uint16_t doEncodeApsFrame(BufBound & buf, uint16_t profileID, uint16_t clusterId, uint8_t sourceEndpoint,
                                 uint8_t destinationEndpoint, EmberApsOption options, uint16_t groupId, uint8_t sequence,
                                 uint8_t radius, bool isMeasuring)
{

    uint8_t control_byte = 0;
    buf.Put(control_byte); // Put in a control byte
    buf.PutLE16(profileID);
    buf.PutLE16(clusterId);
    buf.Put(sourceEndpoint);
    buf.Put(destinationEndpoint);
    buf.PutLE(options, sizeof(EmberApsOption));
    buf.PutLE16(groupId);
    buf.Put(sequence);
    buf.Put(radius);

    uint16_t result = 0;
    if (isMeasuring)
    {
        result = buf.Written();
        ChipLogProgress(Zcl, "Measured APS frame size %d", result);
    }
    else
    {
        result = buf.Fit() ? buf.Written() : 0;
        CHECK_FRAME_LENGTH(result, "Buffer too small");
        ChipLogProgress(Zcl, "Successfully encoded %d bytes", result);
    }
    return result;
}

uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame)
{
    BufBound buf = BufBound(buffer, buf_length);
    return doEncodeApsFrame(buf, apsFrame->profileId, apsFrame->clusterId, apsFrame->sourceEndpoint, apsFrame->destinationEndpoint,
                            apsFrame->options, apsFrame->groupId, apsFrame->sequence, apsFrame->radius, !buffer);
}

uint16_t _encodeCommand(BufBound & buf, uint8_t destination_endpoint, uint16_t cluster_id, uint8_t command, uint8_t frame_control)
{
    CHECK_FRAME_LENGTH(buf.Size(), "Buffer is empty");

    uint8_t seq_num         = 1;     // Transaction sequence number.  Just pick something.
    uint8_t source_endpoint = 1;     // Pick source endpoint as 1 for now.
    uint16_t profile_id     = 65535; // Profile is 65535 because that matches our simple generated code, but we
                                     // should sort out the profile situation.

    if (doEncodeApsFrame(buf, profile_id, cluster_id, source_endpoint, destination_endpoint, 0, 0, 0, 0, false))
    {
        buf.Put(frame_control);
        buf.Put(seq_num);
        buf.Put(command);
    }

    return buf.Fit() ? buf.Written() : 0;
}

uint16_t _encodeClusterSpecificCommand(BufBound & buf, uint8_t destination_endpoint, uint16_t cluster_id, uint8_t command)
{
    // This is a cluster-specific command so low two bits are 0b01.  The command
    // is standard, so does not need a manufacturer code, and we're sending
    // client to server, so all the remaining bits are 0.
    uint8_t frame_control = 0x01;

    return _encodeCommand(buf, destination_endpoint, cluster_id, command, frame_control);
}

uint16_t _encodeGlobalCommand(BufBound & buf, uint8_t destination_endpoint, uint16_t cluster_id, uint8_t command)
{
    // This is a global command, so the low bits are 0b00.  The command is
    // standard, so does not need a manufacturer code, and we're sending client
    // to server, so all the remaining bits are 0.
    uint8_t frame_control = 0x00;

    return _encodeCommand(buf, destination_endpoint, cluster_id, command, frame_control);
}

uint16_t encodeReadAttributesCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t cluster_id,
                                     uint16_t * attr_ids, uint16_t attr_id_count)
{
    BufBound buf = BufBound(buffer, buf_length);
    if (_encodeGlobalCommand(buf, destination_endpoint, cluster_id, 0x00))
    {
        for (uint16_t i = 0; i < attr_id_count; ++i)
        {
            uint16_t attr_id = attr_ids[i];
            buf.PutLE16(attr_id);
        }
    }

    return buf.Fit() ? buf.Written() : 0;
}

/*
 * On/Off Cluster commands
 *
 * Pick cluster id as 0x0006 for now
 */

uint16_t encodeOffCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint)
{
    BufBound buf    = BufBound(buffer, buf_length);
    uint16_t result = _encodeClusterSpecificCommand(buf, destination_endpoint, 0x0006, 0x00);
    CHECK_COMMAND_LENGTH(result, "Off");
    return result;
};

uint16_t encodeOnCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint)
{
    BufBound buf    = BufBound(buffer, buf_length);
    uint16_t result = _encodeClusterSpecificCommand(buf, destination_endpoint, 0x0006, 0x01);
    CHECK_COMMAND_LENGTH(result, "On");
    return result;
}

uint16_t encodeToggleCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint)
{
    BufBound buf    = BufBound(buffer, buf_length);
    uint16_t result = _encodeClusterSpecificCommand(buf, destination_endpoint, 0x0006, 0x02);
    CHECK_COMMAND_LENGTH(result, "Toggle");
    return result;
}

uint16_t encodeReadOnOffCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint)
{
    uint16_t attr_id       = 0x0000; /* OnOff attribute */
    uint16_t attr_id_count = 1;

    uint16_t result = encodeReadAttributesCommand(buffer, buf_length, destination_endpoint, 0x0006, &attr_id, attr_id_count);
    CHECK_COMMAND_LENGTH(result, "ReadOnOff");
    return result;
}

uint16_t _encodeIdentifyClusterCommand(uint8_t * buffer, uint16_t buf_length, uint8_t command_id, uint8_t destination_endpoint,
                                       uint16_t * identify_duration)
{

    BufBound buf    = BufBound(buffer, buf_length);
    uint16_t result = _encodeClusterSpecificCommand(buf, destination_endpoint, 0x0003, command_id);

    if (identify_duration)
    {
        buf.PutLE16(*identify_duration);
    }

    result = buf.Fit() ? buf.Written() : 0;

    return result;
}

uint16_t encodeIdentifyQueryCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint)
{
    uint16_t result = _encodeIdentifyClusterCommand(buffer, buf_length, 1, destination_endpoint, nullptr);
    CHECK_COMMAND_LENGTH(result, "Identify Query");
    return result;
}

uint16_t encodeIdentifyCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint16_t duration)
{
    uint16_t result = _encodeIdentifyClusterCommand(buffer, buf_length, 0, destination_endpoint, &duration);
    CHECK_COMMAND_LENGTH(result, "Identify");
    return result;
}

} // extern "C"
