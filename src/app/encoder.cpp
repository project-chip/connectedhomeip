
/*
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

#include <support/BufBound.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>

#include <app/util/basic-types.h>

using namespace chip;

#define CHECK_FRAME_LENGTH(value, name)                                                                                            \
    if (value == 0)                                                                                                                \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding APS Frame: %s", name);                                                                   \
        return 0;                                                                                                                  \
    }

#define READ_ATTRIBUTES(name, cluster_id)                                                                                          \
    uint16_t attr_id_count = sizeof(attr_ids) / sizeof(attr_ids[0]);                                                               \
    uint16_t result = encodeReadAttributesCommand(buffer, buf_length, destination_endpoint, cluster_id, attr_ids, attr_id_count);  \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return 0;                                                                                                                  \
    }                                                                                                                              \
    return result;

#define WRITE_ATTRIBUTE(name, cluster_id, value)                                                                                   \
    BufBound buf = BufBound(buffer, buf_length);                                                                                   \
    if (_encodeGlobalCommand(buf, destination_endpoint, cluster_id, 0x02))                                                         \
    {                                                                                                                              \
        buf.Put16(attr_id);                                                                                                        \
        buf.Put(attr_type);                                                                                                        \
        switch (attr_type)                                                                                                         \
        {                                                                                                                          \
        case 0x21:                                                                                                                 \
            buf.Put16(static_cast<uint16_t>(value));                                                                               \
            break;                                                                                                                 \
        case 0x18:                                                                                                                 \
            buf.Put(static_cast<uint8_t>(value));                                                                                  \
            break;                                                                                                                 \
        case 0x20:                                                                                                                 \
            buf.Put(static_cast<uint8_t>(value));                                                                                  \
            break;                                                                                                                 \
        case 0xF0:                                                                                                                 \
            buf.Put64(static_cast<uint64_t>(value));                                                                               \
            break;                                                                                                                 \
        default:                                                                                                                   \
            ChipLogError(Zcl, "Error encoding %s command", name);                                                                  \
            return 0;                                                                                                              \
        }                                                                                                                          \
    }                                                                                                                              \
                                                                                                                                   \
    uint16_t result = buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;                    \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return 0;                                                                                                                  \
    }                                                                                                                              \
    return result;

#define REPORT_ATTRIBUTE(name, cluster_id, isAnalog, value)                                                                        \
    BufBound buf = BufBound(buffer, buf_length);                                                                                   \
    if (_encodeGlobalCommand(buf, destination_endpoint, cluster_id, 0x06))                                                         \
    {                                                                                                                              \
        uint8_t direction = 0x00;                                                                                                  \
        buf.Put(direction);                                                                                                        \
        buf.Put16(attr_id);                                                                                                        \
        buf.Put(attr_type);                                                                                                        \
        buf.Put16(min_interval);                                                                                                   \
        buf.Put16(max_interval);                                                                                                   \
        if (isAnalog)                                                                                                              \
        {                                                                                                                          \
            switch (attr_type)                                                                                                     \
            {                                                                                                                      \
            case 0x20:                                                                                                             \
                buf.Put(static_cast<uint8_t>(value));                                                                              \
                break;                                                                                                             \
            case 0x21:                                                                                                             \
                buf.Put16(static_cast<uint16_t>(value));                                                                           \
                break;                                                                                                             \
            case 0x29:                                                                                                             \
                buf.Put16(static_cast<uint16_t>(value));                                                                           \
                break;                                                                                                             \
            default:                                                                                                               \
                ChipLogError(Zcl, "Type is not supported for report attribute: '0x%02x'", attr_type);                              \
                break;                                                                                                             \
            }                                                                                                                      \
        }                                                                                                                          \
    }                                                                                                                              \
                                                                                                                                   \
    uint16_t result = buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;                    \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return 0;                                                                                                                  \
    }                                                                                                                              \
    return result;

#define DISCOVER_ATTRIBUTES(name, cluster_id)                                                                                      \
    BufBound buf = BufBound(buffer, buf_length);                                                                                   \
    if (_encodeGlobalCommand(buf, destination_endpoint, cluster_id, 0x0c))                                                         \
    {                                                                                                                              \
        /* Discover all attributes */                                                                                              \
        buf.Put16(0x0000);                                                                                                         \
        buf.Put(0xFF);                                                                                                             \
    }                                                                                                                              \
                                                                                                                                   \
    uint16_t result = buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;                    \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return 0;                                                                                                                  \
    }                                                                                                                              \
    return result;

#define COMMAND_HEADER(name, cluster_id, command_id)                                                                               \
    BufBound buf    = BufBound(buffer, buf_length);                                                                                \
    uint16_t result = _encodeClusterSpecificCommand(buf, destination_endpoint, cluster_id, command_id);                            \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return 0;                                                                                                                  \
    }

#define COMMAND_FOOTER(name)                                                                                                       \
    result = buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;                             \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Error encoding %s command", name);                                                                      \
        return 0;                                                                                                                  \
    }                                                                                                                              \
    return result;

#define COMMAND_INSERT_STRING(name, str)                                                                                           \
    {                                                                                                                              \
        size_t str_length = strlen(str);                                                                                           \
        if (!CanCastTo<uint8_t>(str_length))                                                                                       \
        {                                                                                                                          \
            ChipLogError(Zcl, "Error encoding %s command. String too long: %d", name, str_length);                                 \
            return 0;                                                                                                              \
        }                                                                                                                          \
        buf.Put(static_cast<uint8_t>(str_length));                                                                                 \
        buf.Put(str);                                                                                                              \
    }

#define COMMAND(name, cluster_id, command_id)                                                                                      \
    COMMAND_HEADER(name, cluster_id, command_id);                                                                                  \
    COMMAND_FOOTER(name);

using namespace chip;
extern "C" {

/*----------------------------------------------------------------------------*\
| Cluster Name                                                        |   ID   |
|---------------------------------------------------------------------+--------|
| BarrierControl                                                      | 0x0103 |
| Basic                                                               | 0x0000 |
| ColorControl                                                        | 0x0300 |
| DoorLock                                                            | 0x0101 |
| Groups                                                              | 0x0004 |
| IasZone                                                             | 0x0500 |
| Identify                                                            | 0x0003 |
| LevelControl                                                        | 0x0008 |
| OnOff                                                               | 0x0006 |
| Scenes                                                              | 0x0005 |
| TemperatureMeasurement                                              | 0x0402 |
\*----------------------------------------------------------------------------*/

#define BARRIER_CONTROL_CLUSTER_ID 0x0103
#define BASIC_CLUSTER_ID 0x0000
#define COLOR_CONTROL_CLUSTER_ID 0x0300
#define DOOR_LOCK_CLUSTER_ID 0x0101
#define GROUPS_CLUSTER_ID 0x0004
#define IAS_ZONE_CLUSTER_ID 0x0500
#define IDENTIFY_CLUSTER_ID 0x0003
#define LEVEL_CONTROL_CLUSTER_ID 0x0008
#define ON_OFF_CLUSTER_ID 0x0006
#define SCENES_CLUSTER_ID 0x0005
#define TEMP_MEASUREMENT_CLUSTER_ID 0x0402

static uint16_t doEncodeApsFrame(BufBound & buf, ClusterId clusterId, EndpointId sourceEndpoint, EndpointId destinationEndpoint,
                                 EmberApsOption options, GroupId groupId, uint8_t sequence, uint8_t radius, bool isMeasuring)
{

    uint8_t control_byte = 0;
    buf.Put(control_byte) // Put in a control byte
        .Put16(clusterId)
        .Put(sourceEndpoint)
        .Put(destinationEndpoint)
        .Put(options, sizeof(EmberApsOption))
        .Put16(groupId)
        .Put(sequence)
        .Put(radius);

    size_t result = 0;
    if (isMeasuring)
    {
        result = buf.Needed();
        ChipLogDetail(Zcl, "Measured APS frame size %d", result);
    }
    else
    {
        result = buf.Fit() ? buf.Needed() : 0;
        CHECK_FRAME_LENGTH(result, "Buffer too small");
        ChipLogDetail(Zcl, "Successfully encoded %d bytes", result);
    }
    if (!CanCastTo<uint16_t>(result))
    {
        ChipLogError(Zcl, "Can't fit our measured size in uint16_t");
        result = 0;
    }

    return static_cast<uint16_t>(result);
}

uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame)
{
    BufBound buf = BufBound(buffer, buf_length);
    return doEncodeApsFrame(buf, apsFrame->clusterId, apsFrame->sourceEndpoint, apsFrame->destinationEndpoint, apsFrame->options,
                            apsFrame->groupId, apsFrame->sequence, apsFrame->radius, !buffer);
}

