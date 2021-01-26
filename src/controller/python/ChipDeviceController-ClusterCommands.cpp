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
#include <app/im-handler.h>
#include <support/ReturnMacros.h>

#include <controller/CHIPClusters.h>
#include <controller/CHIPDevice.h>

using namespace chip;
using namespace chip::app;

namespace chip {
namespace app {
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
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::BarrierControlCluster clusterBarrierControl;
    (void) ZCLgroupId;
    clusterBarrierControl.Associate(device, ZCLendpointId);
    return clusterBarrierControl.BarrierControlGoToPercent(nullptr, percentOpen);
}
CHIP_ERROR chip_ime_AppendCommand_BarrierControl_BarrierControlStop(chip::Controller::Device * device,
                                                                    chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::BarrierControlCluster clusterBarrierControl;
    (void) ZCLgroupId;
    clusterBarrierControl.Associate(device, ZCLendpointId);
    return clusterBarrierControl.BarrierControlStop(nullptr);
}
// End of Cluster BarrierControl

// Cluster Basic
CHIP_ERROR chip_ime_AppendCommand_Basic_ResetToFactoryDefaults(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                               chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::BasicCluster clusterBasic;
    (void) ZCLgroupId;
    clusterBasic.Associate(device, ZCLendpointId);
    return clusterBasic.ResetToFactoryDefaults(nullptr);
}
// End of Cluster Basic

// Cluster ColorControl
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveColor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, int16_t rateX, int16_t rateY,
                                                         uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveColor(nullptr, rateX, rateY, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveColorTemperature(
    chip::Controller::Device * device, chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId, uint8_t moveMode, uint16_t rate,
    uint16_t colorTemperatureMinimum, uint16_t colorTemperatureMaximum, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveColorTemperature(nullptr, moveMode, rate, colorTemperatureMinimum, colorTemperatureMaximum,
                                                    optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveHue(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate,
                                                       uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveHue(nullptr, moveMode, rate, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveSaturation(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate,
                                                              uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveSaturation(nullptr, moveMode, rate, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToColor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                           chip::GroupId ZCLgroupId, uint16_t colorX, uint16_t colorY,
                                                           uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveToColor(nullptr, colorX, colorY, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToColorTemperature(chip::Controller::Device * device,
                                                                      chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                      uint16_t colorTemperature, uint16_t transitionTime,
                                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveToColorTemperature(nullptr, colorTemperature, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToHue(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, uint8_t hue, uint8_t direction,
                                                         uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveToHue(nullptr, hue, direction, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToHueAndSaturation(chip::Controller::Device * device,
                                                                      chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                      uint8_t hue, uint8_t saturation, uint16_t transitionTime,
                                                                      uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveToHueAndSaturation(nullptr, hue, saturation, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_MoveToSaturation(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t saturation,
                                                                uint16_t transitionTime, uint8_t optionsMask,
                                                                uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.MoveToSaturation(nullptr, saturation, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepColor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, int16_t stepX, int16_t stepY,
                                                         uint16_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.StepColor(nullptr, stepX, stepY, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepColorTemperature(chip::Controller::Device * device,
                                                                    chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t stepMode, uint16_t stepSize, uint16_t transitionTime,
                                                                    uint16_t colorTemperatureMinimum,
                                                                    uint16_t colorTemperatureMaximum, uint8_t optionsMask,
                                                                    uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.StepColorTemperature(nullptr, stepMode, stepSize, transitionTime, colorTemperatureMinimum,
                                                    colorTemperatureMaximum, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepHue(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                       uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.StepHue(nullptr, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StepSaturation(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                              uint8_t transitionTime, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.StepSaturation(nullptr, stepMode, stepSize, transitionTime, optionsMask, optionsOverride);
}
CHIP_ERROR chip_ime_AppendCommand_ColorControl_StopMoveStep(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                            chip::GroupId ZCLgroupId, uint8_t optionsMask, uint8_t optionsOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ColorControlCluster clusterColorControl;
    (void) ZCLgroupId;
    clusterColorControl.Associate(device, ZCLendpointId);
    return clusterColorControl.StopMoveStep(nullptr, optionsMask, optionsOverride);
}
// End of Cluster ColorControl

// Cluster DoorLock
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearAllPins(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                        chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearAllPins(nullptr);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearAllRfids(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearAllRfids(nullptr);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearHolidaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t scheduleId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearHolidaySchedule(nullptr, scheduleId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearPin(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearPin(nullptr, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearRfid(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearRfid(nullptr, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearWeekdaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearWeekdaySchedule(nullptr, scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_ClearYeardaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                                chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.ClearYeardaySchedule(nullptr, scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetHolidaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetHolidaySchedule(nullptr, scheduleId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetLogRecord(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                        chip::GroupId ZCLgroupId, uint16_t logIndex)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetLogRecord(nullptr, logIndex);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetPin(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetPin(nullptr, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetRfid(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetRfid(nullptr, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetUserType(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetUserType(nullptr, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetWeekdaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetWeekdaySchedule(nullptr, scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_GetYeardaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.GetYeardaySchedule(nullptr, scheduleId, userId);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_LockDoor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, char * pin)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.LockDoor(nullptr, pin);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetHolidaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint32_t localStartTime,
                                                              uint32_t localEndTime, uint8_t operatingModeDuringHoliday)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.SetHolidaySchedule(nullptr, scheduleId, localStartTime, localEndTime, operatingModeDuringHoliday);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetPin(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t userId, uint8_t userStatus, uint8_t userType,
                                                  char * pin)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.SetPin(nullptr, userId, userStatus, userType, pin);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetRfid(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t userId, uint8_t userStatus, uint8_t userType,
                                                   char * id)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.SetRfid(nullptr, userId, userStatus, userType, id);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetUserType(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                       chip::GroupId ZCLgroupId, uint16_t userId, uint8_t userType)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.SetUserType(nullptr, userId, userType);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetWeekdaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId,
                                                              uint8_t daysMask, uint8_t startHour, uint8_t startMinute,
                                                              uint8_t endHour, uint8_t endMinute)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.SetWeekdaySchedule(nullptr, scheduleId, userId, daysMask, startHour, startMinute, endHour, endMinute);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_SetYeardaySchedule(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                              chip::GroupId ZCLgroupId, uint8_t scheduleId, uint16_t userId,
                                                              uint32_t localStartTime, uint32_t localEndTime)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.SetYeardaySchedule(nullptr, scheduleId, userId, localStartTime, localEndTime);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_UnlockDoor(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                      chip::GroupId ZCLgroupId, char * pin)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.UnlockDoor(nullptr, pin);
}
CHIP_ERROR chip_ime_AppendCommand_DoorLock_UnlockWithTimeout(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId, uint16_t timeoutInSeconds, char * pin)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::DoorLockCluster clusterDoorLock;
    (void) ZCLgroupId;
    clusterDoorLock.Associate(device, ZCLendpointId);
    return clusterDoorLock.UnlockWithTimeout(nullptr, timeoutInSeconds, pin);
}
// End of Cluster DoorLock

// Cluster Groups
CHIP_ERROR chip_ime_AppendCommand_Groups_AddGroup(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t groupId, char * groupName)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::GroupsCluster clusterGroups;
    (void) ZCLgroupId;
    clusterGroups.Associate(device, ZCLendpointId);
    return clusterGroups.AddGroup(nullptr, groupId, groupName);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_AddGroupIfIdentifying(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                               chip::GroupId ZCLgroupId, uint16_t groupId, char * groupName)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::GroupsCluster clusterGroups;
    (void) ZCLgroupId;
    clusterGroups.Associate(device, ZCLendpointId);
    return clusterGroups.AddGroupIfIdentifying(nullptr, groupId, groupName);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_GetGroupMembership(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                            chip::GroupId ZCLgroupId, uint8_t groupCount, uint16_t groupList)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::GroupsCluster clusterGroups;
    (void) ZCLgroupId;
    clusterGroups.Associate(device, ZCLendpointId);
    return clusterGroups.GetGroupMembership(nullptr, groupCount, groupList);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_RemoveAllGroups(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::GroupsCluster clusterGroups;
    (void) ZCLgroupId;
    clusterGroups.Associate(device, ZCLendpointId);
    return clusterGroups.RemoveAllGroups(nullptr);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_RemoveGroup(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t groupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::GroupsCluster clusterGroups;
    (void) ZCLgroupId;
    clusterGroups.Associate(device, ZCLendpointId);
    return clusterGroups.RemoveGroup(nullptr, groupId);
}
CHIP_ERROR chip_ime_AppendCommand_Groups_ViewGroup(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t groupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::GroupsCluster clusterGroups;
    (void) ZCLgroupId;
    clusterGroups.Associate(device, ZCLendpointId);
    return clusterGroups.ViewGroup(nullptr, groupId);
}
// End of Cluster Groups

// Cluster IasZone
// End of Cluster IasZone

// Cluster Identify
CHIP_ERROR chip_ime_AppendCommand_Identify_Identify(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint16_t identifyTime)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::IdentifyCluster clusterIdentify;
    (void) ZCLgroupId;
    clusterIdentify.Associate(device, ZCLendpointId);
    return clusterIdentify.Identify(nullptr, identifyTime);
}
CHIP_ERROR chip_ime_AppendCommand_Identify_IdentifyQuery(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::IdentifyCluster clusterIdentify;
    (void) ZCLgroupId;
    clusterIdentify.Associate(device, ZCLendpointId);
    return clusterIdentify.IdentifyQuery(nullptr);
}
// End of Cluster Identify

// Cluster LevelControl
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Move(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate, uint8_t optionMask,
                                                    uint8_t optionOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.Move(nullptr, moveMode, rate, optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveToLevel(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                           chip::GroupId ZCLgroupId, uint8_t level, uint16_t transitionTime,
                                                           uint8_t optionMask, uint8_t optionOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.MoveToLevel(nullptr, level, transitionTime, optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveToLevelWithOnOff(chip::Controller::Device * device,
                                                                    chip::EndpointId ZCLendpointId, chip::GroupId ZCLgroupId,
                                                                    uint8_t level, uint16_t transitionTime)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.MoveToLevelWithOnOff(nullptr, level, transitionTime);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_MoveWithOnOff(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId, uint8_t moveMode, uint8_t rate)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.MoveWithOnOff(nullptr, moveMode, rate);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Step(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                    uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.Step(nullptr, stepMode, stepSize, transitionTime, optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_StepWithOnOff(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId, uint8_t stepMode, uint8_t stepSize,
                                                             uint16_t transitionTime)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.StepWithOnOff(nullptr, stepMode, stepSize, transitionTime);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_Stop(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint8_t optionMask, uint8_t optionOverride)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.Stop(nullptr, optionMask, optionOverride);
}
CHIP_ERROR chip_ime_AppendCommand_LevelControl_StopWithOnOff(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                             chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::LevelControlCluster clusterLevelControl;
    (void) ZCLgroupId;
    clusterLevelControl.Associate(device, ZCLendpointId);
    return clusterLevelControl.StopWithOnOff(nullptr);
}
// End of Cluster LevelControl

// Cluster OnOff
CHIP_ERROR chip_ime_AppendCommand_OnOff_Off(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                            chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OnOffCluster clusterOnOff;
    (void) ZCLgroupId;
    clusterOnOff.Associate(device, ZCLendpointId);
    return clusterOnOff.Off(nullptr);
}
CHIP_ERROR chip_ime_AppendCommand_OnOff_On(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                           chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OnOffCluster clusterOnOff;
    (void) ZCLgroupId;
    clusterOnOff.Associate(device, ZCLendpointId);
    return clusterOnOff.On(nullptr);
}
CHIP_ERROR chip_ime_AppendCommand_OnOff_Toggle(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                               chip::GroupId ZCLgroupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::OnOffCluster clusterOnOff;
    (void) ZCLgroupId;
    clusterOnOff.Associate(device, ZCLendpointId);
    return clusterOnOff.Toggle(nullptr);
}
// End of Cluster OnOff

// Cluster Scenes
CHIP_ERROR chip_ime_AppendCommand_Scenes_AddScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                  chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId,
                                                  uint16_t transitionTime, char * sceneName, chip::ClusterId clusterId,
                                                  uint8_t length, uint8_t value)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.AddScene(nullptr, groupId, sceneId, transitionTime, sceneName, clusterId, length, value);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_GetSceneMembership(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                            chip::GroupId ZCLgroupId, uint16_t groupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.GetSceneMembership(nullptr, groupId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_RecallScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId,
                                                     uint16_t transitionTime)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.RecallScene(nullptr, groupId, sceneId, transitionTime);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_RemoveAllScenes(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                         chip::GroupId ZCLgroupId, uint16_t groupId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.RemoveAllScenes(nullptr, groupId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_RemoveScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                     chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.RemoveScene(nullptr, groupId, sceneId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_StoreScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                    chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.StoreScene(nullptr, groupId, sceneId);
}
CHIP_ERROR chip_ime_AppendCommand_Scenes_ViewScene(chip::Controller::Device * device, chip::EndpointId ZCLendpointId,
                                                   chip::GroupId ZCLgroupId, uint16_t groupId, uint8_t sceneId)
{
    VerifyOrReturnError(device != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    chip::Controller::ScenesCluster clusterScenes;
    (void) ZCLgroupId;
    clusterScenes.Associate(device, ZCLendpointId);
    return clusterScenes.ViewScene(nullptr, groupId, sceneId);
}
// End of Cluster Scenes

// Cluster TemperatureMeasurement
// End of Cluster TemperatureMeasurement
}
