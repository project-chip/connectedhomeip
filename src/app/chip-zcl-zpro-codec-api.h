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

// Prevent multiple inclusion
#pragma once

#include <app/util/basic-types.h>
#include <stdint.h>
#include <system/SystemPacketBuffer.h>

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

/**
 * @brief
 *    Encode an BarrierControlGoToPercent command for Barrier Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterBarrierControlGoToPercentCommand(chip::EndpointId destinationEndpoint,
                                                                                             uint8_t percentOpen);

/**
 * @brief
 *    Encode an BarrierControlStop command for Barrier Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterBarrierControlStopCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Barrier Control server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier moving state attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterReadBarrierMovingStateAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier safety status attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterReadBarrierSafetyStatusAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier capabilities attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterReadBarrierCapabilitiesAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier position attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterReadBarrierPositionAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBarrierControlClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an MfgSpecificPing command for Basic server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBasicClusterMfgSpecificPingCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode an ResetToFactoryDefaults command for Basic server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBasicClusterResetToFactoryDefaultsCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Basic server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBasicClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Basic server read command for the ZCL version attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBasicClusterReadZclVersionAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Basic server read command for the power source attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBasicClusterReadPowerSourceAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Basic server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBasicClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an Bind command for Binding server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBindingClusterBindCommand(chip::EndpointId destinationEndpoint, chip::NodeId nodeId,
                                                                 chip::GroupId groupId, chip::EndpointId endpointId,
                                                                 chip::ClusterId clusterId);

/**
 * @brief
 *    Encode an Unbind command for Binding server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBindingClusterUnbindCommand(chip::EndpointId destinationEndpoint, chip::NodeId nodeId,
                                                                   chip::GroupId groupId, chip::EndpointId endpointId,
                                                                   chip::ClusterId clusterId);

/**
 * @brief
 *    Encode a Binding server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBindingClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Binding server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeBindingClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an MoveColor command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveColorCommand(chip::EndpointId destinationEndpoint, int16_t rateX,
                                                                           int16_t rateY, uint8_t optionsMask,
                                                                           uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveColorTemperature command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveColorTemperatureCommand(chip::EndpointId destinationEndpoint,
                                                                                      uint8_t moveMode, uint16_t rate,
                                                                                      uint16_t colorTemperatureMinimum,
                                                                                      uint16_t colorTemperatureMaximum,
                                                                                      uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveHue command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveHueCommand(chip::EndpointId destinationEndpoint, uint8_t moveMode,
                                                                         uint8_t rate, uint8_t optionsMask,
                                                                         uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveSaturation command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveSaturationCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t moveMode, uint8_t rate, uint8_t optionsMask,
                                                                                uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToColor command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveToColorCommand(chip::EndpointId destinationEndpoint, uint16_t colorX,
                                                                             uint16_t colorY, uint16_t transitionTime,
                                                                             uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToColorTemperature command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeColorControlClusterMoveToColorTemperatureCommand(chip::EndpointId destinationEndpoint, uint16_t colorTemperature,
                                                       uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToHue command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveToHueCommand(chip::EndpointId destinationEndpoint, uint8_t hue,
                                                                           uint8_t direction, uint16_t transitionTime,
                                                                           uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToHueAndSaturation command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeColorControlClusterMoveToHueAndSaturationCommand(chip::EndpointId destinationEndpoint, uint8_t hue, uint8_t saturation,
                                                       uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToSaturation command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterMoveToSaturationCommand(chip::EndpointId destinationEndpoint,
                                                                                  uint8_t saturation, uint16_t transitionTime,
                                                                                  uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepColor command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterStepColorCommand(chip::EndpointId destinationEndpoint, int16_t stepX,
                                                                           int16_t stepY, uint16_t transitionTime,
                                                                           uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepColorTemperature command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterStepColorTemperatureCommand(
    chip::EndpointId destinationEndpoint, uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
    uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepHue command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterStepHueCommand(chip::EndpointId destinationEndpoint, uint8_t stepMode,
                                                                         uint8_t stepSize, uint8_t transitionTime,
                                                                         uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepSaturation command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterStepSaturationCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t stepMode, uint8_t stepSize,
                                                                                uint8_t transitionTime, uint8_t optionsMask,
                                                                                uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StopMoveStep command for Color Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterStopMoveStepCommand(chip::EndpointId destinationEndpoint,
                                                                              uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode a Color Control server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the current hue attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadCurrentHueAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current hue attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReportCurrentHueAttribute(chip::EndpointId destinationEndpoint,
                                                                                    uint16_t minInterval, uint16_t maxInterval,
                                                                                    uint8_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the current saturation attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadCurrentSaturationAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current saturation attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReportCurrentSaturationAttribute(chip::EndpointId destinationEndpoint,
                                                                                           uint16_t minInterval,
                                                                                           uint16_t maxInterval, uint8_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the remaining time attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadRemainingTimeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the current x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadCurrentXAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReportCurrentXAttribute(chip::EndpointId destinationEndpoint,
                                                                                  uint16_t minInterval, uint16_t maxInterval,
                                                                                  uint16_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the current y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadCurrentYAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReportCurrentYAttribute(chip::EndpointId destinationEndpoint,
                                                                                  uint16_t minInterval, uint16_t maxInterval,
                                                                                  uint16_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the drift compensation attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadDriftCompensationAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the compensation text attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadCompensationTextAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color temperature attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorTemperatureAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the color temperature attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReportColorTemperatureAttribute(chip::EndpointId destinationEndpoint,
                                                                                          uint16_t minInterval,
                                                                                          uint16_t maxInterval, uint16_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the color mode attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorModeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color control options attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorControlOptionsAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color control options attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorControlOptionsAttribute(chip::EndpointId destinationEndpoint,
                                                                                            uint8_t colorControlOptions);

/**
 * @brief
 *    Encode a Color Control server read command for the number of primaries attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadNumberOfPrimariesAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 1 x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary1XAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 1 y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary1YAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 1 intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary1IntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 2 x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary2XAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 2 y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary2YAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 2 intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary2IntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 3 x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary3XAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 3 y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary3YAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 3 intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary3IntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 4 x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary4XAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 4 y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary4YAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 4 intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary4IntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 5 x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary5XAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 5 y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary5YAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 5 intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary5IntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 6 x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary6XAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 6 y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary6YAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 6 intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadPrimary6IntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the white point x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadWhitePointXAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the white point x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteWhitePointXAttribute(chip::EndpointId destinationEndpoint,
                                                                                    uint16_t whitePointX);

/**
 * @brief
 *    Encode a Color Control server read command for the white point y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadWhitePointYAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the white point y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteWhitePointYAttribute(chip::EndpointId destinationEndpoint,
                                                                                    uint16_t whitePointY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point r x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointRXAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point r x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointRXAttribute(chip::EndpointId destinationEndpoint,
                                                                                     uint16_t colorPointRX);

/**
 * @brief
 *    Encode a Color Control server read command for the color point r y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointRYAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point r y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointRYAttribute(chip::EndpointId destinationEndpoint,
                                                                                     uint16_t colorPointRY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point r intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointRIntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point r intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointRIntensityAttribute(chip::EndpointId destinationEndpoint,
                                                                                             uint8_t colorPointRIntensity);

/**
 * @brief
 *    Encode a Color Control server read command for the color point g x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointGXAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point g x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointGXAttribute(chip::EndpointId destinationEndpoint,
                                                                                     uint16_t colorPointGX);

/**
 * @brief
 *    Encode a Color Control server read command for the color point g y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointGYAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point g y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointGYAttribute(chip::EndpointId destinationEndpoint,
                                                                                     uint16_t colorPointGY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point g intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointGIntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point g intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointGIntensityAttribute(chip::EndpointId destinationEndpoint,
                                                                                             uint8_t colorPointGIntensity);

/**
 * @brief
 *    Encode a Color Control server read command for the color point b x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointBXAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point b x attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointBXAttribute(chip::EndpointId destinationEndpoint,
                                                                                     uint16_t colorPointBX);

/**
 * @brief
 *    Encode a Color Control server read command for the color point b y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointBYAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point b y attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointBYAttribute(chip::EndpointId destinationEndpoint,
                                                                                     uint16_t colorPointBY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point b intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorPointBIntensityAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point b intensity attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterWriteColorPointBIntensityAttribute(chip::EndpointId destinationEndpoint,
                                                                                             uint8_t colorPointBIntensity);

/**
 * @brief
 *    Encode a Color Control server read command for the enhanced current hue attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadEnhancedCurrentHueAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the enhanced color mode attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadEnhancedColorModeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color loop active attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorLoopActiveAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color loop direction attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorLoopDirectionAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color loop time attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorLoopTimeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color capabilities attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorCapabilitiesAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color temp physical min attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorTempPhysicalMinAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color temp physical max attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadColorTempPhysicalMaxAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the couple color temp to level min-mireds attribute into buffer including the
 * APS frame
 */