uint16_t _encodeCommand(BufBound & buf, EndpointId destination_endpoint, ClusterId cluster_id, CommandId command,
                        uint8_t frame_control)
{
    CHECK_FRAME_LENGTH(buf.Size(), "Buffer is empty");

    uint8_t seq_num            = 1; // Transaction sequence number.  Just pick something.
    EndpointId source_endpoint = 1; // Pick source endpoint as 1 for now.

    if (doEncodeApsFrame(buf, cluster_id, source_endpoint, destination_endpoint, 0, 0, 0, 0, false))
    {
        buf.Put(frame_control);
        buf.Put(seq_num);
        buf.Put(command);
    }

    return buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;
}

uint16_t _encodeClusterSpecificCommand(BufBound & buf, EndpointId destination_endpoint, ClusterId cluster_id, CommandId command)
{
    // This is a cluster-specific command so low two bits are 0b01.  The command
    // is standard, so does not need a manufacturer code, and we're sending
    // client to server, so all the remaining bits are 0.
    uint8_t frame_control = 0x01;

    return _encodeCommand(buf, destination_endpoint, cluster_id, command, frame_control);
}

uint16_t _encodeGlobalCommand(BufBound & buf, EndpointId destination_endpoint, ClusterId cluster_id, CommandId command)
{
    // This is a global command, so the low bits are 0b00.  The command is
    // standard, so does not need a manufacturer code, and we're sending client
    // to server, so all the remaining bits are 0.
    uint8_t frame_control = 0x00;

    return _encodeCommand(buf, destination_endpoint, cluster_id, command, frame_control);
}

uint16_t encodeReadAttributesCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint, ClusterId cluster_id,
                                     const uint16_t * attr_ids, uint16_t attr_id_count)
{
    BufBound buf = BufBound(buffer, buf_length);
    if (_encodeGlobalCommand(buf, destination_endpoint, cluster_id, 0x00))
    {
        for (uint16_t i = 0; i < attr_id_count; ++i)
        {
            uint16_t attr_id = attr_ids[i];
            buf.Put16(attr_id);
        }
    }

    return buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;
}

/*----------------------------------------------------------------------------*\
| Cluster BarrierControl                                              | 0x0103 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * BarrierControlGoToPercent                                         |   0x00 |
| * BarrierControlStop                                                |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * BarrierMovingState                                                | 0x0001 |
| * BarrierSafetyStatus                                               | 0x0002 |
| * BarrierCapabilities                                               | 0x0003 |
| * BarrierPosition                                                   | 0x000A |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command BarrierControlGoToPercent
 */
uint16_t encodeBarrierControlClusterBarrierControlGoToPercentCommand(uint8_t * buffer, uint16_t buf_length,
                                                                     EndpointId destination_endpoint, uint8_t percentOpen)
{
    const char * kName = "BarrierControlBarrierControlGoToPercent";
    COMMAND_HEADER(kName, BARRIER_CONTROL_CLUSTER_ID, 0x00);
    buf.Put(percentOpen);
    COMMAND_FOOTER(kName);
}

/*
 * Command BarrierControlStop
 */
uint16_t encodeBarrierControlClusterBarrierControlStopCommand(uint8_t * buffer, uint16_t buf_length,
                                                              EndpointId destination_endpoint)
{
    const char * kName = "BarrierControlBarrierControlStop";
    COMMAND_HEADER(kName, BARRIER_CONTROL_CLUSTER_ID, 0x01);
    COMMAND_FOOTER(kName);
}

uint16_t encodeBarrierControlClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverBarrierControlAttributes", BARRIER_CONTROL_CLUSTER_ID);
}

/*
 * Attribute BarrierMovingState
 */
uint16_t encodeBarrierControlClusterReadBarrierMovingStateAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0001 };
    READ_ATTRIBUTES("ReadBarrierControlBarrierMovingState", BARRIER_CONTROL_CLUSTER_ID);
}

/*
 * Attribute BarrierSafetyStatus
 */
uint16_t encodeBarrierControlClusterReadBarrierSafetyStatusAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0002 };
    READ_ATTRIBUTES("ReadBarrierControlBarrierSafetyStatus", BARRIER_CONTROL_CLUSTER_ID);
}

/*
 * Attribute BarrierCapabilities
 */
uint16_t encodeBarrierControlClusterReadBarrierCapabilitiesAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0003 };
    READ_ATTRIBUTES("ReadBarrierControlBarrierCapabilities", BARRIER_CONTROL_CLUSTER_ID);
}

/*
 * Attribute BarrierPosition
 */
uint16_t encodeBarrierControlClusterReadBarrierPositionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x000A };
    READ_ATTRIBUTES("ReadBarrierControlBarrierPosition", BARRIER_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeBarrierControlClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadBarrierControlClusterRevision", BARRIER_CONTROL_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster Basic                                                       | 0x0000 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * ResetToFactoryDefaults                                            |   0x00 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * ZclVersion                                                        | 0x0000 |
| * PowerSource                                                       | 0x0007 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command ResetToFactoryDefaults
 */
uint16_t encodeBasicClusterResetToFactoryDefaultsCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "BasicResetToFactoryDefaults";
    COMMAND_HEADER(kName, BASIC_CLUSTER_ID, 0x00);
    COMMAND_FOOTER(kName);
}

uint16_t encodeBasicClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverBasicAttributes", BASIC_CLUSTER_ID);
}

/*
 * Attribute ZclVersion
 */
uint16_t encodeBasicClusterReadZclVersionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadBasicZclVersion", BASIC_CLUSTER_ID);
}

/*
 * Attribute PowerSource
 */
