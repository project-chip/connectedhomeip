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
using namespace chip::System;

static uint16_t doEncodeApsFrame(BufBound & buf, ClusterId clusterId, EndpointId sourceEndpoint, EndpointId destinationEndpoint,
                                 EmberApsOption options, GroupId groupId, uint8_t sequence, uint8_t radius, bool isMeasuring)
{

    uint8_t control_byte = 0;
    buf.Put(control_byte) // Put in a control byte
        .Put16(clusterId)
        .Put8(sourceEndpoint)
        .Put8(destinationEndpoint)
        .Put(options, sizeof(EmberApsOption))
        .Put16(groupId)
        .Put8(sequence)
        .Put8(radius);

    size_t result = buf.Needed();
    if (isMeasuring)
    {
        ChipLogDetail(Zcl, "Measured APS frame size %d", result);
    }
    else if (buf.Fit())
    {
        ChipLogDetail(Zcl, "Successfully encoded %d bytes", result);
    }
    else
    {
        ChipLogError(Zcl, "Error encoding APS Frame: Buffer too small");
        result = 0;
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

#define COMMAND_HEADER(name, clusterId)                                                                                            \
    const char * kName = name;                                                                                                     \
                                                                                                                                   \
    PacketBufferHandle payload = PacketBuffer::NewWithAvailableSize(kMaxBufferSize);                                               \
    if (payload.IsNull())                                                                                                          \
    {                                                                                                                              \
        ChipLogError(Zcl, "Could not allocate packet buffer while trying to encode %s command", kName);                            \
        return payload;                                                                                                            \
    }                                                                                                                              \
                                                                                                                                   \
    BufBound buf = BufBound(payload->Start(), kMaxBufferSize);                                                                     \
    if (doEncodeApsFrame(buf, clusterId, kSourceEndpoint, destinationEndpoint, 0, 0, 0, 0, false))                                 \
    {

#define COMMAND_FOOTER()                                                                                                           \
    }                                                                                                                              \
    uint16_t result = buf.Fit() && CanCastTo<uint16_t>(buf.Needed()) ? static_cast<uint16_t>(buf.Needed()) : 0;                    \
    if (result == 0)                                                                                                               \
    {                                                                                                                              \
        ChipLogError(Zcl, "Command %s can't fit in the allocated buffer", kName);                                                  \
        return PacketBufferHandle();                                                                                               \
    }                                                                                                                              \
                                                                                                                                   \
    payload->SetDataLength(result);                                                                                                \
    return payload;

/*----------------------------------------------------------------------------*\
| Cluster Name                                                        |   ID   |
|---------------------------------------------------------------------+--------|
| BarrierControl                                                      | 0x0103 |
| Basic                                                               | 0x0000 |
| Binding                                                             | 0xF000 |
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

#define EMBER_ZCL_REPORTING_DIRECTION_REPORTED 0x00

#define ZCL_READ_ATTRIBUTES_COMMAND_ID (0x00)
#define ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID (0x01)
#define ZCL_WRITE_ATTRIBUTES_COMMAND_ID (0x02)
#define ZCL_WRITE_ATTRIBUTES_UNDIVIDED_COMMAND_ID (0x03)
#define ZCL_WRITE_ATTRIBUTES_RESPONSE_COMMAND_ID (0x04)
#define ZCL_WRITE_ATTRIBUTES_NO_RESPONSE_COMMAND_ID (0x05)
#define ZCL_CONFIGURE_REPORTING_COMMAND_ID (0x06)
#define ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID (0x07)
#define ZCL_READ_REPORTING_CONFIGURATION_COMMAND_ID (0x08)
#define ZCL_READ_REPORTING_CONFIGURATION_RESPONSE_COMMAND_ID (0x09)
#define ZCL_REPORT_ATTRIBUTES_COMMAND_ID (0x0A)
#define ZCL_DEFAULT_RESPONSE_COMMAND_ID (0x0B)
#define ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID (0x0C)
#define ZCL_DISCOVER_ATTRIBUTES_RESPONSE_COMMAND_ID (0x0D)
#define ZCL_READ_ATTRIBUTES_STRUCTURED_COMMAND_ID (0x0E)
#define ZCL_WRITE_ATTRIBUTES_STRUCTURED_COMMAND_ID (0x0F)
#define ZCL_WRITE_ATTRIBUTES_STRUCTURED_RESPONSE_COMMAND_ID (0x10)
#define ZCL_DISCOVER_COMMANDS_RECEIVED_COMMAND_ID (0x11)
#define ZCL_DISCOVER_COMMANDS_RECEIVED_RESPONSE_COMMAND_ID (0x12)
#define ZCL_DISCOVER_COMMANDS_GENERATED_COMMAND_ID (0x13)
#define ZCL_DISCOVER_COMMANDS_GENERATED_RESPONSE_COMMAND_ID (0x14)
#define ZCL_DISCOVER_ATTRIBUTES_EXTENDED_COMMAND_ID (0x15)
#define ZCL_DISCOVER_ATTRIBUTES_EXTENDED_RESPONSE_COMMAND_ID (0x16)

#define BARRIER_CONTROL_CLUSTER_ID 0x0103
#define ZCL_BARRIER_CONTROL_GO_TO_PERCENT_COMMAND_ID (0x00)
#define ZCL_BARRIER_CONTROL_STOP_COMMAND_ID (0x01)

#define BASIC_CLUSTER_ID 0x0000
#define ZCL_MFG_SPECIFIC_PING_COMMAND_ID (0x00)
#define ZCL_RESET_TO_FACTORY_DEFAULTS_COMMAND_ID (0x00)

#define BINDING_CLUSTER_ID 0xF000
#define ZCL_BIND_COMMAND_ID (0x00)
#define ZCL_UNBIND_COMMAND_ID (0x01)

#define COLOR_CONTROL_CLUSTER_ID 0x0300
#define ZCL_MOVE_COLOR_COMMAND_ID (0x08)
#define ZCL_MOVE_COLOR_TEMPERATURE_COMMAND_ID (0x4B)
#define ZCL_MOVE_HUE_COMMAND_ID (0x01)
#define ZCL_MOVE_SATURATION_COMMAND_ID (0x04)
#define ZCL_MOVE_TO_COLOR_COMMAND_ID (0x07)
#define ZCL_MOVE_TO_COLOR_TEMPERATURE_COMMAND_ID (0x0A)
#define ZCL_MOVE_TO_HUE_COMMAND_ID (0x00)
#define ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID (0x06)
#define ZCL_MOVE_TO_SATURATION_COMMAND_ID (0x03)
#define ZCL_STEP_COLOR_COMMAND_ID (0x09)
#define ZCL_STEP_COLOR_TEMPERATURE_COMMAND_ID (0x4C)
#define ZCL_STEP_HUE_COMMAND_ID (0x02)
#define ZCL_STEP_SATURATION_COMMAND_ID (0x05)
#define ZCL_STOP_MOVE_STEP_COMMAND_ID (0x47)

#define DOOR_LOCK_CLUSTER_ID 0x0101
#define ZCL_CLEAR_ALL_PINS_COMMAND_ID (0x08)
#define ZCL_CLEAR_ALL_RFIDS_COMMAND_ID (0x19)
#define ZCL_CLEAR_HOLIDAY_SCHEDULE_COMMAND_ID (0x13)
#define ZCL_CLEAR_PIN_COMMAND_ID (0x07)
#define ZCL_CLEAR_RFID_COMMAND_ID (0x18)
#define ZCL_CLEAR_WEEKDAY_SCHEDULE_COMMAND_ID (0x0D)
#define ZCL_CLEAR_YEARDAY_SCHEDULE_COMMAND_ID (0x10)
#define ZCL_GET_HOLIDAY_SCHEDULE_COMMAND_ID (0x12)
#define ZCL_GET_LOG_RECORD_COMMAND_ID (0x04)
#define ZCL_GET_PIN_COMMAND_ID (0x06)
#define ZCL_GET_RFID_COMMAND_ID (0x17)
#define ZCL_GET_USER_TYPE_COMMAND_ID (0x15)
#define ZCL_GET_WEEKDAY_SCHEDULE_COMMAND_ID (0x0C)
#define ZCL_GET_YEARDAY_SCHEDULE_COMMAND_ID (0x0F)
#define ZCL_LOCK_DOOR_COMMAND_ID (0x00)
#define ZCL_SET_HOLIDAY_SCHEDULE_COMMAND_ID (0x11)
#define ZCL_SET_PIN_COMMAND_ID (0x05)
#define ZCL_SET_RFID_COMMAND_ID (0x16)
#define ZCL_SET_USER_TYPE_COMMAND_ID (0x14)
#define ZCL_SET_WEEKDAY_SCHEDULE_COMMAND_ID (0x0B)
#define ZCL_SET_YEARDAY_SCHEDULE_COMMAND_ID (0x0E)
#define ZCL_UNLOCK_DOOR_COMMAND_ID (0x01)
#define ZCL_UNLOCK_WITH_TIMEOUT_COMMAND_ID (0x03)

#define GROUPS_CLUSTER_ID 0x0004
#define ZCL_ADD_GROUP_COMMAND_ID (0x00)
#define ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID (0x05)
#define ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID (0x02)
#define ZCL_REMOVE_ALL_GROUPS_COMMAND_ID (0x04)
#define ZCL_REMOVE_GROUP_COMMAND_ID (0x03)
#define ZCL_VIEW_GROUP_COMMAND_ID (0x01)

#define IAS_ZONE_CLUSTER_ID 0x0500

#define IDENTIFY_CLUSTER_ID 0x0003
#define ZCL_IDENTIFY_COMMAND_ID (0x00)
#define ZCL_IDENTIFY_QUERY_COMMAND_ID (0x01)

#define LEVEL_CONTROL_CLUSTER_ID 0x0008
#define ZCL_MOVE_COMMAND_ID (0x01)
#define ZCL_MOVE_TO_LEVEL_COMMAND_ID (0x00)
#define ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID (0x04)
#define ZCL_MOVE_WITH_ON_OFF_COMMAND_ID (0x05)
#define ZCL_STEP_COMMAND_ID (0x02)
#define ZCL_STEP_WITH_ON_OFF_COMMAND_ID (0x06)
#define ZCL_STOP_COMMAND_ID (0x03)
#define ZCL_STOP_WITH_ON_OFF_COMMAND_ID (0x07)

#define ON_OFF_CLUSTER_ID 0x0006
#define ZCL_OFF_COMMAND_ID (0x00)
#define ZCL_ON_COMMAND_ID (0x01)
#define ZCL_TOGGLE_COMMAND_ID (0x02)

#define SCENES_CLUSTER_ID 0x0005
#define ZCL_ADD_SCENE_COMMAND_ID (0x00)
#define ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID (0x06)
#define ZCL_RECALL_SCENE_COMMAND_ID (0x05)
#define ZCL_REMOVE_ALL_SCENES_COMMAND_ID (0x03)
#define ZCL_REMOVE_SCENE_COMMAND_ID (0x02)
#define ZCL_STORE_SCENE_COMMAND_ID (0x04)
#define ZCL_VIEW_SCENE_COMMAND_ID (0x01)

#define TEMP_MEASUREMENT_CLUSTER_ID 0x0402

// TODO: Find a way to calculate maximum message length for clusters
//       https://github.com/project-chip/connectedhomeip/issues/965
constexpr uint16_t kMaxBufferSize = 1024;

// This is a cluster-specific command so low two bits are 0b01.  The command
// is standard, so does not need a manufacturer code, and we're sending
// client to server, so all the remaining bits are 0.
constexpr uint8_t kFrameControlClusterSpecificCommand = 0x01;

// This is a global command, so the low bits are 0b00.  The command is
// standard, so does not need a manufacturer code, and we're sending client
// to server, so all the remaining bits are 0.
constexpr uint8_t kFrameControlGlobalCommand = 0x00;

// Pick source endpoint as 1 for now
constexpr EndpointId kSourceEndpoint = 1;

// Transaction sequence number. Just pick something for now.
constexpr uint8_t kSeqNum = 1;

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
PacketBufferHandle encodeBarrierControlClusterBarrierControlGoToPercentCommand(EndpointId destinationEndpoint, uint8_t percentOpen)
{
    COMMAND_HEADER("BarrierControlGoToPercent", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_BARRIER_CONTROL_GO_TO_PERCENT_COMMAND_ID)
        .Put8(percentOpen);
    COMMAND_FOOTER();
}

/*
 * Command BarrierControlStop
 */
PacketBufferHandle encodeBarrierControlClusterBarrierControlStopCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("BarrierControlStop", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_BARRIER_CONTROL_STOP_COMMAND_ID);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeBarrierControlClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverBarrierControlAttributes", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute BarrierMovingState
 */
PacketBufferHandle encodeBarrierControlClusterReadBarrierMovingStateAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBarrierControlBarrierMovingState", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0001);
    COMMAND_FOOTER();
}

/*
 * Attribute BarrierSafetyStatus
 */
PacketBufferHandle encodeBarrierControlClusterReadBarrierSafetyStatusAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBarrierControlBarrierSafetyStatus", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0002);
    COMMAND_FOOTER();
}

