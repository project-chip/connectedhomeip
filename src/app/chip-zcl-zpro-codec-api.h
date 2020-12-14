
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

#ifdef __cplusplus
extern "C" {
#endif

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
uint16_t encodeBarrierControlClusterBarrierControlGoToPercentCommand(uint8_t * buffer, uint16_t buf_length,
                                                                     chip::EndpointId destination_endpoint, uint8_t percentOpen);

/**
 * @brief
 *    Encode an BarrierControlStop command for Barrier Control server into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterBarrierControlStopCommand(uint8_t * buffer, uint16_t buf_length,
                                                              chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Barrier Control server discover command into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length,
                                                       chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier moving state attribute into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterReadBarrierMovingStateAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier safety status attribute into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterReadBarrierSafetyStatusAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier capabilities attribute into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterReadBarrierCapabilitiesAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the barrier position attribute into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterReadBarrierPositionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Barrier Control server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeBarrierControlClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

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
uint16_t encodeBasicClusterMfgSpecificPingCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode an ResetToFactoryDefaults command for Basic server into buffer including the APS frame
 */
uint16_t encodeBasicClusterResetToFactoryDefaultsCommand(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Basic server discover command into buffer including the APS frame
 */
uint16_t encodeBasicClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Basic server read command for the ZCL version attribute into buffer including the APS frame
 */
uint16_t encodeBasicClusterReadZclVersionAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Basic server read command for the power source attribute into buffer including the APS frame
 */
uint16_t encodeBasicClusterReadPowerSourceAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Basic server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeBasicClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint);

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
uint16_t encodeColorControlClusterMoveColorCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                   int16_t rateX, int16_t rateY, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveColorTemperature command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length,
                                                              chip::EndpointId destination_endpoint, uint8_t moveMode,
                                                              uint16_t rate, uint16_t colorTemperatureMinimum,
                                                              uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                              uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveHue command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveHueCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                 uint8_t moveMode, uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveSaturation command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveSaturationCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t moveMode, uint8_t rate,
                                                        uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToColor command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveToColorCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                     uint16_t colorX, uint16_t colorY, uint16_t transitionTime, uint8_t optionsMask,
                                                     uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToColorTemperature command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveToColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length,
                                                                chip::EndpointId destination_endpoint, uint16_t colorTemperature,
                                                                uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToHue command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveToHueCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                   uint8_t hue, uint8_t direction, uint16_t transitionTime, uint8_t optionsMask,
                                                   uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToHueAndSaturation command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveToHueAndSaturationCommand(uint8_t * buffer, uint16_t buf_length,
                                                                chip::EndpointId destination_endpoint, uint8_t hue,
                                                                uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride);

/**
 * @brief
 *    Encode an MoveToSaturation command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterMoveToSaturationCommand(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint, uint8_t saturation,
                                                          uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepColor command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterStepColorCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                   int16_t stepX, int16_t stepY, uint16_t transitionTime, uint8_t optionsMask,
                                                   uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepColorTemperature command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterStepColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length,
                                                              chip::EndpointId destination_endpoint, uint8_t stepMode,
                                                              uint16_t stepSize, uint16_t transitionTime,
                                                              uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum,
                                                              uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepHue command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterStepHueCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                 uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime, uint8_t optionsMask,
                                                 uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StepSaturation command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterStepSaturationCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t stepMode, uint8_t stepSize,
                                                        uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode an StopMoveStep command for Color Control server into buffer including the APS frame
 */
uint16_t encodeColorControlClusterStopMoveStepCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                      uint8_t optionsMask, uint8_t optionsOverride);

/**
 * @brief
 *    Encode a Color Control server discover command into buffer including the APS frame
 */
