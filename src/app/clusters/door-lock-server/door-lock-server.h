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
 * @brief APIs and defines for the Door Lock Server
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/af.h>

#ifndef DOOR_LOCK_SERVER_ENDPOINT
#define DOOR_LOCK_SERVER_ENDPOINT 1
#endif

class DoorLockServer
{
    static DoorLockServer & Instance();

    void InitServer();

    bool SetLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState newLockState);
    bool SetActuatorState(chip::EndpointId endpointId, bool actuatorState);
    bool SetDoorState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState doorState);

    bool SetLanguage(chip::EndpointId endpointId, const char * newLanguage);
    bool SetAutoRelockTime(chip::EndpointId, uint32_t newAutoRelockTimeSec);
    bool SetSoundVolume(chip::EndpointId endpointId, uint8_t newSoundVolume);

    bool SetOneTouchLocking(chip::EndpointId endpointId, bool isEnabled);
    bool SetPrivacyModeButton(chip::EndpointId endpointId, bool isEnabled);

private:
    static DoorLockServer instance;
};

#if 0
bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const char * PINCOde);
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const char * PINCode);

bool emberAfPluginDoorLockGetUsers(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockClearUser(chip::EndpointId endpointId, ...);

bool emberAfPluginDoorLockGetCredentials(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, ...);
bool emberAfPluginDoorLockClearCredential(chip::EndpointId endpointId, ...);
#endif


#if 0
bool emberAfDoorLockClusterLockDoorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                            const Commands::LockDoor::DecodableType & commandData);
bool emberAfDoorLockClusterUnlockDoorCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::UnlockDoor::DecodableType & commandData);
bool emberAfDoorLockClusterGetLogRecordCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::GetLogRecord::DecodableType & commandData);
bool emberAfDoorLockClusterSetWeekDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetWeekDaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterGetWeekDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetWeekDaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterClearWeekDayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearWeekDaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterSetYearDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetYearDaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterGetYearDayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetYearDaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterClearYearDayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearYearDaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterSetHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::SetHolidaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterGetHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const Commands::GetHolidaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterClearHolidayScheduleCallback(app::CommandHandler * commandObj,
                                                        const app::ConcreteCommandPath & commandPath,
                                                        const Commands::ClearHolidaySchedule::DecodableType & commandData);
bool emberAfDoorLockClusterGetUserTypeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::GetUserType::DecodableType & commandData);
bool emberAfDoorLockClusterSetUserTypeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::SetUserType::DecodableType & commandData);
bool emberAfDoorLockClusterSetPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::SetPINCode::DecodableType & commandData);
bool emberAfDoorLockClusterGetPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                              const Commands::GetPINCode::DecodableType & commandData);
bool emberAfDoorLockClusterClearPINCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Commands::ClearPINCode::DecodableType & commandData);
bool emberAfDoorLockClusterClearAllPINCodesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const Commands::ClearAllPINCodes::DecodableType & commandData);
bool emberAfDoorLockClusterSetRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::SetRFIDCode::DecodableType & commandData);
bool emberAfDoorLockClusterGetRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                               const Commands::GetRFIDCode::DecodableType & commandData);
bool emberAfDoorLockClusterClearRFIDCodeCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                 const Commands::ClearRFIDCode::DecodableType & commandData);
bool emberAfDoorLockClusterClearAllRFIDCodesCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::ClearAllRFIDCodes::DecodableType & commandData);
void emberAfPluginDoorLockServerLockoutEventHandler(void);
void emberAfPluginDoorLockServerRelockEventHandler(void);
bool emberAfDoorLockClusterUnlockWithTimeoutCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                     const Commands::UnlockWithTimeout::DecodableType & commandData);
void MatterDoorLockPluginServerInitCallback();
void MatterDoorLockClusterServerAttributeChangedCallback(const app::ConcreteAttributePath & attributePath);
#endif