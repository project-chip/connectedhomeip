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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"
#include "AppTask.h"
#include "LockManager.h"
#include <platform/CHIPDeviceLayer.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace ::chip::DeviceLayer::Internal;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == DoorLock::Id && attributeId == DoorLock::Attributes::LockState::Id)
    {
        ChipLogProgress(Zcl, "Door lock cluster: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));
    }
}

/** @brief DoorLock Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint) {}

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Lock Command endpoint=%d", endpointId);
    bool status = LockMgr().Lock(endpointId, pinCode, err);
    if (status == true)
    {
        LockMgr().InitiateAction(AppEvent::kEventType_Lock, LockManager::LOCK_ACTION);
    }
    return status;
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode,
                                              DlOperationError & err)
{
    ChipLogProgress(Zcl, "Door Lock App: Unlock Command endpoint=%d", endpointId);
    bool status = LockMgr().Unlock(endpointId, pinCode, err);
    if (status == true)
    {
        LockMgr().InitiateAction(AppEvent::kEventType_Lock, LockManager::UNLOCK_ACTION);
    }

    return status;
}

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return LockMgr().GetCredential(endpointId, credentialIndex, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                        chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                        DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    return LockMgr().SetCredential(endpointId, credentialIndex, creator, modifier, credentialStatus, credentialType,
                                   credentialData);
}

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return LockMgr().GetUser(endpointId, userIndex, user);
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  DlUserStatus userStatus, DlUserType usertype, DlCredentialRule credentialRule,
                                  const DlCredential * credentials, size_t totalCredentials)
{

    return LockMgr().SetUser(endpointId, userIndex, creator, modifier, userName, uniqueId, userStatus, usertype, credentialRule,
                             credentials, totalCredentials);
}

// TODO: These functions will be supported by door-lock-server in the future. These are set to return failure until implemented.
DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule) //
{
    return LockMgr().GetWeekdaySchedule(endpointId, weekdayIndex, userIndex, schedule);
    // return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    return LockMgr().GetYeardaySchedule(endpointId, yearDayIndex, userIndex, schedule);
    // return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DlDaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute)
{
    return LockMgr().SetWeekdaySchedule(endpointId, weekdayIndex, userIndex, status, daysMask, startHour, startMinute, endHour,
                                        endMinute);
    // return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    return LockMgr().SetYeardaySchedule(endpointId, yearDayIndex, userIndex, status, localStartTime, localEndTime);
    // return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockGetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    return LockMgr().GetHolidaySchedule(endpointId, holidayIndex, schedule);
    // return DlStatus::kFailure;
}

DlStatus emberAfPluginDoorLockSetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, DlOperatingMode operatingMode)
{
    return LockMgr().SetHolidaySchedule(endpointId, holidayIndex, status, localStartTime, localEndTime, operatingMode);
    // return DlStatus::kFailure;
}
