
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

#pragma once

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {
namespace cluster {

namespace BarrierControl {
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
constexpr uint16_t kClusterId                         = 0x0103;
constexpr uint8_t kBarrierControlGoToPercentCommandId = 0x00;
constexpr uint8_t kBarrierControlStopCommandId        = 0x01;

void HandleBarrierControlGoToPercentCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleBarrierControlStopCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeBarrierControlGoToPercentCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, const uint8_t percentOpen);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeBarrierControlStopCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace BarrierControl

namespace Basic {
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
constexpr uint16_t kClusterId                      = 0x0000;
constexpr uint8_t kResetToFactoryDefaultsCommandId = 0x00;

void HandleResetToFactoryDefaultsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeResetToFactoryDefaultsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace Basic

namespace ColorControl {
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
constexpr uint16_t kClusterId                      = 0x0300;
constexpr uint8_t kMoveColorCommandId              = 0x08;
constexpr uint8_t kMoveColorTemperatureCommandId   = 0x4B;
constexpr uint8_t kMoveHueCommandId                = 0x01;
constexpr uint8_t kMoveSaturationCommandId         = 0x04;
constexpr uint8_t kMoveToColorCommandId            = 0x07;
constexpr uint8_t kMoveToColorTemperatureCommandId = 0x0A;
constexpr uint8_t kMoveToHueCommandId              = 0x00;
constexpr uint8_t kMoveToHueAndSaturationCommandId = 0x06;
constexpr uint8_t kMoveToSaturationCommandId       = 0x03;
constexpr uint8_t kStepColorCommandId              = 0x09;
constexpr uint8_t kStepColorTemperatureCommandId   = 0x4C;
constexpr uint8_t kStepHueCommandId                = 0x02;
constexpr uint8_t kStepSaturationCommandId         = 0x05;
constexpr uint8_t kStopMoveStepCommandId           = 0x47;

void HandleMoveColorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveColorTemperatureCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveHueCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToColorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToColorTemperatureCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToHueCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToHueAndSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStepColorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStepColorTemperatureCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStepHueCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStepSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStopMoveStepCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveColorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const int16_t rateX, const int16_t rateY, const uint8_t optionsMask,
                                  const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveColorTemperatureCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t moveMode, const uint16_t rate,
                                             const uint16_t colorTemperatureMinimum, const uint16_t colorTemperatureMaximum,
                                             const uint8_t optionsMask, const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveHueCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint8_t moveMode, const uint8_t rate, const uint8_t optionsMask,
                                const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                       const uint8_t moveMode, const uint8_t rate, const uint8_t optionsMask,
                                       const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToColorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t colorX, const uint16_t colorY, const uint16_t transitionTime,
                                    const uint8_t optionsMask, const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToColorTemperatureCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId, const uint16_t colorTemperature,
                                               const uint16_t transitionTime, const uint8_t optionsMask,
                                               const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToHueCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint8_t hue, const uint8_t direction, const uint16_t transitionTime,
                                  const uint8_t optionsMask, const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToHueAndSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId, const uint8_t hue, const uint8_t saturation,
                                               const uint16_t transitionTime, const uint8_t optionsMask,
                                               const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                         const uint8_t saturation, const uint16_t transitionTime, const uint8_t optionsMask,
                                         const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStepColorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const int16_t stepX, const int16_t stepY, const uint16_t transitionTime,
                                  const uint8_t optionsMask, const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStepColorTemperatureCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t stepMode, const uint16_t stepSize,
                                             const uint16_t transitionTime, const uint16_t colorTemperatureMinimum,
                                             const uint16_t colorTemperatureMaximum, const uint8_t optionsMask,
                                             const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStepHueCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint8_t stepMode, const uint8_t stepSize, const uint8_t transitionTime,
                                const uint8_t optionsMask, const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStepSaturationCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                       const uint8_t stepMode, const uint8_t stepSize, const uint8_t transitionTime,
                                       const uint8_t optionsMask, const uint8_t optionsOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStopMoveStepCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                     const uint8_t optionsMask, const uint8_t optionsOverride);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace ColorControl

namespace DoorLock {
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
constexpr uint16_t kClusterId                    = 0x0101;
constexpr uint8_t kClearAllPinsCommandId         = 0x08;
constexpr uint8_t kClearAllRfidsCommandId        = 0x19;
constexpr uint8_t kClearHolidayScheduleCommandId = 0x13;
constexpr uint8_t kClearPinCommandId             = 0x07;
constexpr uint8_t kClearRfidCommandId            = 0x18;
constexpr uint8_t kClearWeekdayScheduleCommandId = 0x0D;
constexpr uint8_t kClearYeardayScheduleCommandId = 0x10;
constexpr uint8_t kGetHolidayScheduleCommandId   = 0x12;
constexpr uint8_t kGetLogRecordCommandId         = 0x04;
constexpr uint8_t kGetPinCommandId               = 0x06;
constexpr uint8_t kGetRfidCommandId              = 0x17;
constexpr uint8_t kGetUserTypeCommandId          = 0x15;
constexpr uint8_t kGetWeekdayScheduleCommandId   = 0x0C;
constexpr uint8_t kGetYeardayScheduleCommandId   = 0x0F;
constexpr uint8_t kLockDoorCommandId             = 0x00;
constexpr uint8_t kSetHolidayScheduleCommandId   = 0x11;
constexpr uint8_t kSetPinCommandId               = 0x05;
constexpr uint8_t kSetRfidCommandId              = 0x16;
constexpr uint8_t kSetUserTypeCommandId          = 0x14;
constexpr uint8_t kSetWeekdayScheduleCommandId   = 0x0B;
constexpr uint8_t kSetYeardayScheduleCommandId   = 0x0E;
constexpr uint8_t kUnlockDoorCommandId           = 0x01;
constexpr uint8_t kUnlockWithTimeoutCommandId    = 0x03;

void HandleClearAllPinsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleClearAllRfidsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleClearHolidayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleClearPinCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleClearRfidCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleClearWeekdayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleClearYeardayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetHolidayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetLogRecordCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetPinCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetRfidCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetUserTypeCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetWeekdayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetYeardayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleLockDoorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleSetHolidayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleSetPinCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleSetRfidCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleSetUserTypeCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleSetWeekdayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleSetYeardayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleUnlockDoorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleUnlockWithTimeoutCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearAllPinsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearAllRfidsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearHolidayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t scheduleId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearPinCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearRfidCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearWeekdayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeClearYeardayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetHolidayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetLogRecordCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                     const uint16_t logIndex);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetPinCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                               const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetRfidCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetUserTypeCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetWeekdayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetYeardayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeLockDoorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const char * pin);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeSetHolidayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint32_t localStartTime,
                                           const uint32_t localEndTime, const uint8_t operatingModeDuringHoliday);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeSetPinCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                               const uint16_t userId, const uint8_t userStatus, const uint8_t userType, const char * pin);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeSetRfidCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                const uint16_t userId, const uint8_t userStatus, const uint8_t userType, const char * id);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeSetUserTypeCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t userId, const uint8_t userType);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeSetWeekdayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId,
                                           const uint8_t daysMask, const uint8_t startHour, const uint8_t startMinute,
                                           const uint8_t endHour, const uint8_t endMinute);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeSetYeardayScheduleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t scheduleId, const uint16_t userId,
                                           const uint32_t localStartTime, const uint32_t localEndTime);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeUnlockDoorCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                   const char * pin);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeUnlockWithTimeoutCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                          const uint16_t timeoutInSeconds, const char * pin);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace DoorLock