uint16_t encodeBasicClusterReadPowerSourceAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0007 };
    READ_ATTRIBUTES("ReadBasicPowerSource", BASIC_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeBasicClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadBasicClusterRevision", BASIC_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster ColorControl                                                | 0x0300 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * MoveColor                                                         |   0x08 |
| * MoveColorTemperature                                              |   0x4B |
| * MoveHue                                                           |   0x01 |
| * MoveSaturation                                                    |   0x04 |
| * MoveToColor                                                       |   0x07 |
| * MoveToColorTemperature                                            |   0x0A |
| * MoveToHue                                                         |   0x00 |
| * MoveToHueAndSaturation                                            |   0x06 |
| * MoveToSaturation                                                  |   0x03 |
| * StepColor                                                         |   0x09 |
| * StepColorTemperature                                              |   0x4C |
| * StepHue                                                           |   0x02 |
| * StepSaturation                                                    |   0x05 |
| * StopMoveStep                                                      |   0x47 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * CurrentHue                                                        | 0x0000 |
| * CurrentSaturation                                                 | 0x0001 |
| * RemainingTime                                                     | 0x0002 |
| * CurrentX                                                          | 0x0003 |
| * CurrentY                                                          | 0x0004 |
| * DriftCompensation                                                 | 0x0005 |
| * CompensationText                                                  | 0x0006 |
| * ColorTemperature                                                  | 0x0007 |
| * ColorMode                                                         | 0x0008 |
| * ColorControlOptions                                               | 0x000F |
| * NumberOfPrimaries                                                 | 0x0010 |
| * Primary1X                                                         | 0x0011 |
| * Primary1Y                                                         | 0x0012 |
| * Primary1Intensity                                                 | 0x0013 |
| * Primary2X                                                         | 0x0015 |
| * Primary2Y                                                         | 0x0016 |
| * Primary2Intensity                                                 | 0x0017 |
| * Primary3X                                                         | 0x0019 |
| * Primary3Y                                                         | 0x001A |
| * Primary3Intensity                                                 | 0x001B |
| * Primary4X                                                         | 0x0020 |
| * Primary4Y                                                         | 0x0021 |
| * Primary4Intensity                                                 | 0x0022 |
| * Primary5X                                                         | 0x0024 |
| * Primary5Y                                                         | 0x0025 |
| * Primary5Intensity                                                 | 0x0026 |
| * Primary6X                                                         | 0x0028 |
| * Primary6Y                                                         | 0x0029 |
| * Primary6Intensity                                                 | 0x002A |
| * WhitePointX                                                       | 0x0030 |
| * WhitePointY                                                       | 0x0031 |
| * ColorPointRX                                                      | 0x0032 |
| * ColorPointRY                                                      | 0x0033 |
| * ColorPointRIntensity                                              | 0x0034 |
| * ColorPointGX                                                      | 0x0036 |
| * ColorPointGY                                                      | 0x0037 |
| * ColorPointGIntensity                                              | 0x0038 |
| * ColorPointBX                                                      | 0x003A |
| * ColorPointBY                                                      | 0x003B |
| * ColorPointBIntensity                                              | 0x003C |
| * EnhancedCurrentHue                                                | 0x4000 |
| * EnhancedColorMode                                                 | 0x4001 |
| * ColorLoopActive                                                   | 0x4002 |
| * ColorLoopDirection                                                | 0x4003 |
| * ColorLoopTime                                                     | 0x4004 |
| * ColorCapabilities                                                 | 0x400A |
| * ColorTempPhysicalMin                                              | 0x400B |
| * ColorTempPhysicalMax                                              | 0x400C |
| * CoupleColorTempToLevelMinMireds                                   | 0x400D |
| * StartUpColorTemperatureMireds                                     | 0x4010 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command MoveColor
 */
uint16_t encodeColorControlClusterMoveColorCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                   int16_t rateX, int16_t rateY, uint8_t optionsMask, uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveColor";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x08);
    buf.Put16(static_cast<uint16_t>(rateX));
    buf.Put16(static_cast<uint16_t>(rateY));
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveColorTemperature
 */
uint16_t encodeColorControlClusterMoveColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length,
                                                              EndpointId destination_endpoint, uint8_t moveMode, uint16_t rate,
                                                              uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum,
                                                              uint8_t optionsMask, uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveColorTemperature";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x4B);
    buf.Put(moveMode);
    buf.Put16(rate);
    buf.Put16(colorTemperatureMinimum);
    buf.Put16(colorTemperatureMaximum);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveHue
 */
uint16_t encodeColorControlClusterMoveHueCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                 uint8_t moveMode, uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveHue";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x01);
    buf.Put(moveMode);
    buf.Put(rate);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveSaturation
 */
uint16_t encodeColorControlClusterMoveSaturationCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t moveMode, uint8_t rate, uint8_t optionsMask,
                                                        uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveSaturation";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x04);
    buf.Put(moveMode);
    buf.Put(rate);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToColor
 */
uint16_t encodeColorControlClusterMoveToColorCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                     uint16_t colorX, uint16_t colorY, uint16_t transitionTime, uint8_t optionsMask,
                                                     uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveToColor";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x07);
    buf.Put16(colorX);
    buf.Put16(colorY);
    buf.Put16(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToColorTemperature
 */
uint16_t encodeColorControlClusterMoveToColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length,
                                                                EndpointId destination_endpoint, uint16_t colorTemperature,
                                                                uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveToColorTemperature";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x0A);
    buf.Put16(colorTemperature);
    buf.Put16(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToHue
 */
uint16_t encodeColorControlClusterMoveToHueCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                   uint8_t hue, uint8_t direction, uint16_t transitionTime, uint8_t optionsMask,
                                                   uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveToHue";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x00);
    buf.Put(hue);
    buf.Put(direction);
    buf.Put16(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToHueAndSaturation
 */
uint16_t encodeColorControlClusterMoveToHueAndSaturationCommand(uint8_t * buffer, uint16_t buf_length,
                                                                EndpointId destination_endpoint, uint8_t hue, uint8_t saturation,
                                                                uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveToHueAndSaturation";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x06);
    buf.Put(hue);
    buf.Put(saturation);
    buf.Put16(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToSaturation
 */
uint16_t encodeColorControlClusterMoveToSaturationCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                          uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                                          uint8_t optionsOverride)
{
    const char * kName = "ColorControlMoveToSaturation";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x03);
    buf.Put(saturation);
    buf.Put16(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StepColor
 */
uint16_t encodeColorControlClusterStepColorCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                   int16_t stepX, int16_t stepY, uint16_t transitionTime, uint8_t optionsMask,
                                                   uint8_t optionsOverride)
{
    const char * kName = "ColorControlStepColor";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x09);
    buf.Put16(static_cast<uint16_t>(stepX));
    buf.Put16(static_cast<uint16_t>(stepY));
    buf.Put16(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StepColorTemperature
 */
uint16_t encodeColorControlClusterStepColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length,
                                                              EndpointId destination_endpoint, uint8_t stepMode, uint16_t stepSize,
                                                              uint16_t transitionTime, uint16_t colorTemperatureMinimum,
                                                              uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                              uint8_t optionsOverride)
{
    const char * kName = "ColorControlStepColorTemperature";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x4C);
    buf.Put(stepMode);
    buf.Put16(stepSize);
    buf.Put16(transitionTime);
    buf.Put16(colorTemperatureMinimum);
    buf.Put16(colorTemperatureMaximum);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StepHue
 */
uint16_t encodeColorControlClusterStepHueCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                 uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime, uint8_t optionsMask,
                                                 uint8_t optionsOverride)
{
    const char * kName = "ColorControlStepHue";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x02);
    buf.Put(stepMode);
    buf.Put(stepSize);
    buf.Put(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StepSaturation
 */
uint16_t encodeColorControlClusterStepSaturationCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                                                        uint8_t optionsMask, uint8_t optionsOverride)
{
    const char * kName = "ColorControlStepSaturation";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x05);
    buf.Put(stepMode);
    buf.Put(stepSize);
    buf.Put(transitionTime);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StopMoveStep
 */
uint16_t encodeColorControlClusterStopMoveStepCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    const char * kName = "ColorControlStopMoveStep";
    COMMAND_HEADER(kName, COLOR_CONTROL_CLUSTER_ID, 0x47);
    buf.Put(optionsMask);
    buf.Put(optionsOverride);
    COMMAND_FOOTER(kName);
}

uint16_t encodeColorControlClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverColorControlAttributes", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute CurrentHue
 */
uint16_t encodeColorControlClusterReadCurrentHueAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadColorControlCurrentHue", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterReportCurrentHueAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                            uint16_t min_interval, uint16_t max_interval, uint8_t change)
{
    uint16_t attr_id  = 0x0000;
    uint8_t attr_type = { 0x20 };
    REPORT_ATTRIBUTE("ReportColorControlCurrentHue", COLOR_CONTROL_CLUSTER_ID, true, change);
}

/*
 * Attribute CurrentSaturation
 */
uint16_t encodeColorControlClusterReadCurrentSaturationAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0001 };
    READ_ATTRIBUTES("ReadColorControlCurrentSaturation", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterReportCurrentSaturationAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                   EndpointId destination_endpoint, uint16_t min_interval,
                                                                   uint16_t max_interval, uint8_t change)
{
    uint16_t attr_id  = 0x0001;
    uint8_t attr_type = { 0x20 };
    REPORT_ATTRIBUTE("ReportColorControlCurrentSaturation", COLOR_CONTROL_CLUSTER_ID, true, change);
}

/*
 * Attribute RemainingTime
 */
uint16_t encodeColorControlClusterReadRemainingTimeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0002 };
    READ_ATTRIBUTES("ReadColorControlRemainingTime", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute CurrentX
 */
uint16_t encodeColorControlClusterReadCurrentXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0003 };
    READ_ATTRIBUTES("ReadColorControlCurrentX", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterReportCurrentXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                          uint16_t min_interval, uint16_t max_interval, uint16_t change)
{
    uint16_t attr_id  = 0x0003;
    uint8_t attr_type = { 0x21 };
    REPORT_ATTRIBUTE("ReportColorControlCurrentX", COLOR_CONTROL_CLUSTER_ID, true, change);
}

/*
 * Attribute CurrentY
 */
uint16_t encodeColorControlClusterReadCurrentYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0004 };
    READ_ATTRIBUTES("ReadColorControlCurrentY", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterReportCurrentYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                          uint16_t min_interval, uint16_t max_interval, uint16_t change)
{
    uint16_t attr_id  = 0x0004;
    uint8_t attr_type = { 0x21 };
    REPORT_ATTRIBUTE("ReportColorControlCurrentY", COLOR_CONTROL_CLUSTER_ID, true, change);
}