/*
 * Attribute BarrierCapabilities
 */
PacketBufferHandle encodeBarrierControlClusterReadBarrierCapabilitiesAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBarrierControlBarrierCapabilities", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0003);
    COMMAND_FOOTER();
}

/*
 * Attribute BarrierPosition
 */
PacketBufferHandle encodeBarrierControlClusterReadBarrierPositionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBarrierControlBarrierPosition", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x000A);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeBarrierControlClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBarrierControlClusterRevision", BARRIER_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
}

/*----------------------------------------------------------------------------*\
| Cluster Basic                                                       | 0x0000 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * MfgSpecificPing                                                   |   0x00 |
| * ResetToFactoryDefaults                                            |   0x00 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * ZclVersion                                                        | 0x0000 |
| * PowerSource                                                       | 0x0007 |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command MfgSpecificPing
 */
PacketBufferHandle encodeBasicClusterMfgSpecificPingCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("MfgSpecificPing", BASIC_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand | (1u << 2)).Put16(0x1002).Put8(kSeqNum).Put8(ZCL_MFG_SPECIFIC_PING_COMMAND_ID);
    COMMAND_FOOTER();
}

/*
 * Command ResetToFactoryDefaults
 */
PacketBufferHandle encodeBasicClusterResetToFactoryDefaultsCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ResetToFactoryDefaults", BASIC_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_RESET_TO_FACTORY_DEFAULTS_COMMAND_ID);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeBasicClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverBasicAttributes", BASIC_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute ZclVersion
 */
PacketBufferHandle encodeBasicClusterReadZclVersionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBasicZclVersion", BASIC_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

/*
 * Attribute PowerSource
 */
PacketBufferHandle encodeBasicClusterReadPowerSourceAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBasicPowerSource", BASIC_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0007);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeBasicClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBasicClusterRevision", BASIC_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
}

