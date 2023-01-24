/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <CHIPProjectConfig.h>

#include <cstring>
#include <iostream>
#include <lib/support/logging/CHIPLogging.h>

using chip::to_underlying;

LockManager LockManager::instance;

LockManager & LockManager::Instance()
{
    return instance;
}

bool LockManager::InitEndpoint(chip::EndpointId endpointId)
{
    uint16_t numberOfSupportedUsers = 0;
    if (!DoorLockServer::Instance().GetNumberOfUserSupported(endpointId, numberOfSupportedUsers))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported users when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
                     endpointId);
        numberOfSupportedUsers = CHIP_LOCK_MANAGER_USER_NUMBER;
    }

    uint16_t numberOfSupportedCredentials = 0;
    // We're planning to use shared storage for PIN and RFID users so we will have the maximum of both sizes her to simplify logic
    uint16_t numberOfPINCredentialsSupported  = 0;
    uint16_t numberOfRFIDCredentialsSupported = 0;
    if (!DoorLockServer::Instance().GetNumberOfPINCredentialsSupported(endpointId, numberOfPINCredentialsSupported) ||
        !DoorLockServer::Instance().GetNumberOfRFIDCredentialsSupported(endpointId, numberOfRFIDCredentialsSupported))
    {
        ChipLogError(
            Zcl, "Unable to get number of supported credentials when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfSupportedCredentials = CHIP_LOCK_MANAGER_CREDENTIALS_NUMBER;
    }
    else
    {
        numberOfSupportedCredentials = std::max(numberOfPINCredentialsSupported, numberOfRFIDCredentialsSupported);
    }

    uint8_t numberOfCredentialsSupportedPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfCredentialsSupportedPerUser(endpointId, numberOfCredentialsSupportedPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of credentials supported per user when initializing lock endpoint, defaulting to 5 "
                     "[endpointId=%d]",
                     endpointId);
        numberOfCredentialsSupportedPerUser = CHIP_LOCK_MANAGER_CREDENTIALS_PER_USER_NUMBER;
    }

    uint8_t numberOfWeekDaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, numberOfWeekDaySchedulesPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported week day schedules per user when initializing lock endpoint, defaulting to "
                     "10 [endpointId=%d]",
                     endpointId);
        numberOfWeekDaySchedulesPerUser = CHIP_LOCK_MANAGER_WEEK_DAY_SCHEDULES_PER_USER_NUMBER;
    }

    uint8_t numberOfYearDaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfYearDaySchedulesPerUserSupported(endpointId, numberOfYearDaySchedulesPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported year day schedules per user when initializing lock endpoint, defaulting to "
                     "10 [endpointId=%d]",
                     endpointId);
        numberOfYearDaySchedulesPerUser = CHIP_LOCK_MANAGER_YEAR_DAY_SCHEDULES_PER_USER_NUMBER;
    }

    mEndpoints.emplace_back(endpointId, numberOfSupportedUsers, numberOfSupportedCredentials, numberOfWeekDaySchedulesPerUser,
                            numberOfYearDaySchedulesPerUser, numberOfCredentialsSupportedPerUser);

    ChipLogProgress(Zcl,
                    "Initialized new lock door endpoint "
                    "[id=%d,users=%d,credentials=%d,weekDaySchedulesPerUser=%d,yearDaySchedulesPerUser=%d,"
                    "numberOfCredentialsSupportedPerUser=%d]",
                    endpointId, numberOfSupportedUsers, numberOfSupportedCredentials, numberOfWeekDaySchedulesPerUser,
                    numberOfYearDaySchedulesPerUser, numberOfCredentialsSupportedPerUser);

    return true;
}

bool LockManager::Lock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to lock the door - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }

    VerifyOrReturnValue(lockEndpoint->Lock(pin, err), false);

    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked);
}

bool LockManager::Unlock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    auto lockEndpoint = getEndpoint(endpointId);
    if (nullptr == lockEndpoint)
    {
        ChipLogError(Zcl, "Unable to unlock the door - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }

    VerifyOrReturnValue(lockEndpoint->Unlock(pin, err), false);

    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked);
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

LockEndpoint * LockManager::getEndpoint(chip::EndpointId endpointId)
{
    for (auto & mEndpoint : mEndpoints)
    {
        if (mEndpoint.GetEndpointId() == endpointId)
        {
            return &mEndpoint;
        }
    }
    return nullptr;
}
