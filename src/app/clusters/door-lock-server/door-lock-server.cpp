/**
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

/****************************************************************************
 * @file
 * @brief Routines for the Door Lock Server plugin.
 *******************************************************************************
 ******************************************************************************/

#include "door-lock-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/command-id.h>
#include <app/util/af-event.h>
#include <app/util/time-util.h>
#include <app/util/af.h>

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app::Clusters::DoorLock;

EmberEventControl emberAfPluginDoorLockServerLockoutEventControl;
EmberEventControl emberAfPluginDoorLockServerRelockEventControl;

void DoorLockServer::InitServer()
{
}

bool DoorLockServer::SetLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState newLockState)
{
    return true;
}

bool DoorLockServer::SetActuatorState(chip::EndpointId endpointId, bool actuatorState)
{
    return true;
}

bool DoorLockServer::SetDoorState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState doorState)
{
    return true;
}


bool DoorLockServer::SetLanguage(chip::EndpointId endpointId, const char * newLanguage)
{
    return true;
}

bool DoorLockServer::SetAutoRelockTime(chip::EndpointId, uint32_t newAutoRelockTimeSec)
{
    return true;
}

bool DoorLockServer::SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume)
{
    return true;
}


bool DoorLockServer::SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled)
{
    return true;
}

bool DoorLockServer::SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled)
{
    return true;
}

// =======================================================

bool emberAfDoorLockClusterLockDoorCallback(chip::app::CommandHandler * commandObj,
                                            const chip::app::ConcreteCommandPath & commandPath,
                                            const chip::app::Clusters::DoorLock::Commands::LockDoor::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Lock Door command (not implemented)");

    // TODO: Implement door locking by calling emberAfPluginDoorLockOnDoorLockCommand

    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterUnlockDoorCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::UnlockDoor::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Unlock Door command (not implemented)");

    // TODO: Implement door unlocking by calling emberAfPluginDoorLockOnDoorUnlockCommand
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::SetUser::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Set User command (not implemented)");

    // TODO: Implement setting the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetUserCallback(chip::app::CommandHandler * commandObj,
                                           const chip::app::ConcreteCommandPath & commandPath,
                                           const chip::app::Clusters::DoorLock::Commands::GetUser::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Get User command (not implemented)");

    // TODO: Implement getting the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearUserCallback(chip::app::CommandHandler * commandObj,
                                             const chip::app::ConcreteCommandPath & commandPath,
                                             const chip::app::Clusters::DoorLock::Commands::ClearUser::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Clear User command (not implemented)");

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterSetCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::SetCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Set Credential command (not implemented)");

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterGetCredentialStatusCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::GetCredentialStatus::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Get Credential Status command (not implemented)");

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterClearCredentialCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::DoorLock::Commands::ClearCredential::DecodableType & commandData)
{
    emberAfDoorLockClusterPrintln("Received Clear Credential command (not implemented)");

    // TODO: Implement clearing the user
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

// =======================================================

bool emberAfDoorLockClusterGetLogRecordCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::GetLogRecord::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterSetWeekDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetWeekDaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterGetWeekDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetWeekDaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearWeekDayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearWeekDaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterSetYearDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetYearDaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterGetYearDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetYearDaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearYearDayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearYearDaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterSetHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetHolidaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterGetHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetHolidaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearHolidaySchedule::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterGetUserTypeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::GetUserType::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterSetUserTypeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::SetUserType::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterSetPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SetPINCode::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterGetPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::GetPINCode::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::ClearPINCode::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearAllPINCodesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::ClearAllPINCodes::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterSetRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::SetRFIDCode::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterGetRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::GetRFIDCode::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::ClearRFIDCode::DecodableType & commandData)
{
    return true;
}

bool emberAfDoorLockClusterClearAllRFIDCodesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::ClearAllRFIDCodes::DecodableType & commandData)
{
    return true;
}

void emberAfPluginDoorLockServerLockoutEventHandler(void)
{
}

void emberAfPluginDoorLockServerRelockEventHandler(void)
{
}

bool emberAfDoorLockClusterUnlockWithTimeoutCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::UnlockWithTimeout::DecodableType & commandData)
{
    return true;
}

void MatterDoorLockPluginServerInitCallback()
{
}

void MatterDoorLockClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath)
{
}