namespace Groups {
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
constexpr uint16_t kClusterId                     = 0x0004;
constexpr uint8_t kAddGroupCommandId              = 0x00;
constexpr uint8_t kAddGroupIfIdentifyingCommandId = 0x05;
constexpr uint8_t kGetGroupMembershipCommandId    = 0x02;
constexpr uint8_t kRemoveAllGroupsCommandId       = 0x04;
constexpr uint8_t kRemoveGroupCommandId           = 0x03;
constexpr uint8_t kViewGroupCommandId             = 0x01;

void HandleAddGroupCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleAddGroupIfIdentifyingCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetGroupMembershipCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleRemoveAllGroupsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleRemoveGroupCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleViewGroupCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeAddGroupCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t groupId, const char * groupName);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeAddGroupIfIdentifyingCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                              chip::GroupId ZCLgroupId, const uint16_t groupId, const char * groupName);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetGroupMembershipCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint8_t groupCount, const uint16_t groupList);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeRemoveAllGroupsCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeRemoveGroupCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t groupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeViewGroupCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint16_t groupId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace Groups

namespace IasZone {
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
constexpr uint16_t kClusterId = 0x0500;

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace IasZone

namespace Identify {
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
constexpr uint16_t kClusterId             = 0x0003;
constexpr uint8_t kIdentifyCommandId      = 0x00;
constexpr uint8_t kIdentifyQueryCommandId = 0x01;

void HandleIdentifyCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleIdentifyQueryCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeIdentifyCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t identifyTime);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeIdentifyQueryCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace Identify

namespace LevelControl {
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
constexpr uint16_t kClusterId                    = 0x0008;
constexpr uint8_t kMoveCommandId                 = 0x01;
constexpr uint8_t kMoveToLevelCommandId          = 0x00;
constexpr uint8_t kMoveToLevelWithOnOffCommandId = 0x04;
constexpr uint8_t kMoveWithOnOffCommandId        = 0x05;
constexpr uint8_t kStepCommandId                 = 0x02;
constexpr uint8_t kStepWithOnOffCommandId        = 0x06;
constexpr uint8_t kStopCommandId                 = 0x03;
constexpr uint8_t kStopWithOnOffCommandId        = 0x07;

void HandleMoveCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToLevelCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveToLevelWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleMoveWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStepCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStepWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStopCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStopWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                             const uint8_t moveMode, const uint8_t rate, const uint8_t optionMask, const uint8_t optionOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToLevelCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint8_t level, const uint16_t transitionTime, const uint8_t optionMask,
                                    const uint8_t optionOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveToLevelWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                             chip::GroupId ZCLgroupId, const uint8_t level, const uint16_t transitionTime);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeMoveWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                      const uint8_t moveMode, const uint8_t rate);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStepCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                             const uint8_t stepMode, const uint8_t stepSize, const uint16_t transitionTime,
                             const uint8_t optionMask, const uint8_t optionOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStepWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                      const uint8_t stepMode, const uint8_t stepSize, const uint16_t transitionTime);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStopCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                             const uint8_t optionMask, const uint8_t optionOverride);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStopWithOnOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace LevelControl

