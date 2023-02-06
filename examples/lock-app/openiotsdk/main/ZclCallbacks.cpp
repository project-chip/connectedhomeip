/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/util/af.h>

#include "LockManager.h"

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DoorLock;

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                            OperationErrorEnum & err)
{
    return LockManager::Instance().Lock(endpointId, pinCode, err);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                              OperationErrorEnum & err)
{
    return LockManager::Instance().Unlock(endpointId, pinCode, err);
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

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    VerifyOrReturn(attributePath.mClusterId == DoorLock::Id && attributePath.mAttributeId == DoorLock::Attributes::LockState::Id);

    emberAfDoorLockClusterPrintln("Door Lock attribute changed");
}

void emberAfDoorLockClusterInitCallback(EndpointId endpoint)
{
    DoorLockServer::Instance().InitServer(endpoint);
    LockManager::Instance().InitEndpoint(endpoint);
}