/*
 * Attribute DriftCompensation
 */
uint16_t encodeColorControlClusterReadDriftCompensationAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0005 };
    READ_ATTRIBUTES("ReadColorControlDriftCompensation", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute CompensationText
 */
uint16_t encodeColorControlClusterReadCompensationTextAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0006 };
    READ_ATTRIBUTES("ReadColorControlCompensationText", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorTemperature
 */
uint16_t encodeColorControlClusterReadColorTemperatureAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0007 };
    READ_ATTRIBUTES("ReadColorControlColorTemperature", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterReportColorTemperatureAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                  EndpointId destination_endpoint, uint16_t min_interval,
                                                                  uint16_t max_interval, uint16_t change)
{
    uint16_t attr_id  = 0x0007;
    uint8_t attr_type = { 0x21 };
    REPORT_ATTRIBUTE("ReportColorControlColorTemperature", COLOR_CONTROL_CLUSTER_ID, true, change);
}

/*
 * Attribute ColorMode
 */
uint16_t encodeColorControlClusterReadColorModeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0008 };
    READ_ATTRIBUTES("ReadColorControlColorMode", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorControlOptions
 */
uint16_t encodeColorControlClusterReadColorControlOptionsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                   EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x000F };
    READ_ATTRIBUTES("ReadColorControlColorControlOptions", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorControlOptionsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint, uint8_t colorControlOptions)
{
    uint16_t attr_id  = 0x000F;
    uint8_t attr_type = { 0x18 };
    WRITE_ATTRIBUTE("WriteColorControlColorControlOptions", COLOR_CONTROL_CLUSTER_ID, colorControlOptions);
}

/*
 * Attribute NumberOfPrimaries
 */
uint16_t encodeColorControlClusterReadNumberOfPrimariesAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0010 };
    READ_ATTRIBUTES("ReadColorControlNumberOfPrimaries", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary1X
 */
uint16_t encodeColorControlClusterReadPrimary1XAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0011 };
    READ_ATTRIBUTES("ReadColorControlPrimary1X", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary1Y
 */
uint16_t encodeColorControlClusterReadPrimary1YAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0012 };
    READ_ATTRIBUTES("ReadColorControlPrimary1Y", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary1Intensity
 */
uint16_t encodeColorControlClusterReadPrimary1IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0013 };
    READ_ATTRIBUTES("ReadColorControlPrimary1Intensity", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary2X
 */
uint16_t encodeColorControlClusterReadPrimary2XAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0015 };
    READ_ATTRIBUTES("ReadColorControlPrimary2X", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary2Y
 */
uint16_t encodeColorControlClusterReadPrimary2YAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0016 };
    READ_ATTRIBUTES("ReadColorControlPrimary2Y", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary2Intensity
 */
uint16_t encodeColorControlClusterReadPrimary2IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0017 };
    READ_ATTRIBUTES("ReadColorControlPrimary2Intensity", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary3X
 */
uint16_t encodeColorControlClusterReadPrimary3XAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0019 };
    READ_ATTRIBUTES("ReadColorControlPrimary3X", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary3Y
 */
uint16_t encodeColorControlClusterReadPrimary3YAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x001A };
    READ_ATTRIBUTES("ReadColorControlPrimary3Y", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary3Intensity
 */
uint16_t encodeColorControlClusterReadPrimary3IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x001B };
    READ_ATTRIBUTES("ReadColorControlPrimary3Intensity", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary4X
 */
uint16_t encodeColorControlClusterReadPrimary4XAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0020 };
    READ_ATTRIBUTES("ReadColorControlPrimary4X", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary4Y
 */
uint16_t encodeColorControlClusterReadPrimary4YAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0021 };
    READ_ATTRIBUTES("ReadColorControlPrimary4Y", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary4Intensity
 */
uint16_t encodeColorControlClusterReadPrimary4IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0022 };
    READ_ATTRIBUTES("ReadColorControlPrimary4Intensity", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary5X
 */
uint16_t encodeColorControlClusterReadPrimary5XAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0024 };
    READ_ATTRIBUTES("ReadColorControlPrimary5X", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary5Y
 */
uint16_t encodeColorControlClusterReadPrimary5YAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0025 };
    READ_ATTRIBUTES("ReadColorControlPrimary5Y", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary5Intensity
 */
uint16_t encodeColorControlClusterReadPrimary5IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0026 };
    READ_ATTRIBUTES("ReadColorControlPrimary5Intensity", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary6X
 */
uint16_t encodeColorControlClusterReadPrimary6XAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0028 };
    READ_ATTRIBUTES("ReadColorControlPrimary6X", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary6Y
 */
uint16_t encodeColorControlClusterReadPrimary6YAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0029 };
    READ_ATTRIBUTES("ReadColorControlPrimary6Y", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute Primary6Intensity
 */
uint16_t encodeColorControlClusterReadPrimary6IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x002A };
    READ_ATTRIBUTES("ReadColorControlPrimary6Intensity", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute WhitePointX
 */
uint16_t encodeColorControlClusterReadWhitePointXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0030 };
    READ_ATTRIBUTES("ReadColorControlWhitePointX", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteWhitePointXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                            uint16_t whitePointX)
{
    uint16_t attr_id  = 0x0030;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlWhitePointX", COLOR_CONTROL_CLUSTER_ID, whitePointX);
}

/*
 * Attribute WhitePointY
 */
uint16_t encodeColorControlClusterReadWhitePointYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0031 };
    READ_ATTRIBUTES("ReadColorControlWhitePointY", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteWhitePointYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                            uint16_t whitePointY)
{
    uint16_t attr_id  = 0x0031;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlWhitePointY", COLOR_CONTROL_CLUSTER_ID, whitePointY);
}

/*
 * Attribute ColorPointRX
 */
uint16_t encodeColorControlClusterReadColorPointRXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0032 };
    READ_ATTRIBUTES("ReadColorControlColorPointRX", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointRXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                             uint16_t colorPointRX)
{
    uint16_t attr_id  = 0x0032;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointRX", COLOR_CONTROL_CLUSTER_ID, colorPointRX);
}

/*
 * Attribute ColorPointRY
 */
uint16_t encodeColorControlClusterReadColorPointRYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0033 };
    READ_ATTRIBUTES("ReadColorControlColorPointRY", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointRYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                             uint16_t colorPointRY)
{
    uint16_t attr_id  = 0x0033;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointRY", COLOR_CONTROL_CLUSTER_ID, colorPointRY);
}

/*
 * Attribute ColorPointRIntensity
 */
uint16_t encodeColorControlClusterReadColorPointRIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0034 };
    READ_ATTRIBUTES("ReadColorControlColorPointRIntensity", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointRIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     EndpointId destination_endpoint, uint8_t colorPointRIntensity)
{
    uint16_t attr_id  = 0x0034;
    uint8_t attr_type = { 0x20 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointRIntensity", COLOR_CONTROL_CLUSTER_ID, colorPointRIntensity);
}

/*
 * Attribute ColorPointGX
 */
uint16_t encodeColorControlClusterReadColorPointGXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0036 };
    READ_ATTRIBUTES("ReadColorControlColorPointGX", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointGXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                             uint16_t colorPointGX)
{
    uint16_t attr_id  = 0x0036;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointGX", COLOR_CONTROL_CLUSTER_ID, colorPointGX);
}

/*
 * Attribute ColorPointGY
 */
uint16_t encodeColorControlClusterReadColorPointGYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0037 };
    READ_ATTRIBUTES("ReadColorControlColorPointGY", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointGYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                             uint16_t colorPointGY)
{
    uint16_t attr_id  = 0x0037;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointGY", COLOR_CONTROL_CLUSTER_ID, colorPointGY);
}

