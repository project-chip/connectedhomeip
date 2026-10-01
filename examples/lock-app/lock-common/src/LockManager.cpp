/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#include "LockManager.h"

#include <app/clusters/door-lock-server/CodegenIntegration.h>
#include <lib/support/logging/CHIPLogging.h>
#include <memory>

using chip::to_underlying;

LockManager LockManager::instance;

LockManager & LockManager::Instance()
{
    return instance;
}

bool LockManager::InitEndpoint(chip::EndpointId endpointId)
{
    // Capacities are read from the code-driven cluster instance, which was
    // created (with the application server-config overrides applied) before
    // this callback runs.
    uint16_t numberOfSupportedUsers    = 10;
    uint16_t numberOfPINCredentials    = 10;
    uint16_t numberOfRFIDCredentials   = 10;
    uint8_t numberOfCredentialsPerUser = 5;
    uint8_t numberOfWeekDaySchedules   = 10;
    uint8_t numberOfYearDaySchedules   = 10;
    uint8_t numberOfHolidaySchedules   = 10;

    auto * cluster = chip::app::Clusters::DoorLock::FindClusterOnEndpoint(endpointId);
    if (cluster != nullptr)
    {
        numberOfSupportedUsers     = cluster->GetNumberOfUserSupported();
        numberOfPINCredentials     = cluster->GetNumberOfPINCredentialsSupported();
        numberOfRFIDCredentials    = cluster->GetNumberOfRFIDCredentialsSupported();
        numberOfCredentialsPerUser = cluster->GetNumberOfCredentialsSupportedPerUser();
        numberOfWeekDaySchedules   = cluster->GetNumberOfWeekDaySchedulesPerUserSupported();
        numberOfYearDaySchedules   = cluster->GetNumberOfYearDaySchedulesPerUserSupported();
        numberOfHolidaySchedules   = cluster->GetNumberOfHolidaySchedulesSupported();
    }
    else
    {
        ChipLogError(Zcl, "Door Lock cluster not found on endpoint %d, using default capacities", endpointId);
    }

    // We're planning to use shared storage for PIN and RFID users so we will
    // have the maximum of both sizes here to simplify logic
    uint16_t numberOfSupportedCredentials = std::max(numberOfPINCredentials, numberOfRFIDCredentials);

    mEndpoints.emplace_back(std::make_unique<LockEndpoint>(endpointId, numberOfSupportedUsers, numberOfSupportedCredentials,
                                                           numberOfWeekDaySchedules, numberOfYearDaySchedules,
                                                           numberOfCredentialsPerUser, numberOfHolidaySchedules));

    ChipLogProgress(Zcl,
                    "Initialized new lock door endpoint "
                    "[id=%d,users=%d,credentials=%d,weekDaySchedulesPerUser=%d,yearDaySchedulesPerUser=%d,"
                    "numberOfCredentialsSupportedPerUser=%d,holidaySchedules=%d]",
                    endpointId, numberOfSupportedUsers, numberOfSupportedCredentials, numberOfWeekDaySchedules,
                    numberOfYearDaySchedules, numberOfCredentialsPerUser, numberOfHolidaySchedules);

    return true;
}

bool LockManager::SetDoorState(chip::EndpointId endpointId, DoorStateEnum doorState)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to toggle the door state - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return false;
    }
    return lockEndpoint->SetDoorState(doorState);
}

bool LockManager::SendLockAlarm(chip::EndpointId endpointId, AlarmCodeEnum alarmCode)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to send lock alarm - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->SendLockAlarm(alarmCode);
}

bool LockManager::Lock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                       const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err,
                       OperationSourceEnum opSource)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to lock the door - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->Lock(fabricIdx, nodeId, pin, err, opSource);
}

bool LockManager::Unlock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                         const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err,
                         OperationSourceEnum opSource)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to unlock the door - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->Unlock(fabricIdx, nodeId, pin, err, opSource);
}

bool LockManager::Unbolt(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                         const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err,
                         OperationSourceEnum opSource)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to unbolt the door - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->Unbolt(fabricIdx, nodeId, pin, err, opSource);
}

bool LockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to get the user - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->GetUser(userIndex, user);
}

bool LockManager::SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                          CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to set the user - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->SetUser(userIndex, creator, modifier, userName, uniqueId, userStatus, usertype, credentialRule,
                                 credentials, totalCredentials);
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to get the credential - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->GetCredential(credentialIndex, credentialType, credential);
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                const chip::ByteSpan & credentialData)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to set the credential - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }
    return lockEndpoint->SetCredential(credentialIndex, creator, modifier, credentialStatus, credentialType, credentialData);
}

DlStatus LockManager::GetSchedule(chip::EndpointId endpointId, uint8_t weekDayIndex, uint16_t userIndex,
                                  EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to get the week day schedule - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }
    return lockEndpoint->GetSchedule(weekDayIndex, userIndex, schedule);
}

DlStatus LockManager::SetSchedule(chip::EndpointId endpointId, uint8_t weekDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                  DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to set the week day schedule - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }
    return lockEndpoint->SetSchedule(weekDayIndex, userIndex, status, daysMask, startHour, startMinute, endHour, endMinute);
}

DlStatus LockManager::GetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                  EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to get the year day schedule - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }
    return lockEndpoint->GetSchedule(yearDayIndex, userIndex, schedule);
}

DlStatus LockManager::SetSchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                  uint32_t localStartTime, uint32_t localEndTime)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to set the year day schedule - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }
    return lockEndpoint->SetSchedule(yearDayIndex, userIndex, status, localStartTime, localEndTime);
}

DlStatus LockManager::GetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                  EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to get the holiday schedule - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }
    return lockEndpoint->GetSchedule(holidayIndex, schedule);
}

DlStatus LockManager::SetSchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                  uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to set the holiday schedule - endpoint does not exist or not initialized [endpointId=%d]",
                     endpointId);
        return DlStatus::kFailure;
    }
    return lockEndpoint->SetSchedule(holidayIndex, status, localStartTime, localEndTime, operatingMode);
}

LockEndpoint * LockManager::getEndpoint(chip::EndpointId endpointId)
{
    for (auto & endpoint : mEndpoints)
    {
        if (endpoint->GetEndpointId() == endpointId)
        {
            return endpoint.get();
        }
    }
    return nullptr;
}

LockEndpoint * LockManager::GetLockEndpoint(chip::EndpointId endpointId)
{
    return getEndpoint(endpointId);
}