chip::System::PacketBufferHandle
encodeColorControlClusterReadCoupleColorTempToLevelMinMiredsAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the start up color temperature mireds attribute into buffer including the APS
 * frame
 */
chip::System::PacketBufferHandle
encodeColorControlClusterReadStartUpColorTemperatureMiredsAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the start up color temperature mireds attribute into buffer including the APS
 * frame
 */
chip::System::PacketBufferHandle
encodeColorControlClusterWriteStartUpColorTemperatureMiredsAttribute(chip::EndpointId destinationEndpoint,
                                                                     uint16_t startUpColorTemperatureMireds);

/**
 * @brief
 *    Encode a Color Control server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeColorControlClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an ClearAllPins command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearAllPinsCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode an ClearAllRfids command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearAllRfidsCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode an ClearHolidaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearHolidayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                  uint8_t scheduleId);

/**
 * @brief
 *    Encode an ClearPin command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearPinCommand(chip::EndpointId destinationEndpoint, uint16_t userId);

/**
 * @brief
 *    Encode an ClearRfid command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearRfidCommand(chip::EndpointId destinationEndpoint, uint16_t userId);

/**
 * @brief
 *    Encode an ClearWeekdaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearWeekdayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                  uint8_t scheduleId, uint16_t userId);

/**
 * @brief
 *    Encode an ClearYeardaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterClearYeardayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                  uint8_t scheduleId, uint16_t userId);

/**
 * @brief
 *    Encode an GetHolidaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetHolidayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t scheduleId);

/**
 * @brief
 *    Encode an GetLogRecord command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetLogRecordCommand(chip::EndpointId destinationEndpoint, uint16_t logIndex);

/**
 * @brief
 *    Encode an GetPin command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetPinCommand(chip::EndpointId destinationEndpoint, uint16_t userId);

/**
 * @brief
 *    Encode an GetRfid command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetRfidCommand(chip::EndpointId destinationEndpoint, uint16_t userId);

/**
 * @brief
 *    Encode an GetUserType command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetUserTypeCommand(chip::EndpointId destinationEndpoint, uint16_t userId);

/**
 * @brief
 *    Encode an GetWeekdaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetWeekdayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t scheduleId, uint16_t userId);

/**
 * @brief
 *    Encode an GetYeardaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterGetYeardayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t scheduleId, uint16_t userId);

/**
 * @brief
 *    Encode an LockDoor command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterLockDoorCommand(chip::EndpointId destinationEndpoint, char * pin);

/**
 * @brief
 *    Encode an SetHolidaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterSetHolidayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t scheduleId, uint32_t localStartTime,
                                                                                uint32_t localEndTime,
                                                                                uint8_t operatingModeDuringHoliday);

/**
 * @brief
 *    Encode an SetPin command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterSetPinCommand(chip::EndpointId destinationEndpoint, uint16_t userId,
                                                                    uint8_t userStatus, uint8_t userType, char * pin);

/**
 * @brief
 *    Encode an SetRfid command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterSetRfidCommand(chip::EndpointId destinationEndpoint, uint16_t userId,
                                                                     uint8_t userStatus, uint8_t userType, char * id);

/**
 * @brief
 *    Encode an SetUserType command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterSetUserTypeCommand(chip::EndpointId destinationEndpoint, uint16_t userId,
                                                                         uint8_t userType);

/**
 * @brief
 *    Encode an SetWeekdaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterSetWeekdayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t scheduleId, uint16_t userId,
                                                                                uint8_t daysMask, uint8_t startHour,
                                                                                uint8_t startMinute, uint8_t endHour,
                                                                                uint8_t endMinute);

/**
 * @brief
 *    Encode an SetYeardaySchedule command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterSetYeardayScheduleCommand(chip::EndpointId destinationEndpoint,
                                                                                uint8_t scheduleId, uint16_t userId,
                                                                                uint32_t localStartTime, uint32_t localEndTime);

/**
 * @brief
 *    Encode an UnlockDoor command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterUnlockDoorCommand(chip::EndpointId destinationEndpoint, char * pin);

/**
 * @brief
 *    Encode an UnlockWithTimeout command for Door Lock server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterUnlockWithTimeoutCommand(chip::EndpointId destinationEndpoint,
                                                                               uint16_t timeoutInSeconds, char * pin);

/**
 * @brief
 *    Encode a Door Lock server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Door Lock server read command for the lock state attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterReadLockStateAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Door Lock server report command for the lock state attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterReportLockStateAttribute(chip::EndpointId destinationEndpoint,
                                                                               uint16_t minInterval, uint16_t maxInterval);

/**
 * @brief
 *    Encode a Door Lock server read command for the lock type attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterReadLockTypeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Door Lock server read command for the actuator enabled attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterReadActuatorEnabledAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Door Lock server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeDoorLockClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an AddGroup command for Groups server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterAddGroupCommand(chip::EndpointId destinationEndpoint, uint16_t groupId,
                                                                    char * groupName);

/**
 * @brief
 *    Encode an AddGroupIfIdentifying command for Groups server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterAddGroupIfIdentifyingCommand(chip::EndpointId destinationEndpoint,
                                                                                 uint16_t groupId, char * groupName);

/**
 * @brief
 *    Encode an GetGroupMembership command for Groups server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterGetGroupMembershipCommand(chip::EndpointId destinationEndpoint,
                                                                              uint8_t groupCount, uint16_t groupList);

/**
 * @brief
 *    Encode an RemoveAllGroups command for Groups server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterRemoveAllGroupsCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode an RemoveGroup command for Groups server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterRemoveGroupCommand(chip::EndpointId destinationEndpoint, uint16_t groupId);

/**
 * @brief
 *    Encode an ViewGroup command for Groups server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterViewGroupCommand(chip::EndpointId destinationEndpoint, uint16_t groupId);

/**
 * @brief
 *    Encode a Groups server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Groups server read command for the name support attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterReadNameSupportAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Groups server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeGroupsClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode a IAS Zone server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the zone state attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterReadZoneStateAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the zone type attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterReadZoneTypeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the zone status attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterReadZoneStatusAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the IAS CIE address attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterReadIasCieAddressAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a IAS Zone server write command for the IAS CIE address attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterWriteIasCieAddressAttribute(chip::EndpointId destinationEndpoint,
                                                                                 uint64_t iasCieAddress);

/**
 * @brief
 *    Encode a IAS Zone server read command for the Zone ID attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterReadZoneIdAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIasZoneClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an Identify command for Identify server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIdentifyClusterIdentifyCommand(chip::EndpointId destinationEndpoint, uint16_t identifyTime);

/**
 * @brief
 *    Encode an IdentifyQuery command for Identify server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIdentifyClusterIdentifyQueryCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Identify server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIdentifyClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Identify server read command for the identify time attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIdentifyClusterReadIdentifyTimeAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Identify server write command for the identify time attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIdentifyClusterWriteIdentifyTimeAttribute(chip::EndpointId destinationEndpoint,
                                                                                 uint16_t identifyTime);

/**
 * @brief
 *    Encode a Identify server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeIdentifyClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an Move command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterMoveCommand(chip::EndpointId destinationEndpoint, uint8_t moveMode,
                                                                      uint8_t rate, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief
 *    Encode an MoveToLevel command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterMoveToLevelCommand(chip::EndpointId destinationEndpoint, uint8_t level,
                                                                             uint16_t transitionTime, uint8_t optionMask,
                                                                             uint8_t optionOverride);

/**
 * @brief
 *    Encode an MoveToLevelWithOnOff command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterMoveToLevelWithOnOffCommand(chip::EndpointId destinationEndpoint,
                                                                                      uint8_t level, uint16_t transitionTime);

/**
 * @brief
 *    Encode an MoveWithOnOff command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterMoveWithOnOffCommand(chip::EndpointId destinationEndpoint,
                                                                               uint8_t moveMode, uint8_t rate);

/**
 * @brief
 *    Encode an Step command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterStepCommand(chip::EndpointId destinationEndpoint, uint8_t stepMode,
                                                                      uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                                                      uint8_t optionOverride);

/**
 * @brief
 *    Encode an StepWithOnOff command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterStepWithOnOffCommand(chip::EndpointId destinationEndpoint,
                                                                               uint8_t stepMode, uint8_t stepSize,
                                                                               uint16_t transitionTime);

/**
 * @brief
 *    Encode an Stop command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterStopCommand(chip::EndpointId destinationEndpoint, uint8_t optionMask,
                                                                      uint8_t optionOverride);

/**
 * @brief
 *    Encode an StopWithOnOff command for Level Control server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterStopWithOnOffCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Level Control server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Level Control server read command for the current level attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterReadCurrentLevelAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Level Control server report command for the current level attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterReportCurrentLevelAttribute(chip::EndpointId destinationEndpoint,
                                                                                      uint16_t minInterval, uint16_t maxInterval,
                                                                                      uint8_t change);

/**
 * @brief
 *    Encode a Level Control server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeLevelControlClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an Off command for On/off server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterOffCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode an On command for On/off server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterOnCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode an Toggle command for On/off server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterToggleCommand(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a On/off server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a On/off server read command for the on/off attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterReadOnOffAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a On/off server report command for the on/off attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterReportOnOffAttribute(chip::EndpointId destinationEndpoint, uint16_t minInterval,
                                                                        uint16_t maxInterval);

/**
 * @brief
 *    Encode a On/off server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeOnOffClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode an AddScene command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterAddSceneCommand(chip::EndpointId destinationEndpoint, uint16_t groupId,
                                                                    uint8_t sceneId, uint16_t transitionTime, char * sceneName,
                                                                    chip::ClusterId clusterId, uint8_t length, uint8_t value);

/**
 * @brief
 *    Encode an GetSceneMembership command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterGetSceneMembershipCommand(chip::EndpointId destinationEndpoint,
                                                                              uint16_t groupId);

/**
 * @brief
 *    Encode an RecallScene command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterRecallSceneCommand(chip::EndpointId destinationEndpoint, uint16_t groupId,
                                                                       uint8_t sceneId, uint16_t transitionTime);

/**
 * @brief
 *    Encode an RemoveAllScenes command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterRemoveAllScenesCommand(chip::EndpointId destinationEndpoint, uint16_t groupId);

/**
 * @brief
 *    Encode an RemoveScene command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterRemoveSceneCommand(chip::EndpointId destinationEndpoint, uint16_t groupId,
                                                                       uint8_t sceneId);

/**
 * @brief
 *    Encode an StoreScene command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterStoreSceneCommand(chip::EndpointId destinationEndpoint, uint16_t groupId,
                                                                      uint8_t sceneId);

/**
 * @brief
 *    Encode an ViewScene command for Scenes server into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterViewSceneCommand(chip::EndpointId destinationEndpoint, uint16_t groupId,
                                                                     uint8_t sceneId);

/**
 * @brief
 *    Encode a Scenes server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the scene count attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterReadSceneCountAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the current scene attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterReadCurrentSceneAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the current group attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterReadCurrentGroupAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the scene valid attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterReadSceneValidAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the name support attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterReadNameSupportAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeScenesClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);

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

/**
 * @brief
 *    Encode a Temperature Measurement server discover command into buffer including the APS frame
 */
chip::System::PacketBufferHandle encodeTemperatureMeasurementClusterDiscoverAttributes(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the measured value attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeTemperatureMeasurementClusterReadMeasuredValueAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server report command for the measured value attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeTemperatureMeasurementClusterReportMeasuredValueAttribute(chip::EndpointId destinationEndpoint, uint16_t minInterval,
                                                                uint16_t maxInterval, int16_t change);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the min measured value attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeTemperatureMeasurementClusterReadMinMeasuredValueAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the max measured value attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeTemperatureMeasurementClusterReadMaxMeasuredValueAttribute(chip::EndpointId destinationEndpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the cluster revision attribute into buffer including the APS frame
 */
chip::System::PacketBufferHandle
encodeTemperatureMeasurementClusterReadClusterRevisionAttribute(chip::EndpointId destinationEndpoint);