/*
 * Attribute ColorPointGIntensity
 */
uint16_t encodeColorControlClusterReadColorPointGIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0038 };
    READ_ATTRIBUTES("ReadColorControlColorPointGIntensity", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointGIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     EndpointId destination_endpoint, uint8_t colorPointGIntensity)
{
    uint16_t attr_id  = 0x0038;
    uint8_t attr_type = { 0x20 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointGIntensity", COLOR_CONTROL_CLUSTER_ID, colorPointGIntensity);
}

/*
 * Attribute ColorPointBX
 */
uint16_t encodeColorControlClusterReadColorPointBXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x003A };
    READ_ATTRIBUTES("ReadColorControlColorPointBX", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointBXAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                             uint16_t colorPointBX)
{
    uint16_t attr_id  = 0x003A;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointBX", COLOR_CONTROL_CLUSTER_ID, colorPointBX);
}

/*
 * Attribute ColorPointBY
 */
uint16_t encodeColorControlClusterReadColorPointBYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x003B };
    READ_ATTRIBUTES("ReadColorControlColorPointBY", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointBYAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                             uint16_t colorPointBY)
{
    uint16_t attr_id  = 0x003B;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointBY", COLOR_CONTROL_CLUSTER_ID, colorPointBY);
}

/*
 * Attribute ColorPointBIntensity
 */
uint16_t encodeColorControlClusterReadColorPointBIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x003C };
    READ_ATTRIBUTES("ReadColorControlColorPointBIntensity", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteColorPointBIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     EndpointId destination_endpoint, uint8_t colorPointBIntensity)
{
    uint16_t attr_id  = 0x003C;
    uint8_t attr_type = { 0x20 };
    WRITE_ATTRIBUTE("WriteColorControlColorPointBIntensity", COLOR_CONTROL_CLUSTER_ID, colorPointBIntensity);
}

/*
 * Attribute EnhancedCurrentHue
 */
uint16_t encodeColorControlClusterReadEnhancedCurrentHueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                  EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x4000 };
    READ_ATTRIBUTES("ReadColorControlEnhancedCurrentHue", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute EnhancedColorMode
 */
uint16_t encodeColorControlClusterReadEnhancedColorModeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x4001 };
    READ_ATTRIBUTES("ReadColorControlEnhancedColorMode", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorLoopActive
 */
uint16_t encodeColorControlClusterReadColorLoopActiveAttribute(uint8_t * buffer, uint16_t buf_length,
                                                               EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x4002 };
    READ_ATTRIBUTES("ReadColorControlColorLoopActive", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorLoopDirection
 */
uint16_t encodeColorControlClusterReadColorLoopDirectionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                  EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x4003 };
    READ_ATTRIBUTES("ReadColorControlColorLoopDirection", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorLoopTime
 */
uint16_t encodeColorControlClusterReadColorLoopTimeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x4004 };
    READ_ATTRIBUTES("ReadColorControlColorLoopTime", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorCapabilities
 */
uint16_t encodeColorControlClusterReadColorCapabilitiesAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x400A };
    READ_ATTRIBUTES("ReadColorControlColorCapabilities", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorTempPhysicalMin
 */
uint16_t encodeColorControlClusterReadColorTempPhysicalMinAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x400B };
    READ_ATTRIBUTES("ReadColorControlColorTempPhysicalMin", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute ColorTempPhysicalMax
 */
uint16_t encodeColorControlClusterReadColorTempPhysicalMaxAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x400C };
    READ_ATTRIBUTES("ReadColorControlColorTempPhysicalMax", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute CoupleColorTempToLevelMinMireds
 */
uint16_t encodeColorControlClusterReadCoupleColorTempToLevelMinMiredsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                               EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x400D };
    READ_ATTRIBUTES("ReadColorControlCoupleColorTempToLevelMinMireds", COLOR_CONTROL_CLUSTER_ID);
}

/*
 * Attribute StartUpColorTemperatureMireds
 */
uint16_t encodeColorControlClusterReadStartUpColorTemperatureMiredsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                             EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x4010 };
    READ_ATTRIBUTES("ReadColorControlStartUpColorTemperatureMireds", COLOR_CONTROL_CLUSTER_ID);
}

uint16_t encodeColorControlClusterWriteStartUpColorTemperatureMiredsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                              EndpointId destination_endpoint,
                                                                              uint16_t startUpColorTemperatureMireds)
{
    uint16_t attr_id  = 0x4010;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteColorControlStartUpColorTemperatureMireds", COLOR_CONTROL_CLUSTER_ID, startUpColorTemperatureMireds);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeColorControlClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                               EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadColorControlClusterRevision", COLOR_CONTROL_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster DoorLock                                                    | 0x0101 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * ClearAllPins                                                      |   0x08 |
| * ClearAllRfids                                                     |   0x19 |
| * ClearHolidaySchedule                                              |   0x13 |
| * ClearPin                                                          |   0x07 |
| * ClearRfid                                                         |   0x18 |
| * ClearWeekdaySchedule                                              |   0x0D |
| * ClearYeardaySchedule                                              |   0x10 |
| * GetHolidaySchedule                                                |   0x12 |
| * GetLogRecord                                                      |   0x04 |
| * GetPin                                                            |   0x06 |
| * GetRfid                                                           |   0x17 |
| * GetUserType                                                       |   0x15 |
| * GetWeekdaySchedule                                                |   0x0C |
| * GetYeardaySchedule                                                |   0x0F |
| * LockDoor                                                          |   0x00 |
| * SetHolidaySchedule                                                |   0x11 |
| * SetPin                                                            |   0x05 |
| * SetRfid                                                           |   0x16 |
| * SetUserType                                                       |   0x14 |
| * SetWeekdaySchedule                                                |   0x0B |
| * SetYeardaySchedule                                                |   0x0E |
| * UnlockDoor                                                        |   0x01 |
| * UnlockWithTimeout                                                 |   0x03 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * LockState                                                         | 0x0000 |
| * LockType                                                          | 0x0001 |
| * ActuatorEnabled                                                   | 0x0002 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command ClearAllPins
 */
uint16_t encodeDoorLockClusterClearAllPinsCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "DoorLockClearAllPins";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x08);
    COMMAND_FOOTER(kName);
}

/*
 * Command ClearAllRfids
 */
uint16_t encodeDoorLockClusterClearAllRfidsCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "DoorLockClearAllRfids";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x19);
    COMMAND_FOOTER(kName);
}

/*
 * Command ClearHolidaySchedule
 */
uint16_t encodeDoorLockClusterClearHolidayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                          uint8_t scheduleId)
{
    const char * kName = "DoorLockClearHolidaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x13);
    buf.Put(scheduleId);
    COMMAND_FOOTER(kName);
}

/*
 * Command ClearPin
 */
uint16_t encodeDoorLockClusterClearPinCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                              uint16_t userId)
{
    const char * kName = "DoorLockClearPin";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x07);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command ClearRfid
 */
uint16_t encodeDoorLockClusterClearRfidCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                               uint16_t userId)
{
    const char * kName = "DoorLockClearRfid";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x18);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command ClearWeekdaySchedule
 */
uint16_t encodeDoorLockClusterClearWeekdayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                          uint8_t scheduleId, uint16_t userId)
{
    const char * kName = "DoorLockClearWeekdaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x0D);
    buf.Put(scheduleId);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command ClearYeardaySchedule
 */
uint16_t encodeDoorLockClusterClearYeardayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                          uint8_t scheduleId, uint16_t userId)
{
    const char * kName = "DoorLockClearYeardaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x10);
    buf.Put(scheduleId);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetHolidaySchedule
 */
uint16_t encodeDoorLockClusterGetHolidayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t scheduleId)
{
    const char * kName = "DoorLockGetHolidaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x12);
    buf.Put(scheduleId);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetLogRecord
 */
uint16_t encodeDoorLockClusterGetLogRecordCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                  uint16_t logIndex)
{
    const char * kName = "DoorLockGetLogRecord";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x04);
    buf.Put16(logIndex);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetPin
 */
uint16_t encodeDoorLockClusterGetPinCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint, uint16_t userId)
{
    const char * kName = "DoorLockGetPin";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x06);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetRfid
 */