uint16_t encodeColorControlClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the current hue attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadCurrentHueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current hue attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReportCurrentHueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint, uint16_t min_interval,
                                                            uint16_t max_interval, uint8_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the current saturation attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadCurrentSaturationAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current saturation attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReportCurrentSaturationAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                   chip::EndpointId destination_endpoint, uint16_t min_interval,
                                                                   uint16_t max_interval, uint8_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the remaining time attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadRemainingTimeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the current x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadCurrentXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReportCurrentXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint, uint16_t min_interval,
                                                          uint16_t max_interval, uint16_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the current y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadCurrentYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the current y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReportCurrentYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint, uint16_t min_interval,
                                                          uint16_t max_interval, uint16_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the drift compensation attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadDriftCompensationAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the compensation text attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadCompensationTextAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color temperature attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorTemperatureAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server report command for the color temperature attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReportColorTemperatureAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                  chip::EndpointId destination_endpoint, uint16_t min_interval,
                                                                  uint16_t max_interval, uint16_t change);

/**
 * @brief
 *    Encode a Color Control server read command for the color mode attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorModeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color control options attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorControlOptionsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                   chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color control options attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorControlOptionsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint,
                                                                    uint8_t colorControlOptions);

/**
 * @brief
 *    Encode a Color Control server read command for the number of primaries attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadNumberOfPrimariesAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 1 x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary1XAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 1 y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary1YAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 1 intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary1IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 2 x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary2XAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 2 y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary2YAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 2 intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary2IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 3 x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary3XAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 3 y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary3YAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 3 intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary3IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 4 x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary4XAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 4 y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary4YAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 4 intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary4IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 5 x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary5XAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 5 y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary5YAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 5 intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary5IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 6 x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary6XAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 6 y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary6YAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the primary 6 intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadPrimary6IntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the white point x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadWhitePointXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                           chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the white point x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteWhitePointXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint, uint16_t whitePointX);

/**
 * @brief
 *    Encode a Color Control server read command for the white point y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadWhitePointYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                           chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the white point y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteWhitePointYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint, uint16_t whitePointY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point r x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointRXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point r x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointRXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint, uint16_t colorPointRX);

/**
 * @brief
 *    Encode a Color Control server read command for the color point r y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointRYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point r y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointRYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint, uint16_t colorPointRY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point r intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointRIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point r intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointRIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     chip::EndpointId destination_endpoint,
                                                                     uint8_t colorPointRIntensity);

/**
 * @brief
 *    Encode a Color Control server read command for the color point g x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointGXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point g x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointGXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint, uint16_t colorPointGX);

/**
 * @brief
 *    Encode a Color Control server read command for the color point g y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointGYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point g y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointGYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint, uint16_t colorPointGY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point g intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointGIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point g intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointGIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     chip::EndpointId destination_endpoint,
                                                                     uint8_t colorPointGIntensity);

/**
 * @brief
 *    Encode a Color Control server read command for the color point b x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointBXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point b x attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointBXAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint, uint16_t colorPointBX);

/**
 * @brief
 *    Encode a Color Control server read command for the color point b y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointBYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point b y attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointBYAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint, uint16_t colorPointBY);

/**
 * @brief
 *    Encode a Color Control server read command for the color point b intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorPointBIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the color point b intensity attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterWriteColorPointBIntensityAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                     chip::EndpointId destination_endpoint,
                                                                     uint8_t colorPointBIntensity);

/**
 * @brief
 *    Encode a Color Control server read command for the enhanced current hue attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadEnhancedCurrentHueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                  chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the enhanced color mode attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadEnhancedColorModeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color loop active attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorLoopActiveAttribute(uint8_t * buffer, uint16_t buf_length,
                                                               chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color loop direction attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorLoopDirectionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                  chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color loop time attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorLoopTimeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                             chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color capabilities attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorCapabilitiesAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                 chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color temp physical min attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorTempPhysicalMinAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the color temp physical max attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadColorTempPhysicalMaxAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                    chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the couple color temp to level min-mireds attribute into buffer including the
 * APS frame
 */
uint16_t encodeColorControlClusterReadCoupleColorTempToLevelMinMiredsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                               chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server read command for the start up color temperature mireds attribute into buffer including the APS
 * frame
 */
uint16_t encodeColorControlClusterReadStartUpColorTemperatureMiredsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                             chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Color Control server write command for the start up color temperature mireds attribute into buffer including the APS
 * frame
 */
uint16_t encodeColorControlClusterWriteStartUpColorTemperatureMiredsAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                              chip::EndpointId destination_endpoint,
                                                                              uint16_t startUpColorTemperatureMireds);

