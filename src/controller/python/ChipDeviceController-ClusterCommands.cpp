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

#include <core/CHIPCore.h>

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/im-encoder.h>

// TODO: This file should be generated

using namespace chip;
using namespace chip::app;

namespace chip {
namespace app {
CommandSender * GetCommandSender();
namespace cluster {

// Empty command handlers
namespace BarrierControl {
void HandleBarrierControlGoToPercentCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleBarrierControlStopCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace BarrierControl
namespace Basic {
void HandleResetToFactoryDefaultsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace Basic
namespace ColorControl {
void HandleMoveColorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveColorTemperatureCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveHueCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToColorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToColorTemperatureCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToHueCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToHueAndSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStepColorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStepColorTemperatureCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStepHueCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStepSaturationCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStopMoveStepCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace ColorControl
namespace DoorLock {
void HandleClearAllPinsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleClearAllRfidsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleClearHolidayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleClearPinCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleClearRfidCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleClearWeekdayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleClearYeardayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetHolidayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetLogRecordCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetPinCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetRfidCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetUserTypeCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetWeekdayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetYeardayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleLockDoorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleSetHolidayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleSetPinCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleSetRfidCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleSetUserTypeCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleSetWeekdayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleSetYeardayScheduleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleUnlockDoorCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleUnlockWithTimeoutCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace DoorLock
namespace Groups {
void HandleAddGroupCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleAddGroupIfIdentifyingCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetGroupMembershipCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleRemoveAllGroupsCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleRemoveGroupCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleViewGroupCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace Groups
namespace IasZone {
} // namespace IasZone
namespace Identify {
void HandleIdentifyCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleIdentifyQueryCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace Identify
namespace LevelControl {
void HandleMoveCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToLevelCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveToLevelWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleMoveWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStepCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStepWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStopCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStopWithOnOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace LevelControl
namespace OnOff {
void HandleOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleOnCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleToggleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace OnOff
namespace Scenes {
void HandleAddSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleGetSceneMembershipCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleRecallSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleRemoveAllScenesCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleRemoveSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleStoreSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
void HandleViewSceneCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj) {}
} // namespace Scenes
namespace TemperatureMeasurement {
} // namespace TemperatureMeasurement
} // namespace cluster
} // namespace app
} // namespace chip

extern "C" {

// Cluster BarrierControl
// Cluster BarrierControl Command BarrierControlGoToPercent
CHIP_ERROR chip_ime_AppendCommand_BarrierControl_BarrierControlGoToPercent(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                           uint8_t percentOpen)
{
    return app::cluster::BarrierControl::EncodeBarrierControlGoToPercentCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                                percentOpen);
}
// Cluster BarrierControl Command BarrierControlStop
CHIP_ERROR chip_ime_AppendCommand_BarrierControl_BarrierControlStop(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::BarrierControl::EncodeBarrierControlStopCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster Basic
// Cluster Basic Command ResetToFactoryDefaults
CHIP_ERROR chip_ime_AppendCommand_Basic_ResetToFactoryDefaults(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::Basic::EncodeResetToFactoryDefaultsCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster ColorControl
// Cluster ColorControl Command MoveColor
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveColor(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, int16_t rateX,
                                                         int16_t rateY, uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveColorCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, rateX, rateY,
                                                              optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveColorTemperature
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveColorTemperature(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t moveMode, uint16_t rate,
                                                                    uint16_t colorTemperatureMinimum,
                                                                    uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                                    uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveColorTemperatureCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode,
                                                                         rate, colorTemperatureMinimum, colorTemperatureMaximum,
                                                                         optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveHue
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveHue(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t moveMode,
                                                       uint8_t rate, uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveHueCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode, rate,
                                                            optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveSaturation
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveSaturation(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t moveMode, uint8_t rate, uint8_t optionsMask,
                                                              uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveSaturationCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode, rate,
                                                                   optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveToColor
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToColor(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                           uint16_t colorX, uint16_t colorY, uint16_t transitionTime,
                                                           uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveToColorCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, colorX, colorY,
                                                                transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveToColorTemperature
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToColorTemperature(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                      uint16_t colorTemperature, uint16_t transitionTime,
                                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveToColorTemperatureCommand(
        GetCommandSender(), ZCLendpointId, ZCLgroupId, colorTemperature, transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveToHue
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToHue(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t hue,
                                                         uint8_t direction, uint16_t transitionTime, uint8_t optionsMask,
                                                         uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveToHueCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, hue, direction,
                                                              transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveToHueAndSaturation
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToHueAndSaturation(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                      uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveToHueAndSaturationCommand(
        GetCommandSender(), ZCLendpointId, ZCLgroupId, hue, saturation, transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command MoveToSaturation
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToSaturation(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                uint8_t saturation, uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeMoveToSaturationCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, saturation,
                                                                     transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command StepColor
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepColor(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, int16_t stepX,
                                                         int16_t stepY, uint16_t transitionTime, uint8_t optionsMask,
                                                         uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeStepColorCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, stepX, stepY,
                                                              transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command StepColorTemperature
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepColorTemperature(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                                                    uint16_t colorTemperatureMinimum,
                                                                    uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                                    uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeStepColorTemperatureCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode,
                                                                         stepSize, transitionTime, colorTemperatureMinimum,
                                                                         colorTemperatureMaximum, optionsMask, optionsOverride);
}
// Cluster ColorControl Command StepHue
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepHue(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t stepMode,
                                                       uint8_t stepSize, uint8_t transitionTime, uint8_t optionsMask,
                                                       uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeStepHueCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode, stepSize,
                                                            transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command StepSaturation
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepSaturation(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t stepMode, uint8_t stepSize, uint8_t transitionTime,
                                                              uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeStepSaturationCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode,
                                                                   stepSize, transitionTime, optionsMask, optionsOverride);
}
// Cluster ColorControl Command StopMoveStep
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StopMoveStep(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                            uint8_t optionsMask, uint8_t optionsOverride)
{
    return app::cluster::ColorControl::EncodeStopMoveStepCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, optionsMask,
                                                                 optionsOverride);
}
// Cluster DoorLock
// Cluster DoorLock Command ClearAllPins
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearAllPins(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::DoorLock::EncodeClearAllPinsCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster DoorLock Command ClearAllRfids
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearAllRfids(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::DoorLock::EncodeClearAllRfidsCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster DoorLock Command ClearHolidaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearHolidaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                uint8_t scheduleId)
{
    return app::cluster::DoorLock::EncodeClearHolidayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId);
}
// Cluster DoorLock Command ClearPin
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearPin(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeClearPinCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
// Cluster DoorLock Command ClearRfid
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearRfid(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeClearRfidCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
// Cluster DoorLock Command ClearWeekdaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearWeekdaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                uint8_t scheduleId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeClearWeekdayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                     userId);
}
// Cluster DoorLock Command ClearYeardaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearYeardaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                uint8_t scheduleId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeClearYeardayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                     userId);
}
// Cluster DoorLock Command GetHolidaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetHolidaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t scheduleId)
{
    return app::cluster::DoorLock::EncodeGetHolidayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId);
}
// Cluster DoorLock Command GetLogRecord
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetLogRecord(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t logIndex)
{
    return app::cluster::DoorLock::EncodeGetLogRecordCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, logIndex);
}
// Cluster DoorLock Command GetPin
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetPin(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeGetPinCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
// Cluster DoorLock Command GetRfid
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetRfid(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeGetRfidCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
// Cluster DoorLock Command GetUserType
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetUserType(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeGetUserTypeCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
// Cluster DoorLock Command GetWeekdaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetWeekdaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t scheduleId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeGetWeekdayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                   userId);
}
// Cluster DoorLock Command GetYeardaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetYeardaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t scheduleId, uint16_t userId)
{
    return app::cluster::DoorLock::EncodeGetYeardayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                   userId);
}
// Cluster DoorLock Command LockDoor
CHIP_ERROR chip_ime_AppendCommand_DoorLock_LockDoor(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, char * pin)
{
    return app::cluster::DoorLock::EncodeLockDoorCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, pin);
}
// Cluster DoorLock Command SetHolidaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetHolidaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t scheduleId, uint32_t localStartTime, uint32_t localEndTime,
                                                              uint8_t operatingModeDuringHoliday)
{
    return app::cluster::DoorLock::EncodeSetHolidayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                   localStartTime, localEndTime, operatingModeDuringHoliday);
}
// Cluster DoorLock Command SetPin
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetPin(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId,
                                                  uint8_t userStatus, uint8_t userType, char * pin)
{
    return app::cluster::DoorLock::EncodeSetPinCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId, userStatus, userType,
                                                       pin);
}
// Cluster DoorLock Command SetRfid
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetRfid(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId,
                                                   uint8_t userStatus, uint8_t userType, char * id)
{
    return app::cluster::DoorLock::EncodeSetRfidCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId, userStatus, userType,
                                                        id);
}
// Cluster DoorLock Command SetUserType
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetUserType(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t userId,
                                                       uint8_t userType)
{
    return app::cluster::DoorLock::EncodeSetUserTypeCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, userId, userType);
}
// Cluster DoorLock Command SetWeekdaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetWeekdaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t scheduleId, uint16_t userId, uint8_t daysMask,
                                                              uint8_t startHour, uint8_t startMinute, uint8_t endHour,
                                                              uint8_t endMinute)
{
    return app::cluster::DoorLock::EncodeSetWeekdayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                   userId, daysMask, startHour, startMinute, endHour, endMinute);
}
// Cluster DoorLock Command SetYeardaySchedule
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetYeardaySchedule(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                              uint8_t scheduleId, uint16_t userId, uint32_t localStartTime,
                                                              uint32_t localEndTime)
{
    return app::cluster::DoorLock::EncodeSetYeardayScheduleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, scheduleId,
                                                                   userId, localStartTime, localEndTime);
}
// Cluster DoorLock Command UnlockDoor
CHIP_ERROR chip_ime_AppendCommand_DoorLock_UnlockDoor(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, char * pin)
{
    return app::cluster::DoorLock::EncodeUnlockDoorCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, pin);
}
// Cluster DoorLock Command UnlockWithTimeout
CHIP_ERROR chip_ime_AppendCommand_DoorLock_UnlockWithTimeout(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                             uint16_t timeoutInSeconds, char * pin)
{
    return app::cluster::DoorLock::EncodeUnlockWithTimeoutCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, timeoutInSeconds,
                                                                  pin);
}
// Cluster Groups
// Cluster Groups Command AddGroup
CHIP_ERROR chip_ime_AppendCommand_Groups_AddGroup(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId,
                                                  char * groupName)
{
    return app::cluster::Groups::EncodeAddGroupCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, groupName);
}
// Cluster Groups Command AddGroupIfIdentifying
CHIP_ERROR chip_ime_AppendCommand_Groups_AddGroupIfIdentifying(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                               uint16_t groupId, char * groupName)
{
    return app::cluster::Groups::EncodeAddGroupIfIdentifyingCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId,
                                                                    groupName);
}
// Cluster Groups Command GetGroupMembership
CHIP_ERROR chip_ime_AppendCommand_Groups_GetGroupMembership(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                            uint8_t groupCount, uint16_t groupList)
{
    return app::cluster::Groups::EncodeGetGroupMembershipCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupCount,
                                                                 groupList);
}
// Cluster Groups Command RemoveAllGroups
CHIP_ERROR chip_ime_AppendCommand_Groups_RemoveAllGroups(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::Groups::EncodeRemoveAllGroupsCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster Groups Command RemoveGroup
CHIP_ERROR chip_ime_AppendCommand_Groups_RemoveGroup(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId)
{
    return app::cluster::Groups::EncodeRemoveGroupCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
// Cluster Groups Command ViewGroup
CHIP_ERROR chip_ime_AppendCommand_Groups_ViewGroup(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId)
{
    return app::cluster::Groups::EncodeViewGroupCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
// Cluster IasZone
// Cluster Identify
// Cluster Identify Command Identify
CHIP_ERROR chip_ime_AppendCommand_Identify_Identify(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t identifyTime)
{
    return app::cluster::Identify::EncodeIdentifyCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, identifyTime);
}
// Cluster Identify Command IdentifyQuery
CHIP_ERROR chip_ime_AppendCommand_Identify_IdentifyQuery(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::Identify::EncodeIdentifyQueryCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster LevelControl
// Cluster LevelControl Command Move
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Move(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t moveMode,
                                                    uint8_t rate, uint8_t optionMask, uint8_t optionOverride)
{
    return app::cluster::LevelControl::EncodeMoveCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode, rate, optionMask,
                                                         optionOverride);
}
// Cluster LevelControl Command MoveToLevel
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveToLevel(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t level,
                                                           uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    return app::cluster::LevelControl::EncodeMoveToLevelCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, level,
                                                                transitionTime, optionMask, optionOverride);
}
// Cluster LevelControl Command MoveToLevelWithOnOff
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveToLevelWithOnOff(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t level, uint16_t transitionTime)
{
    return app::cluster::LevelControl::EncodeMoveToLevelWithOnOffCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, level,
                                                                         transitionTime);
}
// Cluster LevelControl Command MoveWithOnOff
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveWithOnOff(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                             uint8_t moveMode, uint8_t rate)
{
    return app::cluster::LevelControl::EncodeMoveWithOnOffCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode, rate);
}
// Cluster LevelControl Command Step
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Step(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t stepMode,
                                                    uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask,
                                                    uint8_t optionOverride)
{
    return app::cluster::LevelControl::EncodeStepCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode, stepSize,
                                                         transitionTime, optionMask, optionOverride);
}
// Cluster LevelControl Command StepWithOnOff
CHIP_ERROR chip_ime_AppendCommand_LevelControl_StepWithOnOff(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                             uint8_t stepMode, uint8_t stepSize, uint16_t transitionTime)
{
    return app::cluster::LevelControl::EncodeStepWithOnOffCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode, stepSize,
                                                                  transitionTime);
}
// Cluster LevelControl Command Stop
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Stop(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t optionMask,
                                                    uint8_t optionOverride)
{
    return app::cluster::LevelControl::EncodeStopCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, optionMask, optionOverride);
}
// Cluster LevelControl Command StopWithOnOff
CHIP_ERROR chip_ime_AppendCommand_LevelControl_StopWithOnOff(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::LevelControl::EncodeStopWithOnOffCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster OnOff
// Cluster OnOff Command Off
CHIP_ERROR chip_ime_AppendCommand_OnOff_Off(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::OnOff::EncodeOffCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster OnOff Command On
CHIP_ERROR chip_ime_AppendCommand_OnOff_On(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::OnOff::EncodeOnCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster OnOff Command Toggle
CHIP_ERROR chip_ime_AppendCommand_OnOff_Toggle(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    return app::cluster::OnOff::EncodeToggleCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// Cluster Scenes
// Cluster Scenes Command AddScene
CHIP_ERROR chip_ime_AppendCommand_Scenes_AddScene(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId,
                                                  uint8_t sceneId, uint16_t transitionTime, char * sceneName,
                                                  chip::ClusterId clusterId, uint8_t length, uint8_t value)
{
    return app::cluster::Scenes::EncodeAddSceneCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId,
                                                       transitionTime, sceneName, clusterId, length, value);
}
// Cluster Scenes Command GetSceneMembership
CHIP_ERROR chip_ime_AppendCommand_Scenes_GetSceneMembership(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                            uint16_t groupId)
{
    return app::cluster::Scenes::EncodeGetSceneMembershipCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
// Cluster Scenes Command RecallScene
CHIP_ERROR chip_ime_AppendCommand_Scenes_RecallScene(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId,
                                                     uint8_t sceneId, uint16_t transitionTime)
{
    return app::cluster::Scenes::EncodeRecallSceneCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId,
                                                          transitionTime);
}
// Cluster Scenes Command RemoveAllScenes
CHIP_ERROR chip_ime_AppendCommand_Scenes_RemoveAllScenes(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId)
{
    return app::cluster::Scenes::EncodeRemoveAllScenesCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
// Cluster Scenes Command RemoveScene
CHIP_ERROR chip_ime_AppendCommand_Scenes_RemoveScene(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId,
                                                     uint8_t sceneId)
{
    return app::cluster::Scenes::EncodeRemoveSceneCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId);
}
// Cluster Scenes Command StoreScene
CHIP_ERROR chip_ime_AppendCommand_Scenes_StoreScene(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId,
                                                    uint8_t sceneId)
{
    return app::cluster::Scenes::EncodeStoreSceneCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId);
}
// Cluster Scenes Command ViewScene
CHIP_ERROR chip_ime_AppendCommand_Scenes_ViewScene(chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint16_t groupId,
                                                   uint8_t sceneId)
{
    return app::cluster::Scenes::EncodeViewSceneCommand(GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId);
}
// Cluster TemperatureMeasurement

CHIP_ERROR chip_ime_SendCommand(chip::NodeId nodeId)
{
    return GetCommandSender()->SendCommandRequest(nodeId);
}
}
