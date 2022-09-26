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
#include "LockEndpoint.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <cstring>

using chip::to_underlying;

bool LockEndpoint::Lock(const Optional<chip::ByteSpan> & pin, DlOperationError & err)
{
    return setLockState(DlLockState::kLocked, pin, err);
}

bool LockEndpoint::Unlock(const Optional<chip::ByteSpan> & pin, DlOperationError & err)
{
    return setLockState(DlLockState::kUnlocked, pin, err);
}

bool LockEndpoint::GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const
{
    ChipLogProgress(Zcl, "Lock App: LockEndpoint::GetUser [endpoint=%d,userIndex=%hu]", mEndpointId, userIndex);

    auto adjustedUserIndex = static_cast<uint16_t>(userIndex - 1);
    if (adjustedUserIndex > mLockUsers.size())
    {
        ChipLogError(Zcl, "Cannot get user - index out of range [endpoint=%d,index=%hu,adjustedIndex=%d]", mEndpointId, userIndex,
                     adjustedUserIndex);
        return false;
    }

    const auto & userInDb = mLockUsers[adjustedUserIndex];
    user.userStatus       = userInDb.userStatus;
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d,adjustedIndex=%hu]", mEndpointId, adjustedUserIndex);
        return true;
    }

    user.userName       = chip::CharSpan(userInDb.userName, strlen(userInDb.userName));
    user.credentials    = chip::Span<const DlCredential>(userInDb.credentials.data(), userInDb.credentials.size());
    user.userUniqueId   = userInDb.userUniqueId;
    user.userType       = userInDb.userType;
    user.credentialRule = userInDb.credentialRule;
    // So far there's no way to actually create the credential outside the matter, so here we always set the creation/modification
    // source to Matter
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = userInDb.createdBy;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = userInDb.lastModifiedBy;

    ChipLogDetail(Zcl,
                  "Found occupied user "
                  "[endpoint=%d,adjustedIndex=%hu,name=\"%.*s\",credentialsCount=%u,uniqueId=%x,type=%u,credentialRule=%u,"
                  "createdBy=%d,lastModifiedBy=%d]",
                  mEndpointId, adjustedUserIndex, static_cast<int>(user.userName.size()), user.userName.data(),
                  static_cast<unsigned int>(user.credentials.size()), user.userUniqueId, to_underlying(user.userType),
                  to_underlying(user.credentialRule), user.createdBy, user.lastModifiedBy);

    return true;
}

bool LockEndpoint::SetUser(uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                           const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                           DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    ChipLogProgress(Zcl,
                    "Lock App: LockEndpoint::SetUser "
                    "[endpoint=%d,userIndex=%u,creator=%d,modifier=%d,userName=\"%.*s\",uniqueId=%" PRIx32
                    ",userStatus=%u,userType=%u,"
                    "credentialRule=%u,credentials=%p,totalCredentials=%u]",
                    mEndpointId, userIndex, creator, modifier, static_cast<int>(userName.size()), userName.data(), uniqueId,
                    to_underlying(userStatus), to_underlying(usertype), to_underlying(credentialRule), credentials,
                    static_cast<unsigned int>(totalCredentials));

    auto adjustedUserIndex = static_cast<uint16_t>(userIndex - 1);
    if (adjustedUserIndex > mLockUsers.size())
    {
        ChipLogError(Zcl, "Cannot set user - index out of range [endpoint=%d,index=%d,adjustedUserIndex=%u]", mEndpointId,
                     userIndex, adjustedUserIndex);
        return false;
    }

    auto & userInStorage = mLockUsers[adjustedUserIndex];
    if (userName.size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        ChipLogError(Zcl, "Cannot set user - user name is too long [endpoint=%d,index=%d,adjustedUserIndex=%u]", mEndpointId,
                     userIndex, adjustedUserIndex);
        return false;
    }

    if (totalCredentials > userInStorage.credentials.capacity())
    {
        ChipLogError(Zcl,
                     "Cannot set user - total number of credentials is too big [endpoint=%d,index=%d,adjustedUserIndex=%u"
                     ",totalCredentials=%u,maxNumberOfCredentials=%u]",
                     mEndpointId, userIndex, adjustedUserIndex, static_cast<unsigned int>(totalCredentials),
                     static_cast<unsigned int>(userInStorage.credentials.capacity()));
        return false;
    }

    chip::Platform::CopyString(userInStorage.userName, userName);
    userInStorage.userName[userName.size()] = 0;
    userInStorage.userUniqueId              = uniqueId;
    userInStorage.userStatus                = userStatus;
    userInStorage.userType                  = usertype;
    userInStorage.credentialRule            = credentialRule;
    userInStorage.lastModifiedBy            = modifier;
    userInStorage.createdBy                 = creator;

    userInStorage.credentials.clear();
    for (size_t i = 0; i < totalCredentials; ++i)
    {
        userInStorage.credentials.push_back(credentials[i]);
    }

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d,adjustedIndex=%d]", mEndpointId, userIndex,
                    adjustedUserIndex);

    return true;
}