/**
 * @brief
 *    Encode a Color Control server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeColorControlClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                               chip::EndpointId destination_endpoint);

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
uint16_t encodeDoorLockClusterClearAllPinsCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode an ClearAllRfids command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterClearAllRfidsCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode an ClearHolidaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterClearHolidayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint, uint8_t scheduleId);

/**
 * @brief
 *    Encode an ClearPin command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterClearPinCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              uint16_t userId);

/**
 * @brief
 *    Encode an ClearRfid command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterClearRfidCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                               uint16_t userId);

/**
 * @brief
 *    Encode an ClearWeekdaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterClearWeekdayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint, uint8_t scheduleId,
                                                          uint16_t userId);

/**
 * @brief
 *    Encode an ClearYeardaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterClearYeardayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint, uint8_t scheduleId,
                                                          uint16_t userId);

/**
 * @brief
 *    Encode an GetHolidaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetHolidayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t scheduleId);

/**
 * @brief
 *    Encode an GetLogRecord command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetLogRecordCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                  uint16_t logIndex);

/**
 * @brief
 *    Encode an GetPin command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetPinCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                            uint16_t userId);

/**
 * @brief
 *    Encode an GetRfid command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetRfidCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                             uint16_t userId);

/**
 * @brief
 *    Encode an GetUserType command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetUserTypeCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                 uint16_t userId);

/**
 * @brief
 *    Encode an GetWeekdaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetWeekdayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t scheduleId, uint16_t userId);

/**
 * @brief
 *    Encode an GetYeardaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterGetYeardayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t scheduleId, uint16_t userId);

/**
 * @brief
 *    Encode an LockDoor command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterLockDoorCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              char * pin);

/**
 * @brief
 *    Encode an SetHolidaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterSetHolidayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t scheduleId,
                                                        uint32_t localStartTime, uint32_t localEndTime,
                                                        uint8_t operatingModeDuringHoliday);

/**
 * @brief
 *    Encode an SetPin command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterSetPinCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                            uint16_t userId, uint8_t userStatus, uint8_t userType, char * pin);

/**
 * @brief
 *    Encode an SetRfid command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterSetRfidCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                             uint16_t userId, uint8_t userStatus, uint8_t userType, char * id);

/**
 * @brief
 *    Encode an SetUserType command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterSetUserTypeCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                 uint16_t userId, uint8_t userType);

/**
 * @brief
 *    Encode an SetWeekdaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterSetWeekdayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t scheduleId, uint16_t userId,
                                                        uint8_t daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                                        uint8_t endMinute);

/**
 * @brief
 *    Encode an SetYeardaySchedule command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterSetYeardayScheduleCommand(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint, uint8_t scheduleId, uint16_t userId,
                                                        uint32_t localStartTime, uint32_t localEndTime);

/**
 * @brief
 *    Encode an UnlockDoor command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterUnlockDoorCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                char * pin);

/**
 * @brief
 *    Encode an UnlockWithTimeout command for Door Lock server into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterUnlockWithTimeoutCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                       uint16_t timeoutInSeconds, char * pin);

/**
 * @brief
 *    Encode a Door Lock server discover command into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Door Lock server read command for the lock state attribute into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterReadLockStateAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Door Lock server report command for the lock state attribute into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterReportLockStateAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                       uint16_t min_interval, uint16_t max_interval);

/**
 * @brief
 *    Encode a Door Lock server read command for the lock type attribute into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterReadLockTypeAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Door Lock server read command for the actuator enabled attribute into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterReadActuatorEnabledAttribute(uint8_t * buffer, uint16_t buf_length,
                                                           chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Door Lock server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeDoorLockClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                           chip::EndpointId destination_endpoint);

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
uint16_t encodeGroupsClusterAddGroupCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                            uint16_t groupId, char * groupName);

/**
 * @brief
 *    Encode an AddGroupIfIdentifying command for Groups server into buffer including the APS frame
 */
uint16_t encodeGroupsClusterAddGroupIfIdentifyingCommand(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint, uint16_t groupId, char * groupName);

/**
 * @brief
 *    Encode an GetGroupMembership command for Groups server into buffer including the APS frame
 */
uint16_t encodeGroupsClusterGetGroupMembershipCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                      uint8_t groupCount, uint16_t groupList);