uint16_t encodeDoorLockClusterGetRfidCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                             uint16_t userId)
{
    const char * kName = "DoorLockGetRfid";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x17);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetUserType
 */
uint16_t encodeDoorLockClusterGetUserTypeCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                 uint16_t userId)
{
    const char * kName = "DoorLockGetUserType";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x15);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetWeekdaySchedule
 */
uint16_t encodeDoorLockClusterGetWeekdayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t scheduleId, uint16_t userId)
{
    const char * kName = "DoorLockGetWeekdaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x0C);
    buf.Put(scheduleId);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetYeardaySchedule
 */
uint16_t encodeDoorLockClusterGetYeardayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t scheduleId, uint16_t userId)
{
    const char * kName = "DoorLockGetYeardaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x0F);
    buf.Put(scheduleId);
    buf.Put16(userId);
    COMMAND_FOOTER(kName);
}

/*
 * Command LockDoor
 */
uint16_t encodeDoorLockClusterLockDoorCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint, char * pin)
{
    const char * kName = "DoorLockLockDoor";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x00);
    COMMAND_INSERT_STRING(kName, pin);
    COMMAND_FOOTER(kName);
}

/*
 * Command SetHolidaySchedule
 */
uint16_t encodeDoorLockClusterSetHolidayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t scheduleId, uint32_t localStartTime, uint32_t localEndTime,
                                                        uint8_t operatingModeDuringHoliday)
{
    const char * kName = "DoorLockSetHolidaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x11);
    buf.Put(scheduleId);
    buf.Put32(localStartTime);
    buf.Put32(localEndTime);
    buf.Put(operatingModeDuringHoliday);
    COMMAND_FOOTER(kName);
}

/*
 * Command SetPin
 */
uint16_t encodeDoorLockClusterSetPinCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint, uint16_t userId,
                                            uint8_t userStatus, uint8_t userType, char * pin)
{
    const char * kName = "DoorLockSetPin";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x05);
    buf.Put16(userId);
    buf.Put(userStatus);
    buf.Put(userType);
    COMMAND_INSERT_STRING(kName, pin);
    COMMAND_FOOTER(kName);
}

/*
 * Command SetRfid
 */
uint16_t encodeDoorLockClusterSetRfidCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                             uint16_t userId, uint8_t userStatus, uint8_t userType, char * id)
{
    const char * kName = "DoorLockSetRfid";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x16);
    buf.Put16(userId);
    buf.Put(userStatus);
    buf.Put(userType);
    COMMAND_INSERT_STRING(kName, id);
    COMMAND_FOOTER(kName);
}

/*
 * Command SetUserType
 */
uint16_t encodeDoorLockClusterSetUserTypeCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                 uint16_t userId, uint8_t userType)
{
    const char * kName = "DoorLockSetUserType";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x14);
    buf.Put16(userId);
    buf.Put(userType);
    COMMAND_FOOTER(kName);
}

/*
 * Command SetWeekdaySchedule
 */
uint16_t encodeDoorLockClusterSetWeekdayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t scheduleId, uint16_t userId, uint8_t daysMask, uint8_t startHour,
                                                        uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    const char * kName = "DoorLockSetWeekdaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x0B);
    buf.Put(scheduleId);
    buf.Put16(userId);
    buf.Put(daysMask);
    buf.Put(startHour);
    buf.Put(startMinute);
    buf.Put(endHour);
    buf.Put(endMinute);
    COMMAND_FOOTER(kName);
}

/*
 * Command SetYeardaySchedule
 */
uint16_t encodeDoorLockClusterSetYeardayScheduleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                        uint8_t scheduleId, uint16_t userId, uint32_t localStartTime,
                                                        uint32_t localEndTime)
{
    const char * kName = "DoorLockSetYeardaySchedule";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x0E);
    buf.Put(scheduleId);
    buf.Put16(userId);
    buf.Put32(localStartTime);
    buf.Put32(localEndTime);
    COMMAND_FOOTER(kName);
}

/*
 * Command UnlockDoor
 */
uint16_t encodeDoorLockClusterUnlockDoorCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint, char * pin)
{
    const char * kName = "DoorLockUnlockDoor";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x01);
    COMMAND_INSERT_STRING(kName, pin);
    COMMAND_FOOTER(kName);
}

/*
 * Command UnlockWithTimeout
 */
uint16_t encodeDoorLockClusterUnlockWithTimeoutCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                       uint16_t timeoutInSeconds, char * pin)
{
    const char * kName = "DoorLockUnlockWithTimeout";
    COMMAND_HEADER(kName, DOOR_LOCK_CLUSTER_ID, 0x03);
    buf.Put16(timeoutInSeconds);
    COMMAND_INSERT_STRING(kName, pin);
    COMMAND_FOOTER(kName);
}

uint16_t encodeDoorLockClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverDoorLockAttributes", DOOR_LOCK_CLUSTER_ID);
}

/*
 * Attribute LockState
 */
uint16_t encodeDoorLockClusterReadLockStateAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadDoorLockLockState", DOOR_LOCK_CLUSTER_ID);
}

uint16_t encodeDoorLockClusterReportLockStateAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                       uint16_t min_interval, uint16_t max_interval)
{
    uint16_t attr_id  = 0x0000;
    uint8_t attr_type = { 0x30 };
    REPORT_ATTRIBUTE("ReportDoorLockLockState", DOOR_LOCK_CLUSTER_ID, false, 0);
}

/*
 * Attribute LockType
 */
uint16_t encodeDoorLockClusterReadLockTypeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0001 };
    READ_ATTRIBUTES("ReadDoorLockLockType", DOOR_LOCK_CLUSTER_ID);
}

/*
 * Attribute ActuatorEnabled
 */
uint16_t encodeDoorLockClusterReadActuatorEnabledAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0002 };
    READ_ATTRIBUTES("ReadDoorLockActuatorEnabled", DOOR_LOCK_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeDoorLockClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadDoorLockClusterRevision", DOOR_LOCK_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster Groups                                                      | 0x0004 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * AddGroup                                                          |   0x00 |
| * AddGroupIfIdentifying                                             |   0x05 |
| * GetGroupMembership                                                |   0x02 |
| * RemoveAllGroups                                                   |   0x04 |
| * RemoveGroup                                                       |   0x03 |
| * ViewGroup                                                         |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * NameSupport                                                       | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command AddGroup
 */
uint16_t encodeGroupsClusterAddGroupCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                            uint16_t groupId, char * groupName)
{
    const char * kName = "GroupsAddGroup";
    COMMAND_HEADER(kName, GROUPS_CLUSTER_ID, 0x00);
    buf.Put16(groupId);
    COMMAND_INSERT_STRING(kName, groupName);
    COMMAND_FOOTER(kName);
}

/*
 * Command AddGroupIfIdentifying
 */
uint16_t encodeGroupsClusterAddGroupIfIdentifyingCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                         uint16_t groupId, char * groupName)
{
    const char * kName = "GroupsAddGroupIfIdentifying";
    COMMAND_HEADER(kName, GROUPS_CLUSTER_ID, 0x05);
    buf.Put16(groupId);
    COMMAND_INSERT_STRING(kName, groupName);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetGroupMembership
 */
uint16_t encodeGroupsClusterGetGroupMembershipCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                      uint8_t groupCount, uint16_t groupList)
{
    const char * kName = "GroupsGetGroupMembership";
    COMMAND_HEADER(kName, GROUPS_CLUSTER_ID, 0x02);
    buf.Put(groupCount);
    buf.Put16(groupList);
    COMMAND_FOOTER(kName);
}

/*
 * Command RemoveAllGroups
 */
uint16_t encodeGroupsClusterRemoveAllGroupsCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "GroupsRemoveAllGroups";
    COMMAND_HEADER(kName, GROUPS_CLUSTER_ID, 0x04);
    COMMAND_FOOTER(kName);
}

/*
 * Command RemoveGroup
 */
uint16_t encodeGroupsClusterRemoveGroupCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                               uint16_t groupId)
{
    const char * kName = "GroupsRemoveGroup";
    COMMAND_HEADER(kName, GROUPS_CLUSTER_ID, 0x03);
    buf.Put16(groupId);
    COMMAND_FOOTER(kName);
}

/*
 * Command ViewGroup
 */
uint16_t encodeGroupsClusterViewGroupCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                             uint16_t groupId)
{
    const char * kName = "GroupsViewGroup";
    COMMAND_HEADER(kName, GROUPS_CLUSTER_ID, 0x01);
    buf.Put16(groupId);
    COMMAND_FOOTER(kName);
}

uint16_t encodeGroupsClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverGroupsAttributes", GROUPS_CLUSTER_ID);
}

/*
 * Attribute NameSupport
 */
uint16_t encodeGroupsClusterReadNameSupportAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadGroupsNameSupport", GROUPS_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeGroupsClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadGroupsClusterRevision", GROUPS_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster IasZone                                                     | 0x0500 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * ZoneState                                                         | 0x0000 |
| * ZoneType                                                          | 0x0001 |
| * ZoneStatus                                                        | 0x0002 |
| * IasCieAddress                                                     | 0x0010 |
| * ZoneId                                                            | 0x0011 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

uint16_t encodeIasZoneClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverIasZoneAttributes", IAS_ZONE_CLUSTER_ID);
}

/*
 * Attribute ZoneState
 */
uint16_t encodeIasZoneClusterReadZoneStateAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadIasZoneZoneState", IAS_ZONE_CLUSTER_ID);
}

/*
 * Attribute ZoneType
 */
uint16_t encodeIasZoneClusterReadZoneTypeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0001 };
    READ_ATTRIBUTES("ReadIasZoneZoneType", IAS_ZONE_CLUSTER_ID);
}

/*
 * Attribute ZoneStatus
 */
uint16_t encodeIasZoneClusterReadZoneStatusAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0002 };
    READ_ATTRIBUTES("ReadIasZoneZoneStatus", IAS_ZONE_CLUSTER_ID);
}

/*
 * Attribute IasCieAddress
 */
uint16_t encodeIasZoneClusterReadIasCieAddressAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0010 };
    READ_ATTRIBUTES("ReadIasZoneIasCieAddress", IAS_ZONE_CLUSTER_ID);
}

uint16_t encodeIasZoneClusterWriteIasCieAddressAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                         uint64_t iasCieAddress)
{
    uint16_t attr_id  = 0x0010;
    uint8_t attr_type = { 0xF0 };
    WRITE_ATTRIBUTE("WriteIasZoneIasCieAddress", IAS_ZONE_CLUSTER_ID, iasCieAddress);
}

/*
 * Attribute ZoneId
 */
uint16_t encodeIasZoneClusterReadZoneIdAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0011 };
    READ_ATTRIBUTES("ReadIasZoneZoneId", IAS_ZONE_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeIasZoneClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadIasZoneClusterRevision", IAS_ZONE_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster Identify                                                    | 0x0003 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Identify                                                          |   0x00 |
| * IdentifyQuery                                                     |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * IdentifyTime                                                      | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command Identify
 */
uint16_t encodeIdentifyClusterIdentifyCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                              uint16_t identifyTime)
{
    const char * kName = "IdentifyIdentify";
    COMMAND_HEADER(kName, IDENTIFY_CLUSTER_ID, 0x00);
    buf.Put16(identifyTime);
    COMMAND_FOOTER(kName);
}

/*
 * Command IdentifyQuery
 */
uint16_t encodeIdentifyClusterIdentifyQueryCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "IdentifyIdentifyQuery";
    COMMAND_HEADER(kName, IDENTIFY_CLUSTER_ID, 0x01);
    COMMAND_FOOTER(kName);
}

uint16_t encodeIdentifyClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverIdentifyAttributes", IDENTIFY_CLUSTER_ID);
}

/*
 * Attribute IdentifyTime
 */
uint16_t encodeIdentifyClusterReadIdentifyTimeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadIdentifyIdentifyTime", IDENTIFY_CLUSTER_ID);
}

uint16_t encodeIdentifyClusterWriteIdentifyTimeAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                         uint16_t identifyTime)
{
    uint16_t attr_id  = 0x0000;
    uint8_t attr_type = { 0x21 };
    WRITE_ATTRIBUTE("WriteIdentifyIdentifyTime", IDENTIFY_CLUSTER_ID, identifyTime);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeIdentifyClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadIdentifyClusterRevision", IDENTIFY_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster LevelControl                                                | 0x0008 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Move                                                              |   0x01 |
| * MoveToLevel                                                       |   0x00 |
| * MoveToLevelWithOnOff                                              |   0x04 |
| * MoveWithOnOff                                                     |   0x05 |
| * Step                                                              |   0x02 |
| * StepWithOnOff                                                     |   0x06 |
| * Stop                                                              |   0x03 |
| * StopWithOnOff                                                     |   0x07 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * CurrentLevel                                                      | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command Move
 */
uint16_t encodeLevelControlClusterMoveCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                              uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride)
{
    const char * kName = "LevelControlMove";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x01);
    buf.Put(moveMode);
    buf.Put(rate);
    buf.Put(optionMask);
    buf.Put(optionOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToLevel
 */
uint16_t encodeLevelControlClusterMoveToLevelCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                     uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                                     uint8_t optionOverride)
{
    const char * kName = "LevelControlMoveToLevel";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x00);
    buf.Put(level);
    buf.Put16(transitionTime);
    buf.Put(optionMask);
    buf.Put(optionOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveToLevelWithOnOff
 */
uint16_t encodeLevelControlClusterMoveToLevelWithOnOffCommand(uint8_t * buffer, uint16_t buf_length,
                                                              EndpointId destination_endpoint, uint8_t level,
                                                              uint16_t transitionTime)
{
    const char * kName = "LevelControlMoveToLevelWithOnOff";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x04);
    buf.Put(level);
    buf.Put16(transitionTime);
    COMMAND_FOOTER(kName);
}

/*
 * Command MoveWithOnOff
 */
uint16_t encodeLevelControlClusterMoveWithOnOffCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                       uint8_t moveMode, uint8_t rate)
{
    const char * kName = "LevelControlMoveWithOnOff";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x05);
    buf.Put(moveMode);
    buf.Put(rate);
    COMMAND_FOOTER(kName);
}

/*
 * Command Step
 */
uint16_t encodeLevelControlClusterStepCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                              uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                              uint8_t optionOverride)
{
    const char * kName = "LevelControlStep";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x02);
    buf.Put(stepMode);
    buf.Put(stepSize);
    buf.Put16(transitionTime);
    buf.Put(optionMask);
    buf.Put(optionOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StepWithOnOff
 */
uint16_t encodeLevelControlClusterStepWithOnOffCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                       uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime)
{
    const char * kName = "LevelControlStepWithOnOff";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x06);
    buf.Put(stepMode);
    buf.Put(stepSize);
    buf.Put16(transitionTime);
    COMMAND_FOOTER(kName);
}

/*
 * Command Stop
 */
uint16_t encodeLevelControlClusterStopCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                              uint8_t optionMask, uint8_t optionOverride)
{
    const char * kName = "LevelControlStop";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x03);
    buf.Put(optionMask);
    buf.Put(optionOverride);
    COMMAND_FOOTER(kName);
}

/*
 * Command StopWithOnOff
 */
uint16_t encodeLevelControlClusterStopWithOnOffCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "LevelControlStopWithOnOff";
    COMMAND_HEADER(kName, LEVEL_CONTROL_CLUSTER_ID, 0x07);
    COMMAND_FOOTER(kName);
}

uint16_t encodeLevelControlClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverLevelControlAttributes", LEVEL_CONTROL_CLUSTER_ID);
}

/*
 * Attribute CurrentLevel
 */
uint16_t encodeLevelControlClusterReadCurrentLevelAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadLevelControlCurrentLevel", LEVEL_CONTROL_CLUSTER_ID);
}