DlDoorState LockEndpoint::GetDoorState() const
{
    return mDoorState;
}

bool LockEndpoint::SetDoorState(DlDoorState newState)
{
    if (mDoorState != newState)
    {
        ChipLogProgress(Zcl, "Changing the door state to: %d [endpointId=%d,previousState=%d]", to_underlying(newState),
                        mEndpointId, to_underlying(mDoorState));

        mDoorState = newState;
        return DoorLockServer::Instance().SetDoorState(mEndpointId, mDoorState);
    }
    return true;
}

bool LockEndpoint::SendLockAlarm(DlAlarmCode alarmCode) const
{
    ChipLogProgress(Zcl, "Sending the LockAlarm event [endpointId=%d,alarmCode=%u]", mEndpointId, to_underlying(alarmCode));
    return DoorLockServer::Instance().SendLockAlarmEvent(mEndpointId, alarmCode);
}

bool LockEndpoint::GetCredential(uint16_t credentialIndex, DlCredentialType credentialType,
                                 EmberAfPluginDoorLockCredentialInfo & credential) const
{
    ChipLogProgress(Zcl, "Lock App: LockEndpoint::GetCredential [endpoint=%d,credentialIndex=%u,credentialType=%u]", mEndpointId,
                    credentialIndex, to_underlying(credentialType));

    if (credentialIndex >= mLockCredentials.size() || (0 == credentialIndex && DlCredentialType::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot get the credential - index out of range [endpoint=%d,index=%d]", mEndpointId, credentialIndex);
        return false;
    }

    const auto & credentialInStorage = mLockCredentials[credentialIndex];

    credential.status = credentialInStorage.status;
    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogDetail(Zcl, "Found unoccupied credential [endpoint=%d,index=%u]", mEndpointId, credentialIndex);
        return true;
    }
    credential.credentialType = credentialInStorage.credentialType;
    credential.credentialData = chip::ByteSpan(credentialInStorage.credentialData, credentialInStorage.credentialDataSize);
    // So far there's no way to actually create the credential outside the matter, so here we always set the creation/modification
    // source to Matter
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.createdBy          = credentialInStorage.createdBy;
    credential.modificationSource = DlAssetSource::kMatterIM;
    credential.lastModifiedBy     = credentialInStorage.modifiedBy;

    ChipLogDetail(Zcl, "Found occupied credential [endpoint=%d,index=%u,type=%u,dataSize=%u,createdBy=%u,modifiedBy=%u]",
                  mEndpointId, credentialIndex, to_underlying(credential.credentialType),
                  static_cast<unsigned int>(credential.credentialData.size()), credential.createdBy, credential.lastModifiedBy);

    return true;
}