/**
 * @brief
 *    Encode an RemoveAllGroups command for Groups server into buffer including the APS frame
 */
uint16_t encodeGroupsClusterRemoveAllGroupsCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode an RemoveGroup command for Groups server into buffer including the APS frame
 */
uint16_t encodeGroupsClusterRemoveGroupCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                               uint16_t groupId);

/**
 * @brief
 *    Encode an ViewGroup command for Groups server into buffer including the APS frame
 */
uint16_t encodeGroupsClusterViewGroupCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                             uint16_t groupId);

/**
 * @brief
 *    Encode a Groups server discover command into buffer including the APS frame
 */
uint16_t encodeGroupsClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Groups server read command for the name support attribute into buffer including the APS frame
 */
uint16_t encodeGroupsClusterReadNameSupportAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Groups server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeGroupsClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

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
uint16_t encodeIasZoneClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the zone state attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterReadZoneStateAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the zone type attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterReadZoneTypeAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the zone status attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterReadZoneStatusAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the IAS CIE address attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterReadIasCieAddressAttribute(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a IAS Zone server write command for the IAS CIE address attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterWriteIasCieAddressAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint, uint64_t iasCieAddress);

/**
 * @brief
 *    Encode a IAS Zone server read command for the Zone ID attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterReadZoneIdAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a IAS Zone server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeIasZoneClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                          chip::EndpointId destination_endpoint);

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
uint16_t encodeIdentifyClusterIdentifyCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              uint16_t identifyTime);

/**
 * @brief
 *    Encode an IdentifyQuery command for Identify server into buffer including the APS frame
 */
uint16_t encodeIdentifyClusterIdentifyQueryCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Identify server discover command into buffer including the APS frame
 */
uint16_t encodeIdentifyClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Identify server read command for the identify time attribute into buffer including the APS frame
 */
uint16_t encodeIdentifyClusterReadIdentifyTimeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Identify server write command for the identify time attribute into buffer including the APS frame
 */
uint16_t encodeIdentifyClusterWriteIdentifyTimeAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint, uint16_t identifyTime);

/**
 * @brief
 *    Encode a Identify server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeIdentifyClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                           chip::EndpointId destination_endpoint);

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
uint16_t encodeLevelControlClusterMoveCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              uint8_t moveMode, uint8_t rate, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief
 *    Encode an MoveToLevel command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterMoveToLevelCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                     uint8_t level, uint16_t transitionTime, uint8_t optionMask,
                                                     uint8_t optionOverride);

/**
 * @brief
 *    Encode an MoveToLevelWithOnOff command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterMoveToLevelWithOnOffCommand(uint8_t * buffer, uint16_t buf_length,
                                                              chip::EndpointId destination_endpoint, uint8_t level,
                                                              uint16_t transitionTime);

/**
 * @brief
 *    Encode an MoveWithOnOff command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterMoveWithOnOffCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                       uint8_t moveMode, uint8_t rate);

/**
 * @brief
 *    Encode an Step command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterStepCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                              uint8_t optionOverride);

/**
 * @brief
 *    Encode an StepWithOnOff command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterStepWithOnOffCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                       uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime);

/**
 * @brief
 *    Encode an Stop command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterStopCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief
 *    Encode an StopWithOnOff command for Level Control server into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterStopWithOnOffCommand(uint8_t * buffer, uint16_t buf_length,
                                                       chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Level Control server discover command into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Level Control server read command for the current level attribute into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterReadCurrentLevelAttribute(uint8_t * buffer, uint16_t buf_length,
                                                            chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Level Control server report command for the current level attribute into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterReportCurrentLevelAttribute(uint8_t * buffer, uint16_t buf_length,
                                                              chip::EndpointId destination_endpoint, uint16_t min_interval,
                                                              uint16_t max_interval, uint8_t change);

/**
 * @brief
 *    Encode a Level Control server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeLevelControlClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                               chip::EndpointId destination_endpoint);

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
uint16_t encodeOnOffClusterOffCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode an On command for On/off server into buffer including the APS frame
 */
uint16_t encodeOnOffClusterOnCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode an Toggle command for On/off server into buffer including the APS frame
 */
uint16_t encodeOnOffClusterToggleCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a On/off server discover command into buffer including the APS frame
 */
uint16_t encodeOnOffClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a On/off server read command for the on/off attribute into buffer including the APS frame
 */
uint16_t encodeOnOffClusterReadOnOffAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a On/off server report command for the on/off attribute into buffer including the APS frame
 */
uint16_t encodeOnOffClusterReportOnOffAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                uint16_t min_interval, uint16_t max_interval);

/**
 * @brief
 *    Encode a On/off server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeOnOffClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                        chip::EndpointId destination_endpoint);

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
uint16_t encodeScenesClusterAddSceneCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                            uint16_t groupId, uint8_t sceneId, uint16_t transitionTime, char * sceneName,
                                            chip::ClusterId clusterId, uint8_t length, uint8_t value);

/**
 * @brief
 *    Encode an GetSceneMembership command for Scenes server into buffer including the APS frame
 */
uint16_t encodeScenesClusterGetSceneMembershipCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                      uint16_t groupId);

/**
 * @brief
 *    Encode an RecallScene command for Scenes server into buffer including the APS frame
 */
uint16_t encodeScenesClusterRecallSceneCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                               uint16_t groupId, uint8_t sceneId, uint16_t transitionTime);

/**
 * @brief
 *    Encode an RemoveAllScenes command for Scenes server into buffer including the APS frame
 */
uint16_t encodeScenesClusterRemoveAllScenesCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                                   uint16_t groupId);

/**
 * @brief
 *    Encode an RemoveScene command for Scenes server into buffer including the APS frame
 */
uint16_t encodeScenesClusterRemoveSceneCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                               uint16_t groupId, uint8_t sceneId);

/**
 * @brief
 *    Encode an StoreScene command for Scenes server into buffer including the APS frame
 */
uint16_t encodeScenesClusterStoreSceneCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                              uint16_t groupId, uint8_t sceneId);

/**
 * @brief
 *    Encode an ViewScene command for Scenes server into buffer including the APS frame
 */
uint16_t encodeScenesClusterViewSceneCommand(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint,
                                             uint16_t groupId, uint8_t sceneId);

/**
 * @brief
 *    Encode a Scenes server discover command into buffer including the APS frame
 */
uint16_t encodeScenesClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the scene count attribute into buffer including the APS frame
 */
uint16_t encodeScenesClusterReadSceneCountAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the current scene attribute into buffer including the APS frame
 */
uint16_t encodeScenesClusterReadCurrentSceneAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the current group attribute into buffer including the APS frame
 */
uint16_t encodeScenesClusterReadCurrentGroupAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the scene valid attribute into buffer including the APS frame
 */
uint16_t encodeScenesClusterReadSceneValidAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the name support attribute into buffer including the APS frame
 */
uint16_t encodeScenesClusterReadNameSupportAttribute(uint8_t * buffer, uint16_t buf_length, chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Scenes server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeScenesClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                         chip::EndpointId destination_endpoint);

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
uint16_t encodeTemperatureMeasurementClusterDiscoverAttributes(uint8_t * buffer, uint16_t buf_length,
                                                               chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the measured value attribute into buffer including the APS frame
 */
uint16_t encodeTemperatureMeasurementClusterReadMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                       chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server report command for the measured value attribute into buffer including the APS frame
 */
uint16_t encodeTemperatureMeasurementClusterReportMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                         chip::EndpointId destination_endpoint,
                                                                         uint16_t min_interval, uint16_t max_interval,
                                                                         int16_t change);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the min measured value attribute into buffer including the APS frame
 */
uint16_t encodeTemperatureMeasurementClusterReadMinMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                          chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the max measured value attribute into buffer including the APS frame
 */
uint16_t encodeTemperatureMeasurementClusterReadMaxMeasuredValueAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                          chip::EndpointId destination_endpoint);

/**
 * @brief
 *    Encode a Temperature Measurement server read command for the cluster revision attribute into buffer including the APS frame
 */
uint16_t encodeTemperatureMeasurementClusterReadClusterRevisionAttribute(uint8_t * buffer, uint16_t buf_length,
                                                                         chip::EndpointId destination_endpoint);

#ifdef __cplusplus
} // extern "C"
#endif
