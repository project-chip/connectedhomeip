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

#include "im-handler.h"

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelEngine.h>

#include <support/ReturnMacros.h>

namespace chip {
namespace app {
namespace cluster {


namespace BarrierControl {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        BarrierControl::kClusterId,
        BarrierControl::kBarrierControlGoToPercentCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleBarrierControlGoToPercentCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        BarrierControl::kClusterId,
        BarrierControl::kBarrierControlStopCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleBarrierControlStopCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        BarrierControl::kClusterId,
        BarrierControl::kBarrierControlGoToPercentCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        BarrierControl::kClusterId,
        BarrierControl::kBarrierControlStopCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace BarrierControl


namespace Basic {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        Basic::kClusterId,
        Basic::kResetToFactoryDefaultsCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleResetToFactoryDefaultsCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        Basic::kClusterId,
        Basic::kResetToFactoryDefaultsCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace Basic


namespace ColorControl {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveColorCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveColorCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveColorTemperatureCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveColorTemperatureCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveHueCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveHueCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveSaturationCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToColorCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToColorCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToColorTemperatureCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToColorTemperatureCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToHueCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToHueCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToHueAndSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToHueAndSaturationCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToSaturationCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepColorCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStepColorCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepColorTemperatureCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStepColorTemperatureCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepHueCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStepHueCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStepSaturationCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStopMoveStepCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStopMoveStepCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveColorCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveColorTemperatureCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveHueCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToColorCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToColorTemperatureCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToHueCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToHueAndSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kMoveToSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepColorCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepColorTemperatureCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepHueCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStepSaturationCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        ColorControl::kClusterId,
        ColorControl::kStopMoveStepCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace ColorControl


namespace DoorLock {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearAllPinsCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearAllPinsCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearAllRfidsCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearAllRfidsCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearHolidayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearHolidayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearPinCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearPinCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearRfidCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearRfidCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearWeekdayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearWeekdayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearYeardayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleClearYeardayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetHolidayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetHolidayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetLogRecordCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetLogRecordCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetPinCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetPinCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetRfidCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetRfidCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetUserTypeCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetUserTypeCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetWeekdayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetWeekdayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetYeardayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetYeardayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kLockDoorCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleLockDoorCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetHolidayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleSetHolidayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetPinCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleSetPinCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetRfidCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleSetRfidCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetUserTypeCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleSetUserTypeCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetWeekdayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleSetWeekdayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetYeardayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleSetYeardayScheduleCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kUnlockDoorCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleUnlockDoorCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kUnlockWithTimeoutCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleUnlockWithTimeoutCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearAllPinsCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearAllRfidsCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearHolidayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearPinCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearRfidCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearWeekdayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kClearYeardayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetHolidayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetLogRecordCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetPinCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetRfidCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetUserTypeCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetWeekdayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kGetYeardayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kLockDoorCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetHolidayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetPinCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetRfidCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetUserTypeCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetWeekdayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kSetYeardayScheduleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kUnlockDoorCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        DoorLock::kClusterId,
        DoorLock::kUnlockWithTimeoutCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace DoorLock


namespace Groups {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kAddGroupCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleAddGroupCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kAddGroupIfIdentifyingCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleAddGroupIfIdentifyingCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kGetGroupMembershipCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetGroupMembershipCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kRemoveAllGroupsCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleRemoveAllGroupsCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kRemoveGroupCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleRemoveGroupCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kViewGroupCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleViewGroupCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kAddGroupCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kAddGroupIfIdentifyingCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kGetGroupMembershipCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kRemoveAllGroupsCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kRemoveGroupCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Groups::kClusterId,
        Groups::kViewGroupCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace Groups


namespace IasZone {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

} // namespace IasZone


namespace Identify {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        Identify::kClusterId,
        Identify::kIdentifyCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleIdentifyCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Identify::kClusterId,
        Identify::kIdentifyQueryCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleIdentifyQueryCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        Identify::kClusterId,
        Identify::kIdentifyCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Identify::kClusterId,
        Identify::kIdentifyQueryCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace Identify


namespace LevelControl {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveToLevelCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToLevelCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveToLevelWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveToLevelWithOnOffCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleMoveWithOnOffCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStepCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStepCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStepWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStepWithOnOffCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStopCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStopCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStopWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStopWithOnOffCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveToLevelCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveToLevelWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kMoveWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStepCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStepWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStopCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        LevelControl::kClusterId,
        LevelControl::kStopWithOnOffCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace LevelControl


namespace OnOff {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        OnOff::kClusterId,
        OnOff::kOffCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleOffCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        OnOff::kClusterId,
        OnOff::kOnCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleOnCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        OnOff::kClusterId,
        OnOff::kToggleCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleToggleCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        OnOff::kClusterId,
        OnOff::kOffCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        OnOff::kClusterId,
        OnOff::kOnCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        OnOff::kClusterId,
        OnOff::kToggleCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace OnOff


namespace Scenes {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kAddSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleAddSceneCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kGetSceneMembershipCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleGetSceneMembershipCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kRecallSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleRecallSceneCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kRemoveAllScenesCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleRemoveAllScenesCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kRemoveSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleRemoveSceneCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kStoreSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleStoreSceneCommandReceived
        );
    ime->RegisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kViewSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId,
        HandleViewSceneCommandReceived
        );
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kAddSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kGetSceneMembershipCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kRecallSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kRemoveAllScenesCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kRemoveSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kStoreSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
    ime->DeregisterClusterCommandHandler(
        Scenes::kClusterId,
        Scenes::kViewSceneCommandId,
        Command::CommandRoleId::kCommandHandlerId
        );
}

} // namespace Scenes


namespace TemperatureMeasurement {

void InitCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

void ShutdownCluster(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
}

} // namespace TemperatureMeasurement


void InitClusters(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    BarrierControl::InitCluster(ime);
    Basic::InitCluster(ime);
    ColorControl::InitCluster(ime);
    DoorLock::InitCluster(ime);
    Groups::InitCluster(ime);
    IasZone::InitCluster(ime);
    Identify::InitCluster(ime);
    LevelControl::InitCluster(ime);
    OnOff::InitCluster(ime);
    Scenes::InitCluster(ime);
    TemperatureMeasurement::InitCluster(ime);
}

void ShutdownClusters(chip::app::InteractionModelEngine * ime)
{
    VerifyOrReturn(ime != nullptr);
    BarrierControl::ShutdownCluster(ime);
    Basic::ShutdownCluster(ime);
    ColorControl::ShutdownCluster(ime);
    DoorLock::ShutdownCluster(ime);
    Groups::ShutdownCluster(ime);
    IasZone::ShutdownCluster(ime);
    Identify::ShutdownCluster(ime);
    LevelControl::ShutdownCluster(ime);
    OnOff::ShutdownCluster(ime);
    Scenes::ShutdownCluster(ime);
    TemperatureMeasurement::ShutdownCluster(ime);
}

} // namespace cluster
} // namespace app
} // namespace chip
