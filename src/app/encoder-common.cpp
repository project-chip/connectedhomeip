/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::Encoding::LittleEndian;

uint16_t doEncodeApsFrame(BufferWriter & buf, chip::ClusterId clusterId, chip::EndpointId sourceEndpoint,
                          chip::EndpointId destinationEndpoint, EmberApsOption options, chip::GroupId groupId, uint8_t sequence,
                          uint8_t radius, bool isMeasuring)
{

    uint8_t control_byte = 0;
    buf.Put(control_byte) // Put in a control byte
        .Put32(clusterId)
        .Put16(sourceEndpoint)
        .Put16(destinationEndpoint)
        .EndianPut(options, sizeof(EmberApsOption))
        .Put16(groupId)
        .Put8(sequence)
        .Put8(radius);

    size_t result = buf.Needed();
    if (isMeasuring)
    {
        ChipLogDetail(Zcl, "Measured APS frame size %zu", result);
    }
    else if (buf.Fit())
    {
        ChipLogDetail(Zcl, "Successfully encoded %zu bytes", result);
    }
    else
    {
        ChipLogError(Zcl, "Error encoding APS Frame: Buffer too small");
        result = 0;
    }

    if (!chip::CanCastTo<uint16_t>(result))
    {
        ChipLogError(Zcl, "Can't fit our measured size in uint16_t");
        result = 0;
    }

    return static_cast<uint16_t>(result);
}

uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame)
{
    BufferWriter buf(buffer, buf_length);
    return doEncodeApsFrame(buf, apsFrame->clusterId, apsFrame->sourceEndpoint, apsFrame->destinationEndpoint, apsFrame->options,
                            apsFrame->groupId, apsFrame->sequence, apsFrame->radius, !buffer);
}