uint16_t encodeLevelControlClusterReportCurrentLevelAttribute(uint8_t * buffer, uint16_t buf_length,
                                                              EndpointId destination_endpoint, uint16_t min_interval,
                                                              uint16_t max_interval, uint8_t change)
{
    uint16_t attr_id  = 0x0000;
    uint8_t attr_type = { 0x20 };
    REPORT_ATTRIBUTE("ReportLevelControlCurrentLevel", LEVEL_CONTROL_CLUSTER_ID, true, change);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeLevelControlClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                               EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadLevelControlClusterRevision", LEVEL_CONTROL_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster OnOff                                                       | 0x0006 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Off                                                               |   0x00 |
| * On                                                                |   0x01 |
| * Toggle                                                            |   0x02 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * OnOff                                                             | 0x0000 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command Off
 */
uint16_t encodeOnOffClusterOffCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "OnOffOff";
    COMMAND_HEADER(kName, ON_OFF_CLUSTER_ID, 0x00);
    COMMAND_FOOTER(kName);
}

/*
 * Command On
 */
uint16_t encodeOnOffClusterOnCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "OnOffOn";
    COMMAND_HEADER(kName, ON_OFF_CLUSTER_ID, 0x01);
    COMMAND_FOOTER(kName);
}

/*
 * Command Toggle
 */
uint16_t encodeOnOffClusterToggleCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    const char * kName = "OnOffToggle";
    COMMAND_HEADER(kName, ON_OFF_CLUSTER_ID, 0x02);
    COMMAND_FOOTER(kName);
}

uint16_t encodeOnOffClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverOnOffAttributes", ON_OFF_CLUSTER_ID);
}

/*
 * Attribute OnOff
 */
uint16_t encodeOnOffClusterReadOnOffAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadOnOffOnOff", ON_OFF_CLUSTER_ID);
}

uint16_t encodeOnOffClusterReportOnOffAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                uint16_t min_interval, uint16_t max_interval)
{
    uint16_t attr_id  = 0x0000;
    uint8_t attr_type = { 0x10 };
    REPORT_ATTRIBUTE("ReportOnOffOnOff", ON_OFF_CLUSTER_ID, false, 0);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeOnOffClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadOnOffClusterRevision", ON_OFF_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster Scenes                                                      | 0x0005 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * AddScene                                                          |   0x00 |
| * GetSceneMembership                                                |   0x06 |
| * RecallScene                                                       |   0x05 |
| * RemoveAllScenes                                                   |   0x03 |
| * RemoveScene                                                       |   0x02 |
| * StoreScene                                                        |   0x04 |
| * ViewScene                                                         |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * SceneCount                                                        | 0x0000 |
| * CurrentScene                                                      | 0x0001 |
| * CurrentGroup                                                      | 0x0002 |
| * SceneValid                                                        | 0x0003 |
| * NameSupport                                                       | 0x0004 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command AddScene
 */
uint16_t encodeScenesClusterAddSceneCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                            uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, char * sceneName,
                                            chip::ClusterId clusterId, uint8_t length, uint8_t value)
{
    const char * kName = "ScenesAddScene";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x00);
    buf.Put16(groupId);
    buf.Put(sceneId);
    buf.Put16(transitionTime);
    COMMAND_INSERT_STRING(kName, sceneName);
    buf.Put16(clusterId);
    buf.Put(length);
    buf.Put(value);
    COMMAND_FOOTER(kName);
}

/*
 * Command GetSceneMembership
 */
uint16_t encodeScenesClusterGetSceneMembershipCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                      uint16_t groupId)
{
    const char * kName = "ScenesGetSceneMembership";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x06);
    buf.Put16(groupId);
    COMMAND_FOOTER(kName);
}

/*
 * Command RecallScene
 */
uint16_t encodeScenesClusterRecallSceneCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                               uint16_t groupId, uint8_t sceneId, uint16_t transitionTime)
{
    const char * kName = "ScenesRecallScene";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x05);
    buf.Put16(groupId);
    buf.Put(sceneId);
    buf.Put16(transitionTime);
    COMMAND_FOOTER(kName);
}

/*
 * Command RemoveAllScenes
 */
uint16_t encodeScenesClusterRemoveAllScenesCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                                   uint16_t groupId)
{
    const char * kName = "ScenesRemoveAllScenes";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x03);
    buf.Put16(groupId);
    COMMAND_FOOTER(kName);
}

/*
 * Command RemoveScene
 */
uint16_t encodeScenesClusterRemoveSceneCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                               uint16_t groupId, uint8_t sceneId)
{
    const char * kName = "ScenesRemoveScene";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x02);
    buf.Put16(groupId);
    buf.Put(sceneId);
    COMMAND_FOOTER(kName);
}

/*
 * Command StoreScene
 */
uint16_t encodeScenesClusterStoreSceneCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                              uint16_t groupId, uint8_t sceneId)
{
    const char * kName = "ScenesStoreScene";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x04);
    buf.Put16(groupId);
    buf.Put(sceneId);
    COMMAND_FOOTER(kName);
}

/*
 * Command ViewScene
 */
uint16_t encodeScenesClusterViewSceneCommand(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint,
                                             uint16_t groupId, uint8_t sceneId)
{
    const char * kName = "ScenesViewScene";
    COMMAND_HEADER(kName, SCENES_CLUSTER_ID, 0x01);
    buf.Put16(groupId);
    buf.Put(sceneId);
    COMMAND_FOOTER(kName);
}

uint16_t encodeScenesClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverScenesAttributes", SCENES_CLUSTER_ID);
}

/*
 * Attribute SceneCount
 */
uint16_t encodeScenesClusterReadSceneCountAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadScenesSceneCount", SCENES_CLUSTER_ID);
}

/*
 * Attribute CurrentScene
 */
uint16_t encodeScenesClusterReadCurrentSceneAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0001 };
    READ_ATTRIBUTES("ReadScenesCurrentScene", SCENES_CLUSTER_ID);
}

/*
 * Attribute CurrentGroup
 */
uint16_t encodeScenesClusterReadCurrentGroupAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0002 };
    READ_ATTRIBUTES("ReadScenesCurrentGroup", SCENES_CLUSTER_ID);
}

/*
 * Attribute SceneValid
 */
uint16_t encodeScenesClusterReadSceneValidAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0003 };
    READ_ATTRIBUTES("ReadScenesSceneValid", SCENES_CLUSTER_ID);
}

/*
 * Attribute NameSupport
 */
uint16_t encodeScenesClusterReadNameSupportAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0004 };
    READ_ATTRIBUTES("ReadScenesNameSupport", SCENES_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeScenesClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length, EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadScenesClusterRevision", SCENES_CLUSTER_ID);
}

/*----------------------------------------------------------------------------*\
| Cluster TemperatureMeasurement                                      | 0x0402 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * MeasuredValue                                                     | 0x0000 |
| * MinMeasuredValue                                                  | 0x0001 |
| * MaxMeasuredValue                                                  | 0x0002 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

uint16_t encodeTemperatureMeasurementClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length,
                                                               EndpointId destination_endpoint)
{
    DISCOVER_ATTRIBUTES("DiscoverTemperatureMeasurementAttributes", TEMP_MEASUREMENT_CLUSTER_ID);
}

/*
 * Attribute MeasuredValue
 */
uint16_t encodeTemperatureMeasurementClusterReadMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                       EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0000 };
    READ_ATTRIBUTES("ReadTemperatureMeasurementMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
}

uint16_t encodeTemperatureMeasurementClusterReportMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                         EndpointId destination_endpoint, uint16_t min_interval,
                                                                         uint16_t max_interval, int16_t change)
{
    uint16_t attr_id  = 0x0000;
    uint8_t attr_type = { 0x29 };
    REPORT_ATTRIBUTE("ReportTemperatureMeasurementMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID, true, change);
}

/*
 * Attribute MinMeasuredValue
 */
uint16_t encodeTemperatureMeasurementClusterReadMinMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                          EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0001 };
    READ_ATTRIBUTES("ReadTemperatureMeasurementMinMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
}

/*
 * Attribute MaxMeasuredValue
 */
uint16_t encodeTemperatureMeasurementClusterReadMaxMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                          EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0x0002 };
    READ_ATTRIBUTES("ReadTemperatureMeasurementMaxMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
}

/*
 * Attribute ClusterRevision
 */
uint16_t encodeTemperatureMeasurementClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                         EndpointId destination_endpoint)
{
    uint16_t attr_ids[] = { 0xFFFD };
    READ_ATTRIBUTES("ReadTemperatureMeasurementClusterRevision", TEMP_MEASUREMENT_CLUSTER_ID);
}

} // extern "C"
