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

#include <core/CHIPCore.h>

#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>
#include <app/im-encoder.h>

#include <controller/CHIPDevice.h>
#include <controller/CHIPDeviceController_deprecated.h>

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
CHIP_ERROR chip_ime_AppendCommand_BarrierControl_BarrierControlGoToPercent(chip::Controller::Device * device,
                                                                           chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                           uint8_t percentOpen)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::BarrierControl::EncodeBarrierControlGoToPercentCommand(device->GetCommandSender(), ZCLendpointId,
                                                                                ZCLgroupId, percentOpen);
}
CHIP_ERROR chip_ime_AppendCommand_BarrierControl_BarrierControlStop(chip::Controller::Device * device,
                                                                    chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::BarrierControl::EncodeBarrierControlStopCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// End of Cluster BarrierControl

// Cluster Basic
CHIP_ERROR chip_ime_AppendCommand_Basic_ResetToFactoryDefaults(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                               chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Basic::EncodeResetToFactoryDefaultsCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// End of Cluster Basic

// Cluster ColorControl
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveColor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, int16_t rateX, int16_t rateY,
                                                         uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveColorCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, rateX, rateY,
                                                              optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveColorTemperature(
    chip::Controller::Device * device, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t moveMode, uint16_t rate,
    uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveColorTemperatureCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                         moveMode, rate, colorTemperatureMinimum,
                                                                         colorTemperatureMaximum, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveHue(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate,
                                                       uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveHueCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode, rate,
                                                            optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveSaturation(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate,
                                                              uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveSaturationCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode,
                                                                   rate, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToColor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                           chip::GroupId ZCLgroupId, uint16_t colorX, uint16_t colorY,
                                                           uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveToColorCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, colorX,
                                                                colorY, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToColorTemperature(chip::Controller::Device * device,
                                                                      chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                      uint16_t colorTemperature, uint16_t transitionTime,
                                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveToColorTemperatureCommand(
        device->GetCommandSender(), ZCLendpointId, ZCLgroupId, colorTemperature, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToHue(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, uint8_t hue, uint8_t direction,
                                                         uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveToHueCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, hue, direction,
                                                              transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToHueAndSaturation(chip::Controller::Device * device,
                                                                      chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                      uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveToHueAndSaturationCommand(
        device->GetCommandSender(), ZCLendpointId, ZCLgroupId, hue, saturation, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToSaturation(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t saturation,
                                                                uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeMoveToSaturationCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                     saturation, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepColor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, int16_t stepX, int16_t stepY,
                                                         uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeStepColorCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, stepX, stepY,
                                                              transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepColorTemperature(chip::Controller::Device * device,
                                                                    chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                                                    uint16_t colorTemperatureMinimum,
                                                                    uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                                    uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeStepColorTemperatureCommand(
        device->GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode, stepSize, transitionTime, colorTemperatureMinimum,
        colorTemperatureMaximum, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepHue(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                       uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeStepHueCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode,
                                                            stepSize, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepSaturation(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                              uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeStepSaturationCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode,
                                                                   stepSize, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StopMoveStep(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                            chip::GroupId ZCLgroupId, uint8_t optionsMask, uint8_t optionsOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::ColorControl::EncodeStopMoveStepCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, optionsMask,
                                                                 optionsOverride);
}
// End of Cluster ColorControl

// Cluster DoorLock
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearAllPins(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                        chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearAllPinsCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearAllRfids(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearAllRfidsCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearHolidaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t scheduleId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearHolidayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                     scheduleId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearPin(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearPinCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearRfid(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearRfidCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearWeekdaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearWeekdayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                     scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearYeardaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeClearYeardayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                     scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetHolidaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetHolidayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                   scheduleId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetLogRecord(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                        chip::GroupId ZCLgroupId, uint16_t logIndex)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetLogRecordCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, logIndex);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetPin(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetPinCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetRfid(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetRfidCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetUserType(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetUserTypeCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetWeekdaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetWeekdayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                   scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetYeardaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeGetYeardayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                   scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_LockDoor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, char * pin)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeLockDoorCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, pin);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetHolidaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint32_t localStartTime,
                                                              uint32_t localEndTime, uint8_t operatingModeDuringHoliday)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeSetHolidayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                   scheduleId, localStartTime, localEndTime,
                                                                   operatingModeDuringHoliday);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetPin(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t userId, uint8_t userStatus, uint8_t userType,
                                                  char * pin)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeSetPinCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId, userStatus,
                                                       userType, pin);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetRfid(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t userId, uint8_t userStatus, uint8_t userType,
                                                   char * id)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeSetRfidCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId, userStatus,
                                                        userType, id);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetUserType(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint16_t userId, uint8_t userType)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeSetUserTypeCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, userId,
                                                            userType);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetWeekdaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId,
                                                              uint8_t daysMask, uint8_t startHour, uint8_t startMinute,
                                                              uint8_t endHour, uint8_t endMinute)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeSetWeekdayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                   scheduleId, userId, daysMask, startHour, startMinute, endHour,
                                                                   endMinute);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetYeardaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId,
                                                              uint32_t localStartTime, uint32_t localEndTime)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeSetYeardayScheduleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                   scheduleId, userId, localStartTime, localEndTime);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_UnlockDoor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                      chip::GroupId ZCLgroupId, char * pin)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeUnlockDoorCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, pin);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_UnlockWithTimeout(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId, uint16_t timeoutInSeconds, char * pin)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::DoorLock::EncodeUnlockWithTimeoutCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                  timeoutInSeconds, pin);
}
// End of Cluster DoorLock

// Cluster Groups
CHIP_ERROR chip_ime_AppendCommand_Groups_AddGroup(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t groupId, char * groupName)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Groups::EncodeAddGroupCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, groupName);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_AddGroupIfIdentifying(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                               chip::GroupId ZCLgroupId, uint16_t groupId, char * groupName)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Groups::EncodeAddGroupIfIdentifyingCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId,
                                                                    groupName);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_GetGroupMembership(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                            chip::GroupId ZCLgroupId, uint8_t groupCount, uint16_t groupList)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Groups::EncodeGetGroupMembershipCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupCount,
                                                                 groupList);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_RemoveAllGroups(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Groups::EncodeRemoveAllGroupsCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_RemoveGroup(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t groupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Groups::EncodeRemoveGroupCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_ViewGroup(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t groupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Groups::EncodeViewGroupCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
// End of Cluster Groups

// Cluster IasZone
// End of Cluster IasZone

// Cluster Identify
CHIP_ERROR chip_ime_AppendCommand_Identify_Identify(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint16_t identifyTime)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Identify::EncodeIdentifyCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, identifyTime);
}
CHIP_ERROR chip_ime_AppendCommand_Identify_IdentifyQuery(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Identify::EncodeIdentifyQueryCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// End of Cluster Identify

// Cluster LevelControl
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Move(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate, uint8_t optionMask,
                                                    uint8_t optionOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeMoveCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode, rate,
                                                         optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveToLevel(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                           chip::GroupId ZCLgroupId, uint8_t level, uint16_t transitionTime,
                                                           uint8_t optionMask, uint8_t optionOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeMoveToLevelCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, level,
                                                                transitionTime, optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveToLevelWithOnOff(chip::Controller::Device * device,
                                                                    chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t level, uint16_t transitionTime)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeMoveToLevelWithOnOffCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId,
                                                                         level, transitionTime);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveWithOnOff(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeMoveWithOnOffCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, moveMode,
                                                                  rate);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Step(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                    uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeStepCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode, stepSize,
                                                         transitionTime, optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_StepWithOnOff(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                             uint16_t transitionTime)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeStepWithOnOffCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, stepMode,
                                                                  stepSize, transitionTime);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Stop(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint8_t optionMask, uint8_t optionOverride)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeStopCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, optionMask,
                                                         optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_StopWithOnOff(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::LevelControl::EncodeStopWithOnOffCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// End of Cluster LevelControl

// Cluster OnOff
CHIP_ERROR chip_ime_AppendCommand_OnOff_Off(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                            chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::OnOff::EncodeOffCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
CHIP_ERROR chip_ime_AppendCommand_OnOff_On(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::OnOff::EncodeOnCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
CHIP_ERROR chip_ime_AppendCommand_OnOff_Toggle(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::OnOff::EncodeToggleCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId);
}
// End of Cluster OnOff

// Cluster Scenes
CHIP_ERROR chip_ime_AppendCommand_Scenes_AddScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId,
                                                  uint16_t transitionTime, char * sceneName, chip::ClusterId clusterId,
                                                  uint8_t length, uint8_t value)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeAddSceneCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId,
                                                       transitionTime, sceneName, clusterId, length, value);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_GetSceneMembership(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                            chip::GroupId ZCLgroupId, uint16_t groupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeGetSceneMembershipCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_RecallScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId,
                                                     uint16_t transitionTime)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeRecallSceneCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId,
                                                          transitionTime);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_RemoveAllScenes(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, uint16_t groupId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeRemoveAllScenesCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_RemoveScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeRemoveSceneCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_StoreScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeStoreSceneCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_ViewScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return app::cluster::Scenes::EncodeViewSceneCommand(device->GetCommandSender(), ZCLendpointId, ZCLgroupId, groupId, sceneId);
}
// End of Cluster Scenes

// Cluster TemperatureMeasurement
// End of Cluster TemperatureMeasurement

CHIP_ERROR chip_ime_SendCommand(chip::Controller::Device * device, chip::NodeId nodeId)
{
    if (device == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return device->SendCommand();
}
}