bool LockEndpoint::SetCredential(uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                                 DlCredentialStatus credentialStatus, DlCredentialType credentialType,
                                 const chip::ByteSpan & credentialData)
{
    ChipLogProgress(
        Zcl,
        "Lock App: LockEndpoint::SetCredential "
        "[endpoint=%d,credentialIndex=%u,credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%u,modifier=%u]",
        mEndpointId, credentialIndex, to_underlying(credentialStatus), to_underlying(credentialType),
        static_cast<unsigned int>(credentialData.size()), creator, modifier);

    if (credentialIndex >= mLockCredentials.size() || (0 == credentialIndex && DlCredentialType::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot set the credential - index out of range [endpoint=%d,index=%d]", mEndpointId, credentialIndex);
        return false;
    }

    auto & credentialInStorage = mLockCredentials[credentialIndex];
    if (credentialData.size() > DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE)
    {
        ChipLogError(Zcl,
                     "Cannot get the credential - data size exceeds limit "
                     "[endpoint=%d,index=%d,dataSize=%u,maxDataSize=%u]",
                     mEndpointId, credentialIndex, static_cast<unsigned int>(credentialData.size()),
                     static_cast<unsigned int>(DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE));
        return false;
    }
    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    credentialInStorage.createdBy      = creator;
    credentialInStorage.modifiedBy     = modifier;
    std::memcpy(credentialInStorage.credentialData, credentialData.data(), credentialData.size());
    credentialInStorage.credentialDataSize = credentialData.size();

    ChipLogProgress(Zcl, "Successfully set the credential [mEndpointId=%d,index=%d,credentialType=%u,creator=%u,modifier=%u]",
                    mEndpointId, credentialIndex, to_underlying(credentialType), credentialInStorage.createdBy,
                    credentialInStorage.modifiedBy);

    return true;
}

DlStatus LockEndpoint::GetSchedule(uint8_t weekDayIndex, uint16_t userIndex, EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    if (0 == userIndex || userIndex > mWeekDaySchedules.size())
    {
        return DlStatus::kFailure;
    }

    if (0 == weekDayIndex || weekDayIndex > mWeekDaySchedules.at(userIndex - 1).size())
    {
        return DlStatus::kFailure;
    }

    const auto & scheduleInStorage = mWeekDaySchedules.at(userIndex - 1).at(weekDayIndex - 1);
    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockEndpoint::SetSchedule(uint8_t weekDayIndex, uint16_t userIndex, DlScheduleStatus status, DlDaysMaskMap daysMask,
                                   uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute)
{
    if (0 == userIndex || userIndex > mWeekDaySchedules.size())
    {
        return DlStatus::kFailure;
    }

    if (0 == weekDayIndex || weekDayIndex > mWeekDaySchedules.at(userIndex - 1).size())
    {
        return DlStatus::kFailure;
    }

    auto & scheduleInStorage = mWeekDaySchedules.at(userIndex - 1).at(weekDayIndex - 1);

    scheduleInStorage.schedule.daysMask    = daysMask;
    scheduleInStorage.schedule.startHour   = startHour;
    scheduleInStorage.schedule.startMinute = startMinute;
    scheduleInStorage.schedule.endHour     = endHour;
    scheduleInStorage.schedule.endMinute   = endMinute;
    scheduleInStorage.status               = status;

    return DlStatus::kSuccess;
}

DlStatus LockEndpoint::GetSchedule(uint8_t yearDayIndex, uint16_t userIndex, EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    if (0 == userIndex || userIndex > mYearDaySchedules.size())
    {
        return DlStatus::kFailure;
    }

    if (0 == yearDayIndex || yearDayIndex > mYearDaySchedules.at(userIndex - 1).size())
    {
        return DlStatus::kFailure;
    }

    const auto & scheduleInStorage = mYearDaySchedules.at(userIndex - 1).at(yearDayIndex - 1);
    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockEndpoint::SetSchedule(uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status, uint32_t localStartTime,
                                   uint32_t localEndTime)
{
    if (0 == userIndex || userIndex > mYearDaySchedules.size())
    {
        return DlStatus::kFailure;
    }

    if (0 == yearDayIndex || yearDayIndex > mYearDaySchedules.at(userIndex - 1).size())
    {
        return DlStatus::kFailure;
    }

    auto & scheduleInStorage                  = mYearDaySchedules.at(userIndex - 1).at(yearDayIndex - 1);
    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.status                  = status;

    return DlStatus::kSuccess;
}

DlStatus LockEndpoint::GetSchedule(uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    if (0 == holidayIndex || holidayIndex > mHolidaySchedules.size())
    {
        return DlStatus::kFailure;
    }

    const auto & scheduleInStorage = mHolidaySchedules[holidayIndex - 1];
    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;
    return DlStatus::kSuccess;
}

DlStatus LockEndpoint::SetSchedule(uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime,
                                   DlOperatingMode operatingMode)
{
    if (0 == holidayIndex || holidayIndex > mHolidaySchedules.size())
    {
        return DlStatus::kFailure;
    }

    auto & scheduleInStorage                  = mHolidaySchedules[holidayIndex - 1];
    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.schedule.operatingMode  = operatingMode;
    scheduleInStorage.status                  = status;

    return DlStatus::kSuccess;
}

bool LockEndpoint::setLockState(DlLockState lockState, const Optional<chip::ByteSpan> & pin, DlOperationError & err)
{
    // Assume pin is required until told otherwise
    bool requirePin = true;
    chip::app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(mEndpointId, &requirePin);

    // If a pin code is not given
    if (!pin.HasValue())
    {
        ChipLogDetail(Zcl, "Door Lock App: PIN code is not specified [endpointId=%d]", mEndpointId);

        // If a pin code is not required
        if (!requirePin)
        {
            ChipLogDetail(Zcl, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", lockStateToString(lockState),
                          mEndpointId);

            DoorLockServer::Instance().SetLockState(mEndpointId, lockState);

            return true;
        }

        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", mEndpointId);

        return false;
    }

    // Find the credential so we can make sure it is not absent right away
    auto credential = std::find_if(mLockCredentials.begin(), mLockCredentials.end(), [&pin](const LockCredentialInfo & c) {
        return (c.credentialType == DlCredentialType::kPin && c.status != DlCredentialStatus::kAvailable) &&
            chip::ByteSpan{ c.credentialData, c.credentialDataSize }.data_equal(pin.Value());
    });
    if (credential == mLockCredentials.end())
    {
        ChipLogDetail(Zcl,
                      "Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                      "[endpointId=%d]",
                      lockStateToString(lockState), mEndpointId);

        err = DlOperationError::kInvalidCredential;
        return false;
    }

    // Find a user that correspond to this credential
    auto credentialIndex = static_cast<unsigned>(credential - mLockCredentials.begin());
    auto user = std::find_if(mLockUsers.begin(), mLockUsers.end(), [credential, credentialIndex](const LockUserInfo & u) {
        return std::any_of(u.credentials.begin(), u.credentials.end(), [&credential, credentialIndex](const DlCredential & c) {
            return c.CredentialIndex == credentialIndex && c.CredentialType == to_underlying(credential->credentialType);
        });
    });
    if (user == mLockUsers.end())
    {
        ChipLogDetail(Zcl,
                      "Lock App: specified PIN code was found in the database, but the lock user is not associated with it "
                      "[endpointId=%d,credentialIndex=%u]",
                      mEndpointId, credentialIndex);
    }

    auto userIndex = static_cast<uint8_t>(user - mLockUsers.begin());

    // Check if schedules affect the user
    if ((user->userType == DlUserType::kScheduleRestrictedUser || user->userType == DlUserType::kWeekDayScheduleUser) &&
        !weekDayScheduleInAction(userIndex))
    {
        if ((user->userType == DlUserType::kScheduleRestrictedUser || user->userType == DlUserType::kYearDayScheduleUser) &&
            !yearDayScheduleInAction(userIndex))
        {
            ChipLogDetail(Zcl,
                          "Lock App: associated user is not allowed to operate the lock due to schedules"
                          "[endpointId=%d,userIndex=%u]",
                          mEndpointId, userIndex);
            err = DlOperationError::kRestricted;
            return false;
        }
    }
    ChipLogDetail(
        Zcl,
        "Lock App: specified PIN code was found in the database, setting door lock state to \"%s\" [endpointId=%d,userIndex=%u]",
        lockStateToString(lockState), mEndpointId, userIndex);

    mLockState = lockState;
    DoorLockServer::Instance().SetLockState(mEndpointId, mLockState);

    return true;
}

bool LockEndpoint::weekDayScheduleInAction(uint16_t userIndex) const
{
    const auto & user = mLockUsers[userIndex];
    if (user.userType != DlUserType::kScheduleRestrictedUser && user.userType != DlUserType::kWeekDayScheduleUser)
    {
        return true;
    }

    chip::System::Clock::Milliseconds64 cTMs;
    auto chipError = chip::System::SystemClock().GetClock_RealTimeMS(cTMs);
    if (chipError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Lock App: unable to get current time to check user schedules [endpointId=%d,error=%d (%s)]", mEndpointId,
                     chipError.AsInteger(), chipError.AsString());
        return false;
    }
    time_t unixEpoch = std::chrono::duration_cast<chip::System::Clock::Seconds32>(cTMs).count();

    tm calendarTime{};
    localtime_r(&unixEpoch, &calendarTime);

    auto currentTime =
        calendarTime.tm_hour * chip::kSecondsPerHour + calendarTime.tm_min * chip::kSecondsPerMinute + calendarTime.tm_sec;

    // Second, check the week day schedules.
    return std::any_of(
        mWeekDaySchedules[userIndex].begin(), mWeekDaySchedules[userIndex].end(),
        [currentTime, calendarTime](const WeekDaysScheduleInfo & s) {
            auto startTime = s.schedule.startHour * chip::kSecondsPerHour + s.schedule.startMinute * chip::kSecondsPerMinute;
            auto endTime   = s.schedule.endHour * chip::kSecondsPerHour + s.schedule.endMinute * chip::kSecondsPerMinute;
            return s.status == DlScheduleStatus::kOccupied && (to_underlying(s.schedule.daysMask) & (1 << calendarTime.tm_wday)) &&
                startTime <= currentTime && currentTime <= endTime;
        });
}

bool LockEndpoint::yearDayScheduleInAction(uint16_t userIndex) const
{
    const auto & user = mLockUsers[userIndex];
    if (user.userType != DlUserType::kScheduleRestrictedUser && user.userType != DlUserType::kYearDayScheduleUser)
    {
        return true;
    }

    chip::System::Clock::Milliseconds64 cTMs;
    auto chipError = chip::System::SystemClock().GetClock_RealTimeMS(cTMs);
    if (chipError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Lock App: unable to get current time to check user schedules [endpointId=%d,error=%d (%s)]", mEndpointId,
                     chipError.AsInteger(), chipError.AsString());
        return false;
    }
    auto unixEpoch     = std::chrono::duration_cast<chip::System::Clock::Seconds32>(cTMs).count();
    uint32_t chipEpoch = 0;
    if (!chip::UnixEpochToChipEpochTime(unixEpoch, chipEpoch))
    {
        ChipLogError(Zcl,
                     "Lock App: unable to convert Unix Epoch time to Matter Epoch Time to check user schedules "
                     "[endpointId=%d,userIndex=%d]",
                     mEndpointId, userIndex);
        return false;
    }

    return std::any_of(mYearDaySchedules[userIndex].begin(), mYearDaySchedules[userIndex].end(),
                       [chipEpoch](const YearDayScheduleInfo & sch) {
                           return sch.status == DlScheduleStatus::kOccupied && sch.schedule.localStartTime <= chipEpoch &&
                               chipEpoch <= sch.schedule.localEndTime;
                       });
}

const char * LockEndpoint::lockStateToString(DlLockState lockState) const
{
    switch (lockState)
    {
    case DlLockState::kNotFullyLocked:
        return "Not Fully Locked";
    case DlLockState::kLocked:
        return "Locked";
    case DlLockState::kUnlocked:
        return "Unlocked";
    case DlLockState::kUnknownEnumValue:
        break;
    }

    return "Unknown";
}