/*----------------------------------------------------------------------------*\
| Cluster Binding                                                     | 0xF000 |
|------------------------------------------------------------------------------|
| Commands:                                                           |        |
| * Bind                                                              |   0x00 |
| * Unbind                                                            |   0x01 |
|------------------------------------------------------------------------------|
| Attributes:                                                         |        |
| * ClusterRevision                                                   | 0xFFFD |
\*----------------------------------------------------------------------------*/

/*
 * Command Bind
 */
PacketBufferHandle encodeBindingClusterBindCommand(EndpointId destinationEndpoint, chip::NodeId nodeId, chip::GroupId groupId,
                                                   chip::EndpointId endpointId, chip::ClusterId clusterId)
{
    COMMAND_HEADER("Bind", BINDING_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_BIND_COMMAND_ID)
        .Put64(nodeId)
        .Put16(groupId)
        .Put8(endpointId)
        .Put16(clusterId);
    COMMAND_FOOTER();
}

/*
 * Command Unbind
 */
PacketBufferHandle encodeBindingClusterUnbindCommand(EndpointId destinationEndpoint, chip::NodeId nodeId, chip::GroupId groupId,
                                                     chip::EndpointId endpointId, chip::ClusterId clusterId)
{
    COMMAND_HEADER("Unbind", BINDING_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_UNBIND_COMMAND_ID)
        .Put64(nodeId)
        .Put16(groupId)
        .Put8(endpointId)
        .Put16(clusterId);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeBindingClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverBindingAttributes", BINDING_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeBindingClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadBindingClusterRevision", BINDING_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeColorControlClusterMoveColorCommand(EndpointId destinationEndpoint, int16_t rateX, int16_t rateY,
                                                             uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveColor", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_COLOR_COMMAND_ID)
        .Put16(static_cast<uint16_t>(rateX))
        .Put16(static_cast<uint16_t>(rateY))
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveColorTemperature
 */
PacketBufferHandle encodeColorControlClusterMoveColorTemperatureCommand(EndpointId destinationEndpoint, uint8_t moveMode,
                                                                        uint16_t rate, uint16_t colorTemperatureMinimum,
                                                                        uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                                        uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveColorTemperature", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_COLOR_TEMPERATURE_COMMAND_ID)
        .Put8(moveMode)
        .Put16(rate)
        .Put16(colorTemperatureMinimum)
        .Put16(colorTemperatureMaximum)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveHue
 */
PacketBufferHandle encodeColorControlClusterMoveHueCommand(EndpointId destinationEndpoint, uint8_t moveMode, uint8_t rate,
                                                           uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveHue", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_HUE_COMMAND_ID)
        .Put8(moveMode)
        .Put8(rate)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveSaturation
 */
PacketBufferHandle encodeColorControlClusterMoveSaturationCommand(EndpointId destinationEndpoint, uint8_t moveMode, uint8_t rate,
                                                                  uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveSaturation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_SATURATION_COMMAND_ID)
        .Put8(moveMode)
        .Put8(rate)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToColor
 */
PacketBufferHandle encodeColorControlClusterMoveToColorCommand(EndpointId destinationEndpoint, uint16_t colorX, uint16_t colorY,
                                                               uint16_t transitionTime, uint8_t optionsMask,
                                                               uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveToColor", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_COLOR_COMMAND_ID)
        .Put16(colorX)
        .Put16(colorY)
        .Put16(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToColorTemperature
 */
PacketBufferHandle encodeColorControlClusterMoveToColorTemperatureCommand(EndpointId destinationEndpoint, uint16_t colorTemperature,
                                                                          uint16_t transitionTime, uint8_t optionsMask,
                                                                          uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveToColorTemperature", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_COLOR_TEMPERATURE_COMMAND_ID)
        .Put16(colorTemperature)
        .Put16(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToHue
 */
PacketBufferHandle encodeColorControlClusterMoveToHueCommand(EndpointId destinationEndpoint, uint8_t hue, uint8_t direction,
                                                             uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveToHue", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_HUE_COMMAND_ID)
        .Put8(hue)
        .Put8(direction)
        .Put16(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToHueAndSaturation
 */
PacketBufferHandle encodeColorControlClusterMoveToHueAndSaturationCommand(EndpointId destinationEndpoint, uint8_t hue,
                                                                          uint8_t saturation, uint16_t transitionTime,
                                                                          uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveToHueAndSaturation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_HUE_AND_SATURATION_COMMAND_ID)
        .Put8(hue)
        .Put8(saturation)
        .Put16(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToSaturation
 */
PacketBufferHandle encodeColorControlClusterMoveToSaturationCommand(EndpointId destinationEndpoint, uint8_t saturation,
                                                                    uint16_t transitionTime, uint8_t optionsMask,
                                                                    uint8_t optionsOverride)
{
    COMMAND_HEADER("MoveToSaturation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_SATURATION_COMMAND_ID)
        .Put8(saturation)
        .Put16(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command StepColor
 */
PacketBufferHandle encodeColorControlClusterStepColorCommand(EndpointId destinationEndpoint, int16_t stepX, int16_t stepY,
                                                             uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("StepColor", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STEP_COLOR_COMMAND_ID)
        .Put16(static_cast<uint16_t>(stepX))
        .Put16(static_cast<uint16_t>(stepY))
        .Put16(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command StepColorTemperature
 */
PacketBufferHandle encodeColorControlClusterStepColorTemperatureCommand(EndpointId destinationEndpoint, uint8_t stepMode,
                                                                        uint16_t stepSize, uint16_t transitionTime,
                                                                        uint16_t colorTemperatureMinimum,
                                                                        uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                                        uint8_t optionsOverride)
{
    COMMAND_HEADER("StepColorTemperature", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STEP_COLOR_TEMPERATURE_COMMAND_ID)
        .Put8(stepMode)
        .Put16(stepSize)
        .Put16(transitionTime)
        .Put16(colorTemperatureMinimum)
        .Put16(colorTemperatureMaximum)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command StepHue
 */
PacketBufferHandle encodeColorControlClusterStepHueCommand(EndpointId destinationEndpoint, uint8_t stepMode, uint8_t stepSize,
                                                           uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    COMMAND_HEADER("StepHue", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STEP_HUE_COMMAND_ID)
        .Put8(stepMode)
        .Put8(stepSize)
        .Put8(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command StepSaturation
 */
PacketBufferHandle encodeColorControlClusterStepSaturationCommand(EndpointId destinationEndpoint, uint8_t stepMode,
                                                                  uint8_t stepSize, uint8_t transitionTime, uint8_t optionsMask,
                                                                  uint8_t optionsOverride)
{
    COMMAND_HEADER("StepSaturation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STEP_SATURATION_COMMAND_ID)
        .Put8(stepMode)
        .Put8(stepSize)
        .Put8(transitionTime)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

/*
 * Command StopMoveStep
 */
PacketBufferHandle encodeColorControlClusterStopMoveStepCommand(EndpointId destinationEndpoint, uint8_t optionsMask,
                                                                uint8_t optionsOverride)
{
    COMMAND_HEADER("StopMoveStep", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STOP_MOVE_STEP_COMMAND_ID)
        .Put8(optionsMask)
        .Put8(optionsOverride);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverColorControlAttributes", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentHue
 */
PacketBufferHandle encodeColorControlClusterReadCurrentHueAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlCurrentHue", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterReportCurrentHueAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                      uint16_t maxInterval, uint8_t change)
{
    COMMAND_HEADER("ReportColorControlCurrentHue", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0000)
        .Put8(32)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put8(static_cast<uint8_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentSaturation
 */
PacketBufferHandle encodeColorControlClusterReadCurrentSaturationAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlCurrentSaturation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0001);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterReportCurrentSaturationAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                             uint16_t maxInterval, uint8_t change)
{
    COMMAND_HEADER("ReportColorControlCurrentSaturation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0001)
        .Put8(32)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put8(static_cast<uint8_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute RemainingTime
 */
PacketBufferHandle encodeColorControlClusterReadRemainingTimeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlRemainingTime", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0002);
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentX
 */
PacketBufferHandle encodeColorControlClusterReadCurrentXAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlCurrentX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0003);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterReportCurrentXAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                    uint16_t maxInterval, uint16_t change)
{
    COMMAND_HEADER("ReportColorControlCurrentX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0003)
        .Put8(33)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put16(static_cast<uint16_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentY
 */
PacketBufferHandle encodeColorControlClusterReadCurrentYAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlCurrentY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0004);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterReportCurrentYAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                    uint16_t maxInterval, uint16_t change)
{
    COMMAND_HEADER("ReportColorControlCurrentY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0004)
        .Put8(33)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put16(static_cast<uint16_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute DriftCompensation
 */
PacketBufferHandle encodeColorControlClusterReadDriftCompensationAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlDriftCompensation", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0005);
    COMMAND_FOOTER();
}

/*
 * Attribute CompensationText
 */
PacketBufferHandle encodeColorControlClusterReadCompensationTextAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlCompensationText", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0006);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorTemperature
 */
PacketBufferHandle encodeColorControlClusterReadColorTemperatureAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorTemperature", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0007);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterReportColorTemperatureAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                            uint16_t maxInterval, uint16_t change)
{
    COMMAND_HEADER("ReportColorControlColorTemperature", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0007)
        .Put8(33)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put16(static_cast<uint16_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorMode
 */
PacketBufferHandle encodeColorControlClusterReadColorModeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorMode", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0008);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorControlOptions
 */
PacketBufferHandle encodeColorControlClusterReadColorControlOptionsAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorControlOptions", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x000F);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorControlOptionsAttribute(EndpointId destinationEndpoint,
                                                                              uint8_t colorControlOptions)
{
    COMMAND_HEADER("WriteColorControlColorControlOptions", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x000F)
        .Put8(24)
        .Put8(static_cast<uint8_t>(colorControlOptions));
    COMMAND_FOOTER();
}

/*
 * Attribute NumberOfPrimaries
 */
PacketBufferHandle encodeColorControlClusterReadNumberOfPrimariesAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlNumberOfPrimaries", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0010);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary1X
 */
PacketBufferHandle encodeColorControlClusterReadPrimary1XAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary1X", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0011);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary1Y
 */
PacketBufferHandle encodeColorControlClusterReadPrimary1YAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary1Y", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0012);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary1Intensity
 */
PacketBufferHandle encodeColorControlClusterReadPrimary1IntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary1Intensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0013);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary2X
 */
PacketBufferHandle encodeColorControlClusterReadPrimary2XAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary2X", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0015);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary2Y
 */
PacketBufferHandle encodeColorControlClusterReadPrimary2YAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary2Y", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0016);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary2Intensity
 */
PacketBufferHandle encodeColorControlClusterReadPrimary2IntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary2Intensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0017);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary3X
 */
PacketBufferHandle encodeColorControlClusterReadPrimary3XAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary3X", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0019);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary3Y
 */
PacketBufferHandle encodeColorControlClusterReadPrimary3YAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary3Y", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x001A);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary3Intensity
 */
PacketBufferHandle encodeColorControlClusterReadPrimary3IntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary3Intensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x001B);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary4X
 */
PacketBufferHandle encodeColorControlClusterReadPrimary4XAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary4X", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0020);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary4Y
 */
PacketBufferHandle encodeColorControlClusterReadPrimary4YAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary4Y", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0021);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary4Intensity
 */
PacketBufferHandle encodeColorControlClusterReadPrimary4IntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary4Intensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0022);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary5X
 */
PacketBufferHandle encodeColorControlClusterReadPrimary5XAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary5X", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0024);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary5Y
 */
PacketBufferHandle encodeColorControlClusterReadPrimary5YAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary5Y", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0025);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary5Intensity
 */
PacketBufferHandle encodeColorControlClusterReadPrimary5IntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary5Intensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0026);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary6X
 */
PacketBufferHandle encodeColorControlClusterReadPrimary6XAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary6X", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0028);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary6Y
 */
PacketBufferHandle encodeColorControlClusterReadPrimary6YAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary6Y", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0029);
    COMMAND_FOOTER();
}

/*
 * Attribute Primary6Intensity
 */
PacketBufferHandle encodeColorControlClusterReadPrimary6IntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlPrimary6Intensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x002A);
    COMMAND_FOOTER();
}

/*
 * Attribute WhitePointX
 */
PacketBufferHandle encodeColorControlClusterReadWhitePointXAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlWhitePointX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0030);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteWhitePointXAttribute(EndpointId destinationEndpoint, uint16_t whitePointX)
{
    COMMAND_HEADER("WriteColorControlWhitePointX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0030)
        .Put8(33)
        .Put16(static_cast<uint16_t>(whitePointX));
    COMMAND_FOOTER();
}

/*
 * Attribute WhitePointY
 */
PacketBufferHandle encodeColorControlClusterReadWhitePointYAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlWhitePointY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0031);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteWhitePointYAttribute(EndpointId destinationEndpoint, uint16_t whitePointY)
{
    COMMAND_HEADER("WriteColorControlWhitePointY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0031)
        .Put8(33)
        .Put16(static_cast<uint16_t>(whitePointY));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointRX
 */
PacketBufferHandle encodeColorControlClusterReadColorPointRXAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointRX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0032);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointRXAttribute(EndpointId destinationEndpoint, uint16_t colorPointRX)
{
    COMMAND_HEADER("WriteColorControlColorPointRX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0032)
        .Put8(33)
        .Put16(static_cast<uint16_t>(colorPointRX));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointRY
 */
PacketBufferHandle encodeColorControlClusterReadColorPointRYAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointRY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0033);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointRYAttribute(EndpointId destinationEndpoint, uint16_t colorPointRY)
{
    COMMAND_HEADER("WriteColorControlColorPointRY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0033)
        .Put8(33)
        .Put16(static_cast<uint16_t>(colorPointRY));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointRIntensity
 */
PacketBufferHandle encodeColorControlClusterReadColorPointRIntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointRIntensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0034);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointRIntensityAttribute(EndpointId destinationEndpoint,
                                                                               uint8_t colorPointRIntensity)
{
    COMMAND_HEADER("WriteColorControlColorPointRIntensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0034)
        .Put8(32)
        .Put8(static_cast<uint8_t>(colorPointRIntensity));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointGX
 */
PacketBufferHandle encodeColorControlClusterReadColorPointGXAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointGX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0036);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointGXAttribute(EndpointId destinationEndpoint, uint16_t colorPointGX)
{
    COMMAND_HEADER("WriteColorControlColorPointGX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0036)
        .Put8(33)
        .Put16(static_cast<uint16_t>(colorPointGX));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointGY
 */
PacketBufferHandle encodeColorControlClusterReadColorPointGYAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointGY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0037);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointGYAttribute(EndpointId destinationEndpoint, uint16_t colorPointGY)
{
    COMMAND_HEADER("WriteColorControlColorPointGY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0037)
        .Put8(33)
        .Put16(static_cast<uint16_t>(colorPointGY));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointGIntensity
 */
PacketBufferHandle encodeColorControlClusterReadColorPointGIntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointGIntensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0038);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointGIntensityAttribute(EndpointId destinationEndpoint,
                                                                               uint8_t colorPointGIntensity)
{
    COMMAND_HEADER("WriteColorControlColorPointGIntensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0038)
        .Put8(32)
        .Put8(static_cast<uint8_t>(colorPointGIntensity));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointBX
 */
PacketBufferHandle encodeColorControlClusterReadColorPointBXAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointBX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x003A);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointBXAttribute(EndpointId destinationEndpoint, uint16_t colorPointBX)
{
    COMMAND_HEADER("WriteColorControlColorPointBX", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x003A)
        .Put8(33)
        .Put16(static_cast<uint16_t>(colorPointBX));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointBY
 */
PacketBufferHandle encodeColorControlClusterReadColorPointBYAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointBY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x003B);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointBYAttribute(EndpointId destinationEndpoint, uint16_t colorPointBY)
{
    COMMAND_HEADER("WriteColorControlColorPointBY", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x003B)
        .Put8(33)
        .Put16(static_cast<uint16_t>(colorPointBY));
    COMMAND_FOOTER();
}

/*
 * Attribute ColorPointBIntensity
 */
PacketBufferHandle encodeColorControlClusterReadColorPointBIntensityAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorPointBIntensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x003C);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteColorPointBIntensityAttribute(EndpointId destinationEndpoint,
                                                                               uint8_t colorPointBIntensity)
{
    COMMAND_HEADER("WriteColorControlColorPointBIntensity", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x003C)
        .Put8(32)
        .Put8(static_cast<uint8_t>(colorPointBIntensity));
    COMMAND_FOOTER();
}

/*
 * Attribute EnhancedCurrentHue
 */
PacketBufferHandle encodeColorControlClusterReadEnhancedCurrentHueAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlEnhancedCurrentHue", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x4000);
    COMMAND_FOOTER();
}

/*
 * Attribute EnhancedColorMode
 */
PacketBufferHandle encodeColorControlClusterReadEnhancedColorModeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlEnhancedColorMode", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x4001);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorLoopActive
 */
PacketBufferHandle encodeColorControlClusterReadColorLoopActiveAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorLoopActive", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x4002);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorLoopDirection
 */
PacketBufferHandle encodeColorControlClusterReadColorLoopDirectionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorLoopDirection", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x4003);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorLoopTime
 */
PacketBufferHandle encodeColorControlClusterReadColorLoopTimeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorLoopTime", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x4004);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorCapabilities
 */
PacketBufferHandle encodeColorControlClusterReadColorCapabilitiesAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorCapabilities", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x400A);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorTempPhysicalMin
 */
PacketBufferHandle encodeColorControlClusterReadColorTempPhysicalMinAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorTempPhysicalMin", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x400B);
    COMMAND_FOOTER();
}

/*
 * Attribute ColorTempPhysicalMax
 */
PacketBufferHandle encodeColorControlClusterReadColorTempPhysicalMaxAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlColorTempPhysicalMax", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x400C);
    COMMAND_FOOTER();
}

/*
 * Attribute CoupleColorTempToLevelMinMireds
 */
PacketBufferHandle encodeColorControlClusterReadCoupleColorTempToLevelMinMiredsAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlCoupleColorTempToLevelMinMireds", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x400D);
    COMMAND_FOOTER();
}

/*
 * Attribute StartUpColorTemperatureMireds
 */
PacketBufferHandle encodeColorControlClusterReadStartUpColorTemperatureMiredsAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlStartUpColorTemperatureMireds", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x4010);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeColorControlClusterWriteStartUpColorTemperatureMiredsAttribute(EndpointId destinationEndpoint,
                                                                                        uint16_t startUpColorTemperatureMireds)
{
    COMMAND_HEADER("WriteColorControlStartUpColorTemperatureMireds", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x4010)
        .Put8(33)
        .Put16(static_cast<uint16_t>(startUpColorTemperatureMireds));
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeColorControlClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadColorControlClusterRevision", COLOR_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeDoorLockClusterClearAllPinsCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ClearAllPins", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_CLEAR_ALL_PINS_COMMAND_ID);
    COMMAND_FOOTER();
}

/*
 * Command ClearAllRfids
 */
PacketBufferHandle encodeDoorLockClusterClearAllRfidsCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ClearAllRfids", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_CLEAR_ALL_RFIDS_COMMAND_ID);
    COMMAND_FOOTER();
}

/*
 * Command ClearHolidaySchedule
 */
PacketBufferHandle encodeDoorLockClusterClearHolidayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId)
{
    COMMAND_HEADER("ClearHolidaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_CLEAR_HOLIDAY_SCHEDULE_COMMAND_ID).Put8(scheduleId);
    COMMAND_FOOTER();
}

/*
 * Command ClearPin
 */
PacketBufferHandle encodeDoorLockClusterClearPinCommand(EndpointId destinationEndpoint, uint16_t userId)
{
    COMMAND_HEADER("ClearPin", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_CLEAR_PIN_COMMAND_ID).Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command ClearRfid
 */
PacketBufferHandle encodeDoorLockClusterClearRfidCommand(EndpointId destinationEndpoint, uint16_t userId)
{
    COMMAND_HEADER("ClearRfid", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_CLEAR_RFID_COMMAND_ID).Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command ClearWeekdaySchedule
 */
PacketBufferHandle encodeDoorLockClusterClearWeekdayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                    uint16_t userId)
{
    COMMAND_HEADER("ClearWeekdaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CLEAR_WEEKDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command ClearYeardaySchedule
 */
PacketBufferHandle encodeDoorLockClusterClearYeardayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                    uint16_t userId)
{
    COMMAND_HEADER("ClearYeardaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CLEAR_YEARDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command GetHolidaySchedule
 */
PacketBufferHandle encodeDoorLockClusterGetHolidayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId)
{
    COMMAND_HEADER("GetHolidaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_GET_HOLIDAY_SCHEDULE_COMMAND_ID).Put8(scheduleId);
    COMMAND_FOOTER();
}

/*
 * Command GetLogRecord
 */
PacketBufferHandle encodeDoorLockClusterGetLogRecordCommand(EndpointId destinationEndpoint, uint16_t logIndex)
{
    COMMAND_HEADER("GetLogRecord", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_GET_LOG_RECORD_COMMAND_ID).Put16(logIndex);
    COMMAND_FOOTER();
}

/*
 * Command GetPin
 */
PacketBufferHandle encodeDoorLockClusterGetPinCommand(EndpointId destinationEndpoint, uint16_t userId)
{
    COMMAND_HEADER("GetPin", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_GET_PIN_COMMAND_ID).Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command GetRfid
 */
PacketBufferHandle encodeDoorLockClusterGetRfidCommand(EndpointId destinationEndpoint, uint16_t userId)
{
    COMMAND_HEADER("GetRfid", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_GET_RFID_COMMAND_ID).Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command GetUserType
 */
PacketBufferHandle encodeDoorLockClusterGetUserTypeCommand(EndpointId destinationEndpoint, uint16_t userId)
{
    COMMAND_HEADER("GetUserType", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_GET_USER_TYPE_COMMAND_ID).Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command GetWeekdaySchedule
 */
PacketBufferHandle encodeDoorLockClusterGetWeekdayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                  uint16_t userId)
{
    COMMAND_HEADER("GetWeekdaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_GET_WEEKDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command GetYeardaySchedule
 */
PacketBufferHandle encodeDoorLockClusterGetYeardayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                  uint16_t userId)
{
    COMMAND_HEADER("GetYeardaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_GET_YEARDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put16(userId);
    COMMAND_FOOTER();
}

/*
 * Command LockDoor
 */
PacketBufferHandle encodeDoorLockClusterLockDoorCommand(EndpointId destinationEndpoint, char * pin)
{
    COMMAND_HEADER("LockDoor", DOOR_LOCK_CLUSTER_ID);
    size_t pinStrLen = strlen(pin);
    if (!CanCastTo<uint8_t>(pinStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, pinStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_LOCK_DOOR_COMMAND_ID)
        .Put(static_cast<uint8_t>(pinStrLen))
        .Put(pin);
    COMMAND_FOOTER();
}

/*
 * Command SetHolidaySchedule
 */
PacketBufferHandle encodeDoorLockClusterSetHolidayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                  uint32_t localStartTime, uint32_t localEndTime,
                                                                  uint8_t operatingModeDuringHoliday)
{
    COMMAND_HEADER("SetHolidaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_SET_HOLIDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put32(localStartTime)
        .Put32(localEndTime)
        .Put8(operatingModeDuringHoliday);
    COMMAND_FOOTER();
}

/*
 * Command SetPin
 */
PacketBufferHandle encodeDoorLockClusterSetPinCommand(EndpointId destinationEndpoint, uint16_t userId, uint8_t userStatus,
                                                      uint8_t userType, char * pin)
{
    COMMAND_HEADER("SetPin", DOOR_LOCK_CLUSTER_ID);
    size_t pinStrLen = strlen(pin);
    if (!CanCastTo<uint8_t>(pinStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, pinStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_SET_PIN_COMMAND_ID)
        .Put16(userId)
        .Put8(userStatus)
        .Put8(userType)
        .Put(static_cast<uint8_t>(pinStrLen))
        .Put(pin);
    COMMAND_FOOTER();
}

/*
 * Command SetRfid
 */
PacketBufferHandle encodeDoorLockClusterSetRfidCommand(EndpointId destinationEndpoint, uint16_t userId, uint8_t userStatus,
                                                       uint8_t userType, char * id)
{
    COMMAND_HEADER("SetRfid", DOOR_LOCK_CLUSTER_ID);
    size_t idStrLen = strlen(id);
    if (!CanCastTo<uint8_t>(idStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, idStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_SET_RFID_COMMAND_ID)
        .Put16(userId)
        .Put8(userStatus)
        .Put8(userType)
        .Put(static_cast<uint8_t>(idStrLen))
        .Put(id);
    COMMAND_FOOTER();
}

/*
 * Command SetUserType
 */
PacketBufferHandle encodeDoorLockClusterSetUserTypeCommand(EndpointId destinationEndpoint, uint16_t userId, uint8_t userType)
{
    COMMAND_HEADER("SetUserType", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_SET_USER_TYPE_COMMAND_ID).Put16(userId).Put8(userType);
    COMMAND_FOOTER();
}

/*
 * Command SetWeekdaySchedule
 */
PacketBufferHandle encodeDoorLockClusterSetWeekdayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                  uint16_t userId, uint8_t daysMask, uint8_t startHour,
                                                                  uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    COMMAND_HEADER("SetWeekdaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_SET_WEEKDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put16(userId)
        .Put8(daysMask)
        .Put8(startHour)
        .Put8(startMinute)
        .Put8(endHour)
        .Put8(endMinute);
    COMMAND_FOOTER();
}

/*
 * Command SetYeardaySchedule
 */
PacketBufferHandle encodeDoorLockClusterSetYeardayScheduleCommand(EndpointId destinationEndpoint, uint8_t scheduleId,
                                                                  uint16_t userId, uint32_t localStartTime, uint32_t localEndTime)
{
    COMMAND_HEADER("SetYeardaySchedule", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_SET_YEARDAY_SCHEDULE_COMMAND_ID)
        .Put8(scheduleId)
        .Put16(userId)
        .Put32(localStartTime)
        .Put32(localEndTime);
    COMMAND_FOOTER();
}

/*
 * Command UnlockDoor
 */
PacketBufferHandle encodeDoorLockClusterUnlockDoorCommand(EndpointId destinationEndpoint, char * pin)
{
    COMMAND_HEADER("UnlockDoor", DOOR_LOCK_CLUSTER_ID);
    size_t pinStrLen = strlen(pin);
    if (!CanCastTo<uint8_t>(pinStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, pinStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_UNLOCK_DOOR_COMMAND_ID)
        .Put(static_cast<uint8_t>(pinStrLen))
        .Put(pin);
    COMMAND_FOOTER();
}

/*
 * Command UnlockWithTimeout
 */
PacketBufferHandle encodeDoorLockClusterUnlockWithTimeoutCommand(EndpointId destinationEndpoint, uint16_t timeoutInSeconds,
                                                                 char * pin)
{
    COMMAND_HEADER("UnlockWithTimeout", DOOR_LOCK_CLUSTER_ID);
    size_t pinStrLen = strlen(pin);
    if (!CanCastTo<uint8_t>(pinStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, pinStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_UNLOCK_WITH_TIMEOUT_COMMAND_ID)
        .Put16(timeoutInSeconds)
        .Put(static_cast<uint8_t>(pinStrLen))
        .Put(pin);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeDoorLockClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverDoorLockAttributes", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute LockState
 */
PacketBufferHandle encodeDoorLockClusterReadLockStateAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadDoorLockLockState", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeDoorLockClusterReportLockStateAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                 uint16_t maxInterval)
{
    COMMAND_HEADER("ReportDoorLockLockState", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0000)
        .Put8(48)
        .Put16(minInterval)
        .Put16(maxInterval);
    COMMAND_FOOTER();
}

/*
 * Attribute LockType
 */
PacketBufferHandle encodeDoorLockClusterReadLockTypeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadDoorLockLockType", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0001);
    COMMAND_FOOTER();
}

/*
 * Attribute ActuatorEnabled
 */
PacketBufferHandle encodeDoorLockClusterReadActuatorEnabledAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadDoorLockActuatorEnabled", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0002);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeDoorLockClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadDoorLockClusterRevision", DOOR_LOCK_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeGroupsClusterAddGroupCommand(EndpointId destinationEndpoint, uint16_t groupId, char * groupName)
{
    COMMAND_HEADER("AddGroup", GROUPS_CLUSTER_ID);
    size_t groupNameStrLen = strlen(groupName);
    if (!CanCastTo<uint8_t>(groupNameStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, groupNameStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_ADD_GROUP_COMMAND_ID)
        .Put16(groupId)
        .Put(static_cast<uint8_t>(groupNameStrLen))
        .Put(groupName);
    COMMAND_FOOTER();
}

/*
 * Command AddGroupIfIdentifying
 */
PacketBufferHandle encodeGroupsClusterAddGroupIfIdentifyingCommand(EndpointId destinationEndpoint, uint16_t groupId,
                                                                   char * groupName)
{
    COMMAND_HEADER("AddGroupIfIdentifying", GROUPS_CLUSTER_ID);
    size_t groupNameStrLen = strlen(groupName);
    if (!CanCastTo<uint8_t>(groupNameStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, groupNameStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_ADD_GROUP_IF_IDENTIFYING_COMMAND_ID)
        .Put16(groupId)
        .Put(static_cast<uint8_t>(groupNameStrLen))
        .Put(groupName);
    COMMAND_FOOTER();
}

/*
 * Command GetGroupMembership
 */
PacketBufferHandle encodeGroupsClusterGetGroupMembershipCommand(EndpointId destinationEndpoint, uint8_t groupCount,
                                                                uint16_t groupList)
{
    COMMAND_HEADER("GetGroupMembership", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_GET_GROUP_MEMBERSHIP_COMMAND_ID)
        .Put8(groupCount)
        .Put16(groupList);
    COMMAND_FOOTER();
}

/*
 * Command RemoveAllGroups
 */
PacketBufferHandle encodeGroupsClusterRemoveAllGroupsCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("RemoveAllGroups", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_REMOVE_ALL_GROUPS_COMMAND_ID);
    COMMAND_FOOTER();
}

/*
 * Command RemoveGroup
 */
PacketBufferHandle encodeGroupsClusterRemoveGroupCommand(EndpointId destinationEndpoint, uint16_t groupId)
{
    COMMAND_HEADER("RemoveGroup", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_REMOVE_GROUP_COMMAND_ID).Put16(groupId);
    COMMAND_FOOTER();
}

/*
 * Command ViewGroup
 */
PacketBufferHandle encodeGroupsClusterViewGroupCommand(EndpointId destinationEndpoint, uint16_t groupId)
{
    COMMAND_HEADER("ViewGroup", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_VIEW_GROUP_COMMAND_ID).Put16(groupId);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeGroupsClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverGroupsAttributes", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute NameSupport
 */
PacketBufferHandle encodeGroupsClusterReadNameSupportAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadGroupsNameSupport", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeGroupsClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadGroupsClusterRevision", GROUPS_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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

PacketBufferHandle encodeIasZoneClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverIasZoneAttributes", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute ZoneState
 */
PacketBufferHandle encodeIasZoneClusterReadZoneStateAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIasZoneZoneState", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

/*
 * Attribute ZoneType
 */
PacketBufferHandle encodeIasZoneClusterReadZoneTypeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIasZoneZoneType", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0001);
    COMMAND_FOOTER();
}

/*
 * Attribute ZoneStatus
 */
PacketBufferHandle encodeIasZoneClusterReadZoneStatusAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIasZoneZoneStatus", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0002);
    COMMAND_FOOTER();
}

/*
 * Attribute IasCieAddress
 */
PacketBufferHandle encodeIasZoneClusterReadIasCieAddressAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIasZoneIasCieAddress", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0010);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeIasZoneClusterWriteIasCieAddressAttribute(EndpointId destinationEndpoint, uint64_t iasCieAddress)
{
    COMMAND_HEADER("WriteIasZoneIasCieAddress", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0010)
        .Put8(240)
        .Put64(static_cast<uint64_t>(iasCieAddress));
    COMMAND_FOOTER();
}

/*
 * Attribute ZoneId
 */
PacketBufferHandle encodeIasZoneClusterReadZoneIdAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIasZoneZoneId", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0011);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeIasZoneClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIasZoneClusterRevision", IAS_ZONE_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeIdentifyClusterIdentifyCommand(EndpointId destinationEndpoint, uint16_t identifyTime)
{
    COMMAND_HEADER("Identify", IDENTIFY_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_IDENTIFY_COMMAND_ID).Put16(identifyTime);
    COMMAND_FOOTER();
}

/*
 * Command IdentifyQuery
 */
PacketBufferHandle encodeIdentifyClusterIdentifyQueryCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("IdentifyQuery", IDENTIFY_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_IDENTIFY_QUERY_COMMAND_ID);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeIdentifyClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverIdentifyAttributes", IDENTIFY_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute IdentifyTime
 */
PacketBufferHandle encodeIdentifyClusterReadIdentifyTimeAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIdentifyIdentifyTime", IDENTIFY_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeIdentifyClusterWriteIdentifyTimeAttribute(EndpointId destinationEndpoint, uint16_t identifyTime)
{
    COMMAND_HEADER("WriteIdentifyIdentifyTime", IDENTIFY_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_WRITE_ATTRIBUTES_COMMAND_ID)
        .Put16(0x0000)
        .Put8(33)
        .Put16(static_cast<uint16_t>(identifyTime));
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeIdentifyClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadIdentifyClusterRevision", IDENTIFY_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeLevelControlClusterMoveCommand(EndpointId destinationEndpoint, uint8_t moveMode, uint8_t rate,
                                                        uint8_t optionMask, uint8_t optionOverride)
{
    COMMAND_HEADER("Move", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_COMMAND_ID)
        .Put8(moveMode)
        .Put8(rate)
        .Put8(optionMask)
        .Put8(optionOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToLevel
 */
PacketBufferHandle encodeLevelControlClusterMoveToLevelCommand(EndpointId destinationEndpoint, uint8_t level,
                                                               uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    COMMAND_HEADER("MoveToLevel", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_LEVEL_COMMAND_ID)
        .Put8(level)
        .Put16(transitionTime)
        .Put8(optionMask)
        .Put8(optionOverride);
    COMMAND_FOOTER();
}

/*
 * Command MoveToLevelWithOnOff
 */
PacketBufferHandle encodeLevelControlClusterMoveToLevelWithOnOffCommand(EndpointId destinationEndpoint, uint8_t level,
                                                                        uint16_t transitionTime)
{
    COMMAND_HEADER("MoveToLevelWithOnOff", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_MOVE_TO_LEVEL_WITH_ON_OFF_COMMAND_ID)
        .Put8(level)
        .Put16(transitionTime);
    COMMAND_FOOTER();
}

/*
 * Command MoveWithOnOff
 */
PacketBufferHandle encodeLevelControlClusterMoveWithOnOffCommand(EndpointId destinationEndpoint, uint8_t moveMode, uint8_t rate)
{
    COMMAND_HEADER("MoveWithOnOff", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_MOVE_WITH_ON_OFF_COMMAND_ID).Put8(moveMode).Put8(rate);
    COMMAND_FOOTER();
}

/*
 * Command Step
 */
PacketBufferHandle encodeLevelControlClusterStepCommand(EndpointId destinationEndpoint, uint8_t stepMode, uint8_t stepSize,
                                                        uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    COMMAND_HEADER("Step", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STEP_COMMAND_ID)
        .Put8(stepMode)
        .Put8(stepSize)
        .Put16(transitionTime)
        .Put8(optionMask)
        .Put8(optionOverride);
    COMMAND_FOOTER();
}

/*
 * Command StepWithOnOff
 */
PacketBufferHandle encodeLevelControlClusterStepWithOnOffCommand(EndpointId destinationEndpoint, uint8_t stepMode, uint8_t stepSize,
                                                                 uint16_t transitionTime)
{
    COMMAND_HEADER("StepWithOnOff", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_STEP_WITH_ON_OFF_COMMAND_ID)
        .Put8(stepMode)
        .Put8(stepSize)
        .Put16(transitionTime);
    COMMAND_FOOTER();
}

/*
 * Command Stop
 */
PacketBufferHandle encodeLevelControlClusterStopCommand(EndpointId destinationEndpoint, uint8_t optionMask, uint8_t optionOverride)
{
    COMMAND_HEADER("Stop", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_STOP_COMMAND_ID).Put8(optionMask).Put8(optionOverride);
    COMMAND_FOOTER();
}

/*
 * Command StopWithOnOff
 */
PacketBufferHandle encodeLevelControlClusterStopWithOnOffCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("StopWithOnOff", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_STOP_WITH_ON_OFF_COMMAND_ID);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeLevelControlClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverLevelControlAttributes", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentLevel
 */
PacketBufferHandle encodeLevelControlClusterReadCurrentLevelAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadLevelControlCurrentLevel", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeLevelControlClusterReportCurrentLevelAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                                        uint16_t maxInterval, uint8_t change)
{
    COMMAND_HEADER("ReportLevelControlCurrentLevel", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0000)
        .Put8(32)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put8(static_cast<uint8_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeLevelControlClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadLevelControlClusterRevision", LEVEL_CONTROL_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeOnOffClusterOffCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("Off", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_OFF_COMMAND_ID);
    COMMAND_FOOTER();
}

/*
 * Command On
 */
PacketBufferHandle encodeOnOffClusterOnCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("On", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_ON_COMMAND_ID);
    COMMAND_FOOTER();
}

/*
 * Command Toggle
 */
PacketBufferHandle encodeOnOffClusterToggleCommand(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("Toggle", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_TOGGLE_COMMAND_ID);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeOnOffClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverOnOffAttributes", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute OnOff
 */
PacketBufferHandle encodeOnOffClusterReadOnOffAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadOnOffOnOff", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeOnOffClusterReportOnOffAttribute(EndpointId destinationEndpoint, uint16_t minInterval,
                                                          uint16_t maxInterval)
{
    COMMAND_HEADER("ReportOnOffOnOff", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0000)
        .Put8(16)
        .Put16(minInterval)
        .Put16(maxInterval);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeOnOffClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadOnOffClusterRevision", ON_OFF_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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
PacketBufferHandle encodeScenesClusterAddSceneCommand(EndpointId destinationEndpoint, uint16_t groupId, uint8_t sceneId,
                                                      uint16_t transitionTime, char * sceneName, chip::ClusterId clusterId,
                                                      uint8_t length, uint8_t value)
{
    COMMAND_HEADER("AddScene", SCENES_CLUSTER_ID);
    size_t sceneNameStrLen = strlen(sceneName);
    if (!CanCastTo<uint8_t>(sceneNameStrLen))
    {
        ChipLogError(Zcl, "Error encoding %s command. String too long: %d", kName, sceneNameStrLen);
        return PacketBufferHandle();
    }
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_ADD_SCENE_COMMAND_ID)
        .Put16(groupId)
        .Put8(sceneId)
        .Put16(transitionTime)
        .Put(static_cast<uint8_t>(sceneNameStrLen))
        .Put(sceneName)
        .Put16(clusterId)
        .Put8(length)
        .Put8(value);
    COMMAND_FOOTER();
}

/*
 * Command GetSceneMembership
 */
PacketBufferHandle encodeScenesClusterGetSceneMembershipCommand(EndpointId destinationEndpoint, uint16_t groupId)
{
    COMMAND_HEADER("GetSceneMembership", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_GET_SCENE_MEMBERSHIP_COMMAND_ID).Put16(groupId);
    COMMAND_FOOTER();
}

/*
 * Command RecallScene
 */
PacketBufferHandle encodeScenesClusterRecallSceneCommand(EndpointId destinationEndpoint, uint16_t groupId, uint8_t sceneId,
                                                         uint16_t transitionTime)
{
    COMMAND_HEADER("RecallScene", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_RECALL_SCENE_COMMAND_ID)
        .Put16(groupId)
        .Put8(sceneId)
        .Put16(transitionTime);
    COMMAND_FOOTER();
}

/*
 * Command RemoveAllScenes
 */
PacketBufferHandle encodeScenesClusterRemoveAllScenesCommand(EndpointId destinationEndpoint, uint16_t groupId)
{
    COMMAND_HEADER("RemoveAllScenes", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_REMOVE_ALL_SCENES_COMMAND_ID).Put16(groupId);
    COMMAND_FOOTER();
}

/*
 * Command RemoveScene
 */
PacketBufferHandle encodeScenesClusterRemoveSceneCommand(EndpointId destinationEndpoint, uint16_t groupId, uint8_t sceneId)
{
    COMMAND_HEADER("RemoveScene", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_REMOVE_SCENE_COMMAND_ID).Put16(groupId).Put8(sceneId);
    COMMAND_FOOTER();
}

/*
 * Command StoreScene
 */
PacketBufferHandle encodeScenesClusterStoreSceneCommand(EndpointId destinationEndpoint, uint16_t groupId, uint8_t sceneId)
{
    COMMAND_HEADER("StoreScene", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_STORE_SCENE_COMMAND_ID).Put16(groupId).Put8(sceneId);
    COMMAND_FOOTER();
}

/*
 * Command ViewScene
 */
PacketBufferHandle encodeScenesClusterViewSceneCommand(EndpointId destinationEndpoint, uint16_t groupId, uint8_t sceneId)
{
    COMMAND_HEADER("ViewScene", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlClusterSpecificCommand).Put8(kSeqNum).Put8(ZCL_VIEW_SCENE_COMMAND_ID).Put16(groupId).Put8(sceneId);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeScenesClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverScenesAttributes", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute SceneCount
 */
PacketBufferHandle encodeScenesClusterReadSceneCountAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadScenesSceneCount", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentScene
 */
PacketBufferHandle encodeScenesClusterReadCurrentSceneAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadScenesCurrentScene", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0001);
    COMMAND_FOOTER();
}

/*
 * Attribute CurrentGroup
 */
PacketBufferHandle encodeScenesClusterReadCurrentGroupAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadScenesCurrentGroup", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0002);
    COMMAND_FOOTER();
}

/*
 * Attribute SceneValid
 */
PacketBufferHandle encodeScenesClusterReadSceneValidAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadScenesSceneValid", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0003);
    COMMAND_FOOTER();
}

/*
 * Attribute NameSupport
 */
PacketBufferHandle encodeScenesClusterReadNameSupportAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadScenesNameSupport", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0004);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeScenesClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadScenesClusterRevision", SCENES_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
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

PacketBufferHandle encodeTemperatureMeasurementClusterDiscoverAttributes(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("DiscoverTemperatureMeasurementAttributes", TEMP_MEASUREMENT_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_DISCOVER_ATTRIBUTES_COMMAND_ID).Put16(0x0000).Put8(0xFF);
    COMMAND_FOOTER();
}

/*
 * Attribute MeasuredValue
 */
PacketBufferHandle encodeTemperatureMeasurementClusterReadMeasuredValueAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadTemperatureMeasurementMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0000);
    COMMAND_FOOTER();
}

PacketBufferHandle encodeTemperatureMeasurementClusterReportMeasuredValueAttribute(EndpointId destinationEndpoint,
                                                                                   uint16_t minInterval, uint16_t maxInterval,
                                                                                   int16_t change)
{
    COMMAND_HEADER("ReportTemperatureMeasurementMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand)
        .Put8(kSeqNum)
        .Put8(ZCL_CONFIGURE_REPORTING_COMMAND_ID)
        .Put8(EMBER_ZCL_REPORTING_DIRECTION_REPORTED)
        .Put16(0x0000)
        .Put8(41)
        .Put16(minInterval)
        .Put16(maxInterval);
    buf.Put16(static_cast<uint16_t>(change));
    COMMAND_FOOTER();
}

/*
 * Attribute MinMeasuredValue
 */
PacketBufferHandle encodeTemperatureMeasurementClusterReadMinMeasuredValueAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadTemperatureMeasurementMinMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0001);
    COMMAND_FOOTER();
}

/*
 * Attribute MaxMeasuredValue
 */
PacketBufferHandle encodeTemperatureMeasurementClusterReadMaxMeasuredValueAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadTemperatureMeasurementMaxMeasuredValue", TEMP_MEASUREMENT_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0x0002);
    COMMAND_FOOTER();
}

/*
 * Attribute ClusterRevision
 */
PacketBufferHandle encodeTemperatureMeasurementClusterReadClusterRevisionAttribute(EndpointId destinationEndpoint)
{
    COMMAND_HEADER("ReadTemperatureMeasurementClusterRevision", TEMP_MEASUREMENT_CLUSTER_ID);
    buf.Put8(kFrameControlGlobalCommand).Put8(kSeqNum).Put8(ZCL_READ_ATTRIBUTES_COMMAND_ID).Put16(0xFFFD);
    COMMAND_FOOTER();
}
