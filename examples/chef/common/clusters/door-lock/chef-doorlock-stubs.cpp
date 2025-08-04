/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
 *    All rights reserved.
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

#include <app/data-model/Nullable.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

#ifdef MATTER_DM_PLUGIN_DOOR_LOCK_SERVER
#include "chef-lock-manager.h"
#include <app/clusters/door-lock-server/door-lock-server.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;
using chip::app::DataModel::Nullable;

// =============================================================================
// 'Default' callbacks for cluster commands
// =============================================================================

// App handles physical aspects of locking but not locking logic. That is it
// should wait for door to be locked on lock command and return success) but
// door lock server should check pin before even calling the lock-door
// callback.
bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    return LockManager::Instance().Lock(endpointId, fabricIdx, nodeId, pinCode, err, OperationSourceEnum::kRemote);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return LockManager::Instance().Unlock(endpointId, fabricIdx, nodeId, pinCode, err, OperationSourceEnum::kRemote);
}

bool emberAfPluginDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return LockManager::Instance().Unbolt(endpointId, fabricIdx, nodeId, pinCode, err, OperationSourceEnum::kRemote);
}

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return LockManager::Instance().GetUser(endpointId, userIndex, user);
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  UserStatusEnum userStatus, UserTypeEnum usertype, CredentialRuleEnum credentialRule,
                                  const CredentialStruct * credentials, size_t totalCredentials)
{

    return LockManager::Instance().SetUser(endpointId, userIndex, creator, modifier, userName, uniqueId, userStatus, usertype,
                                           credentialRule, credentials, totalCredentials);
}

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return LockManager::Instance().GetCredential(endpointId, credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                        chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                        CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData)
{
    return LockManager::Instance().SetCredential(endpointId, credentialIndex, creator, modifier, credentialStatus, credentialType,
                                                 credentialData);
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    return LockManager::Instance().GetSchedule(endpointId, weekdayIndex, userIndex, schedule);
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    return LockManager::Instance().GetSchedule(endpointId, holidayIndex, schedule);
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute)
{
    return LockManager::Instance().SetSchedule(endpointId, weekdayIndex, userIndex, status, daysMask, startHour, startMinute,
                                               endHour, endMinute);
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    return LockManager::Instance().SetSchedule(endpointId, yearDayIndex, userIndex, status, localStartTime, localEndTime);
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    return LockManager::Instance().GetSchedule(endpointId, yearDayIndex, userIndex, schedule);
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    return LockManager::Instance().SetSchedule(endpointId, holidayIndex, status, localStartTime, localEndTime, operatingMode);
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    DoorLockServer::Instance().InitServer(endpoint);
    LockManager::Instance().InitEndpoint(endpoint);
}
#endif // MATTER_DM_PLUGIN_DOOR_LOCK_SERVER
