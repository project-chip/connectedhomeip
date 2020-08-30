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
        if (result > 0)
        {
            ChipLogProgress(Zcl, "Successfully encoded %d bytes", result);
        }
        else
        {
            ChipLogError(Zcl, "Error encoding APS frame");
        }
    }
    return result;
}

uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame)
{
    BufBound buf = BufBound(buffer, buf_length);
    return doEncodeApsFrame(buf, apsFrame->profileId, apsFrame->clusterId, apsFrame->sourceEndpoint, apsFrame->destinationEndpoint,
                            apsFrame->options, apsFrame->groupId, apsFrame->sequence, apsFrame->radius, !buffer);
}

uint32_t _encodeOnOffCommand(uint8_t * buffer, uint32_t buf_length, uint8_t command, uint8_t destination_endpoint)
{
    uint32_t result = 0;
    // pick cluster id as 6 for now.
    // pick source and destination end points as 1 for now.
    // Profile is 65535 because that matches our simple generated code, but we
    // should sort out the profile situation.
    if (!buffer)
    {
        return 0;
    }
    BufBound buf = BufBound(buffer, buf_length);
    result       = doEncodeApsFrame(buf, 65535, 6, 1, destination_endpoint, 0, 0, 0, 0, false);
    if (result == 0)
    {
        ChipLogError(Zcl, "Error encoding aps frame result %" PRIu32 "\n", result);
        return result;
    }

    // This is a cluster-specific command so low two bits are 0b01.  The command
    // is standard, so does not need a manufacturer code, and we're sending
    // client to server, so all the remaining bits are 0.
    buf.Put(uint8_t(1));

    // Transaction sequence number.  Just pick something.
    buf.Put(uint8_t(1));

    buf.Put(command);

    result = buf.Fit() ? buf.Written() : 0;
    if (result == 0)
    {
        ChipLogError(Zcl, "Error encoding on / off cmd");
    }
    return result;
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

uint16_t encodeReadAttributesCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t cluster_id,
                                     uint16_t * attr_ids, uint16_t attr_id_count)
{
    if (!buffer)
    {
        return 0;
    }

    BufBound buf    = BufBound(buffer, buf_length);
    uint16_t result = doEncodeApsFrame(buf, 65535, 6, 1, destination_endpoint, 0, 0, 0, 0, false);
    if (result == 0)
    {
        ChipLogError(Zcl, "Error encoding read attributes aps frame\n");
        return 0;
    }

    // This is a global command, so the low bits are 0b00.  The command is
    // standard, so does not need a manufacturer code, and we're sending client
    // to server, so all the remaining bits are 0.
    uint8_t frameControl = 0x00;
    buf.Put(frameControl);

    // Transaction sequence number.  Just pick something.
    uint8_t seqNum = 0x1;
    buf.Put(seqNum);

    uint8_t readAttributesCommandId = 0x00;
    buf.Put(readAttributesCommandId);

    for (uint16_t i = 0; i < attr_id_count; ++i)
    {
        uint16_t attr_id = attr_ids[i];
        buf.PutLE16(attr_id);
    }

    result = buf.Fit() ? buf.Written() : 0;
    if (result == 0)
    {
        ChipLogError(Zcl, "Error encoding read attributes cmd");
    }
    return result;
}

uint16_t encodeReadOnOffCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint)
{
    uint16_t attr_id = 0x0000; /* OnOff attribute */
    return encodeReadAttributesCommand(buffer, buf_length, destination_endpoint, 0x6 /* cluster_id */, &attr_id,
                                       1 /* attr_id_count */);
}

} // extern "C"
