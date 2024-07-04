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
#include <app/util/config.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef MATTER_DM_PLUGIN_DOOR_LOCK_SERVER
#include "chef-lock-manager.h"

#include <algorithm>
#include <iostream>

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
        numberOfSupportedUsers = 10;
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
        numberOfSupportedCredentials = 10;
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
        numberOfCredentialsSupportedPerUser = 5;
    }

    uint8_t numberOfWeekDaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, numberOfWeekDaySchedulesPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported week day schedules per user when initializing lock endpoint, defaulting to "
                     "10 [endpointId=%d]",
                     endpointId);
        numberOfWeekDaySchedulesPerUser = 10;
    }

    uint8_t numberOfYearDaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfYearDaySchedulesPerUserSupported(endpointId, numberOfYearDaySchedulesPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported year day schedules per user when initializing lock endpoint, defaulting to "
                     "10 [endpointId=%d]",
                     endpointId);
        numberOfYearDaySchedulesPerUser = 10;
    }

    uint8_t numberOfHolidaySchedules = 0;
    if (!DoorLockServer::Instance().GetNumberOfHolidaySchedulesSupported(endpointId, numberOfHolidaySchedules))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported holiday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfHolidaySchedules = 10;
    }

    mEndpoints.emplace_back(endpointId, numberOfSupportedUsers, numberOfSupportedCredentials, numberOfWeekDaySchedulesPerUser,
                            numberOfYearDaySchedulesPerUser, numberOfCredentialsSupportedPerUser, numberOfHolidaySchedules);

    // Refer to 5.2.10.34. SetUser Command, when Creat a new user record
    //    - UserIndex value SHALL be set to a user record with UserType set to Available
    //    - UserName MAY be null causing new user record to use empty string for UserName
    //        otherwise UserName SHALL be set to the value provided in the new user record.
    //    - UserUniqueID MAY be null causing new user record to use 0xFFFFFFFF for UserUniqueID
    //        otherwise UserUniqueID SHALL be set to the value provided in the new user record
    //    - UserStatus MAY be null causing new user record to use OccupiedEnabled for UserStatus
    //        otherwise UserStatus SHALL be set to the value provided in the new user record
    //   - UserType MAY be null causing new user record to use UnrestrictedUser for UserType
    //        otherwise UserType SHALL be set to the value provided in the new user record.
    uint16_t userIndex(1);
    chip::FabricIndex creator(1);
    chip::FabricIndex modifier(1);
    const chip::CharSpan userName = chip::CharSpan::fromCharString("user1"); // default
                                                                             // username
    uint32_t uniqueId         = 0xFFFFFFFF;                                  // null
    UserStatusEnum userStatus = UserStatusEnum::kOccupiedEnabled;
    // Set to programming user instead of unrestrict user to perform
    // priviledged function
    UserTypeEnum usertype             = UserTypeEnum::kProgrammingUser;
    CredentialRuleEnum credentialRule = CredentialRuleEnum::kSingle;

    constexpr size_t totalCredentials(2);
    // According to spec (5.2.6.26.2. CredentialIndex Field), programming PIN credential should be always indexed as 0
    uint16_t credentialIndex0(0);
    // 1st non ProgrammingPIN credential should be indexed as 1
    uint16_t credentialIndex1(1);

    const CredentialStruct credentials[totalCredentials] = {
        { credentialType : CredentialTypeEnum::kProgrammingPIN, credentialIndex : credentialIndex0 },
        { credentialType : CredentialTypeEnum::kPin, credentialIndex : credentialIndex1 }
    };

    if (!SetUser(endpointId, userIndex, creator, modifier, userName, uniqueId, userStatus, usertype, credentialRule,
                 &credentials[0], totalCredentials))
    {
        ChipLogError(Zcl, "Unable to set the User [endpointId=%d]", endpointId);
        return false;
    }

    DlCredentialStatus credentialStatus = DlCredentialStatus::kOccupied;

    // Set the default user's ProgrammingPIN credential
    uint8_t defaultProgrammingPIN[6] = { 0x39, 0x39, 0x39, 0x39, 0x39, 0x39 }; // 000000
    if (!SetCredential(endpointId, credentialIndex0, creator, modifier, credentialStatus, CredentialTypeEnum::kProgrammingPIN,
                       chip::ByteSpan(defaultProgrammingPIN)))
    {
        ChipLogError(Zcl, "Unable to set the credential - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }

    // Set the default user's non ProgrammingPIN credential
    uint8_t defaultPin[6] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 }; // 123456
    if (!SetCredential(endpointId, credentialIndex1, creator, modifier, credentialStatus, CredentialTypeEnum::kPin,
                       chip::ByteSpan(defaultPin)))
    {
        ChipLogError(Zcl, "Unable to set the credential - endpoint does not exist or not initialized [endpointId=%d]", endpointId);
        return false;
    }

    ChipLogProgress(Zcl,
                    "Initialized new lock door endpoint "
                    "[id=%d,users=%d,credentials=%d,weekDaySchedulesPerUser=%d,yearDaySchedulesPerUser=%d,"
                    "numberOfCredentialsSupportedPerUser=%d,holidaySchedules=%d]",
                    endpointId, numberOfSupportedUsers, numberOfSupportedCredentials, numberOfWeekDaySchedulesPerUser,
                    numberOfYearDaySchedulesPerUser, numberOfCredentialsSupportedPerUser, numberOfHolidaySchedules);

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
    for (auto & mEndpoint : mEndpoints)
    {
        if (mEndpoint.GetEndpointId() == endpointId)
        {
            return &mEndpoint;
        }
    }
    return nullptr;
}
#endif // MATTER_DM_PLUGIN_DOOR_LOCK_SERVER