namespace OnOff {
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
constexpr uint16_t kClusterId      = 0x0006;
constexpr uint8_t kOffCommandId    = 0x00;
constexpr uint8_t kOnCommandId     = 0x01;
constexpr uint8_t kToggleCommandId = 0x02;

void HandleOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleOnCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleToggleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeOffCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeOnCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeToggleCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace OnOff

namespace Scenes {
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
constexpr uint16_t kClusterId                  = 0x0005;
constexpr uint8_t kAddSceneCommandId           = 0x00;
constexpr uint8_t kGetSceneMembershipCommandId = 0x06;
constexpr uint8_t kRecallSceneCommandId        = 0x05;
constexpr uint8_t kRemoveAllScenesCommandId    = 0x03;
constexpr uint8_t kRemoveSceneCommandId        = 0x02;
constexpr uint8_t kStoreSceneCommandId         = 0x04;
constexpr uint8_t kViewSceneCommandId          = 0x01;

void HandleAddSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleGetSceneMembershipCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleRecallSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleRemoveAllScenesCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleRemoveSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleStoreSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);
void HandleViewSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj);

// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeAddSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                 const uint16_t groupId, const uint8_t sceneId, const uint16_t transitionTime,
                                 const char * sceneName, const chip::ClusterId clusterId, const uint8_t length,
                                 const uint8_t value);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeGetSceneMembershipCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId, const uint16_t groupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeRecallSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t groupId, const uint8_t sceneId, const uint16_t transitionTime);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeRemoveAllScenesCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                        const uint16_t groupId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeRemoveSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                    const uint16_t groupId, const uint8_t sceneId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeStoreSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                   const uint16_t groupId, const uint8_t sceneId);
// The "ZCL" prefix here is used for avoiding duplicate names.
CHIP_ERROR EncodeViewSceneCommand(chip::app::Command * ZCLcommand, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                  const uint16_t groupId, const uint8_t sceneId);

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace Scenes

namespace TemperatureMeasurement {
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
constexpr uint16_t kClusterId = 0x0402;

void InitCluster(chip::app::InteractionModelEngine * ime);
void ShutdownCluster(chip::app::InteractionModelEngine * ime);

} // namespace TemperatureMeasurement

void InitClusters(chip::app::InteractionModelEngine * ime);
void ShutdownClusters(chip::app::InteractionModelEngine * ime);

} // namespace cluster
} // namespace app
} // namespace chip
