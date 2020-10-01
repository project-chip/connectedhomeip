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

#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>

#define CHECK_FRAME_LENGTH(value, name)                                                                                            \
    if (value == 0)                                                                                                                \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding APS Frame: %s", name);                                                                   \
        return 0;                                                                                                                  \
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

    size_t result = 0;
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
    if (!CanCastTo<uint16_t>(result))
    {
        ChipLogProgress(Zcl, "Can't fit our measured size in uint16_t");
        result = 0;
    }

    return static_cast<uint16_t>(result);
}

uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame)
{
    BufBound buf = BufBound(buffer, buf_length);
    return doEncodeApsFrame(buf, apsFrame->profileId, apsFrame->clusterId, apsFrame->sourceEndpoint, apsFrame->destinationEndpoint,
                            apsFrame->options, apsFrame->groupId, apsFrame->sequence, apsFrame->radius, !buffer);
}

} // extern "C"

uint16_t encodeCommand(BufBound & buf, uint8_t destinationEndpointId, uint16_t clusterId, uint8_t commandId, uint8_t frameControl)
{
    CHECK_FRAME_LENGTH(buf.Size(), "Buffer is empty");

    uint8_t seqNum           = 1;     // Transaction sequence number.  Just pick something.
    uint8_t sourceEndpointId = 1;     // Pick source endpoint as 1 for now.
    uint16_t profileId       = 65535; // Profile is 65535 because that matches our simple generated code, but we
                                      // should sort out the profile situation.

    if (doEncodeApsFrame(buf, profileId, clusterId, sourceEndpointId, destinationEndpointId, 0, 0, 0, 0, false))
    {
        buf.Put(frameControl);
        buf.Put(seqNum);
        buf.Put(commandId);
    }

    return buf.Fit() && CanCastTo<uint16_t>(buf.Written()) ? static_cast<uint16_t>(buf.Written()) : 0;
}
uint16_t encodeClusterSpecificCommandHeader(BufBound & buf, uint8_t endpointId, uint16_t clusterId, uint8_t commandId)
{
    // This is a cluster-specific command so low two bits are 0b01.  The command
    // is standard, so does not need a manufacturer code, and we're sending
    // client to server, so all the remaining bits are 0.
    uint8_t frameControl = 0x01;

    return encodeCommand(buf, endpointId, clusterId, commandId, frameControl);
}

uint16_t encodeGlobalCommandHeader(BufBound & buf, uint8_t endpointId, uint16_t clusterId, uint8_t commandId)
{
    // This is a global command, so the low bits are 0b00.  The command is
    // standard, so does not need a manufacturer code, and we're sending client
    // to server, so all the remaining bits are 0.
    uint8_t frameControl = 0x00;

    return encodeCommand(buf, endpointId, clusterId, commandId, frameControl);
}
