/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/util/af.h>
#include <cstring>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#ifdef MATTER_DM_PLUGIN_DOOR_LOCK_SERVER
#include "chef-lock-endpoint.h"

using chip::to_underlying;
using chip::app::DataModel::MakeNullable;

struct LockActionData
{
    chip::EndpointId endpointId;
    DlLockState lockState;
    OperationSourceEnum opSource;
    Nullable<uint16_t> userIndex;
    uint16_t credentialIndex;
    Nullable<chip::FabricIndex> fabricIdx;
    Nullable<chip::NodeId> nodeId;
    bool moving = false;
};

static LockActionData gCurrentAction;

bool LockEndpoint::Lock(const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                        const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err, OperationSourceEnum opSource)
{
    return setLockState(fabricIdx, nodeId, DlLockState::kLocked, pin, err, opSource);
}

bool LockEndpoint::Unlock(const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                          const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err, OperationSourceEnum opSource)
{
    if (DoorLockServer::Instance().SupportsUnbolt(mEndpointId))
    {
        // If Unbolt is supported Unlock is supposed to pull the latch
        return setLockState(fabricIdx, nodeId, DlLockState::kUnlatched, pin, err, opSource);
    }

    return setLockState(fabricIdx, nodeId, DlLockState::kUnlocked, pin, err, opSource);
}

bool LockEndpoint::Unbolt(const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                          const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err, OperationSourceEnum opSource)
{
    return setLockState(fabricIdx, nodeId, DlLockState::kUnlocked, pin, err, opSource);
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
    if (UserStatusEnum::kAvailable == user.userStatus)
    {
        ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d,adjustedIndex=%hu]", mEndpointId, adjustedUserIndex);
        return true;
    }

    user.userName       = userInDb.userName;
    user.credentials    = chip::Span<const CredentialStruct>(userInDb.credentials.data(), userInDb.credentials.size());
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
                           const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                           CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
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

    userInStorage.userName = chip::MutableCharSpan(userInStorage.userNameBuf, DOOR_LOCK_USER_NAME_BUFFER_SIZE);
    CopyCharSpanToMutableCharSpan(userName, userInStorage.userName);
    userInStorage.userUniqueId   = uniqueId;
    userInStorage.userStatus     = userStatus;
    userInStorage.userType       = usertype;
    userInStorage.credentialRule = credentialRule;
    userInStorage.lastModifiedBy = modifier;
    userInStorage.createdBy      = creator;

    userInStorage.credentials.clear();
    for (size_t i = 0; i < totalCredentials; ++i)
    {
        userInStorage.credentials.push_back(credentials[i]);
    }

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d,adjustedIndex=%d]", mEndpointId, userIndex,
                    adjustedUserIndex);

    return true;
}

DoorStateEnum LockEndpoint::GetDoorState() const
{
    return mDoorState;
}

bool LockEndpoint::SetDoorState(DoorStateEnum newState)
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

bool LockEndpoint::SendLockAlarm(AlarmCodeEnum alarmCode) const
{
    ChipLogProgress(Zcl, "Sending the LockAlarm event [endpointId=%d,alarmCode=%u]", mEndpointId, to_underlying(alarmCode));
    return DoorLockServer::Instance().SendLockAlarmEvent(mEndpointId, alarmCode);
}

bool LockEndpoint::GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                 EmberAfPluginDoorLockCredentialInfo & credential) const
{
    ChipLogProgress(Zcl, "Lock App: LockEndpoint::GetCredential [endpoint=%d,credentialIndex=%u,credentialType=%u]", mEndpointId,
                    credentialIndex, to_underlying(credentialType));

    if (to_underlying(credentialType) >= mLockCredentials.size())
    {
        ChipLogError(Zcl, "Cannot get the credential - index out of range [endpoint=%d,index=%d]", mEndpointId, credentialIndex);
        return false;
    }

    if (credentialIndex >= mLockCredentials.at(to_underlying(credentialType)).size() ||
        (0 == credentialIndex && CredentialTypeEnum::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot get the credential - index out of range [endpoint=%d,index=%d]", mEndpointId, credentialIndex);
        return false;
    }

    const auto & credentialInStorage = mLockCredentials[to_underlying(credentialType)][credentialIndex];

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
                                 DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                 const chip::ByteSpan & credentialData)
{
    ChipLogProgress(
        Zcl,
        "Lock App: LockEndpoint::SetCredential "
        "[endpoint=%d,credentialIndex=%u,credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%u,modifier=%u]",
        mEndpointId, credentialIndex, to_underlying(credentialStatus), to_underlying(credentialType),
        static_cast<unsigned int>(credentialData.size()), creator, modifier);

    if (to_underlying(credentialType) >= mLockCredentials.capacity())
    {
        ChipLogError(Zcl, "Cannot set the credential - type out of range [endpoint=%d,type=%d]", mEndpointId,
                     to_underlying(credentialType));
        return false;
    }

    if (credentialIndex >= mLockCredentials.at(to_underlying(credentialType)).size() ||
        (0 == credentialIndex && CredentialTypeEnum::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot set the credential - index out of range [endpoint=%d,index=%d]", mEndpointId, credentialIndex);
        return false;
    }

    // Assign to array by credentialIndex. Note: 0 is reserved for programmingPIN only
    auto & credentialInStorage = mLockCredentials[to_underlying(credentialType)][credentialIndex];
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

DlStatus LockEndpoint::SetSchedule(uint8_t weekDayIndex, uint16_t userIndex, DlScheduleStatus status, DaysMaskMap daysMask,
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
                                   OperatingModeEnum operatingMode)
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

bool LockEndpoint::setLockState(const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                                DlLockState lockState, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err,
                                OperationSourceEnum opSource)
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
            ChipLogProgress(Zcl, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", lockStateToString(lockState),
                            mEndpointId);

            if (gCurrentAction.moving == true)
            {
                ChipLogProgress(Zcl, "Lock App: not executing lock action as another lock action is already active [endpointId=%d]",
                                mEndpointId);
                return false;
            }

            gCurrentAction.moving     = true;
            gCurrentAction.endpointId = mEndpointId;
            gCurrentAction.lockState  = lockState;
            gCurrentAction.opSource   = opSource;
            gCurrentAction.userIndex  = NullNullable;
            gCurrentAction.fabricIdx  = fabricIdx;
            gCurrentAction.nodeId     = nodeId;

            // Do this async as a real lock would do too but use 0s delay to speed up CI tests
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(0), OnLockActionCompleteCallback, nullptr);

            return true;
        }

        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", mEndpointId);

        err = OperationErrorEnum::kInvalidCredential;
        return false;
    }

    // Find the credential so we can make sure it is not absent right away
    auto & pinCredentials = mLockCredentials[to_underlying(CredentialTypeEnum::kPin)];
    auto credential       = std::find_if(pinCredentials.begin(), pinCredentials.end(), [&pin](const LockCredentialInfo & c) {
        return (c.status != DlCredentialStatus::kAvailable) &&
            chip::ByteSpan{ c.credentialData, c.credentialDataSize }.data_equal(pin.Value());
    });
    if (credential == pinCredentials.end())
    {
        ChipLogDetail(Zcl,
                      "Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                      "[endpointId=%d]",
                      lockStateToString(lockState), mEndpointId);

        err = OperationErrorEnum::kInvalidCredential;
        return false;
    }

    // Find a user that correspond to this credential
    auto credentialIndex = static_cast<unsigned>(credential - pinCredentials.begin());
    auto user = std::find_if(mLockUsers.begin(), mLockUsers.end(), [credential, credentialIndex](const LockUserInfo & u) {
        return std::any_of(u.credentials.begin(), u.credentials.end(), [&credential, credentialIndex](const CredentialStruct & c) {
            return c.credentialIndex == credentialIndex && c.credentialType == credential->credentialType;
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
    bool haveWeekDaySchedules = false;
    bool haveYearDaySchedules = false;
    if (weekDayScheduleForbidsAccess(userIndex, &haveWeekDaySchedules) ||
        yearDayScheduleForbidsAccess(userIndex, &haveYearDaySchedules) ||
        // Also disallow access for a user that's supposed to have _some_
        // schedule but doesn't have any
        (user->userType == UserTypeEnum::kScheduleRestrictedUser && !haveWeekDaySchedules && !haveYearDaySchedules))
    {
        ChipLogDetail(Zcl,
                      "Lock App: associated user is not allowed to operate the lock due to schedules"
                      "[endpointId=%d,userIndex=%u]",
                      mEndpointId, userIndex);
        err = OperationErrorEnum::kRestricted;
        return false;
    }
    ChipLogProgress(
        Zcl,
        "Lock App: specified PIN code was found in the database, setting door lock state to \"%s\" [endpointId=%d,userIndex=%u]",
        lockStateToString(lockState), mEndpointId, userIndex);

    if (gCurrentAction.moving == true)
    {
        ChipLogProgress(Zcl,
                        "Lock App: not executing lock action as another lock action is already active [endpointId=%d,userIndex=%u]",
                        mEndpointId, userIndex);
        return false;
    }

    gCurrentAction.moving          = true;
    gCurrentAction.endpointId      = mEndpointId;
    gCurrentAction.lockState       = lockState;
    gCurrentAction.opSource        = opSource;
    gCurrentAction.userIndex       = MakeNullable(static_cast<uint16_t>(userIndex + 1));
    gCurrentAction.credentialIndex = static_cast<uint16_t>(credentialIndex);
    gCurrentAction.fabricIdx       = fabricIdx;
    gCurrentAction.nodeId          = nodeId;

    // Do this async as a real lock would do too but use 0s delay to speed up CI tests
    chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(0), OnLockActionCompleteCallback, nullptr);

    return true;
}

void LockEndpoint::OnLockActionCompleteCallback(chip::System::Layer *, void * callbackContext)
{
    if (gCurrentAction.userIndex.IsNull())
    {
        DoorLockServer::Instance().SetLockState(gCurrentAction.endpointId, gCurrentAction.lockState, gCurrentAction.opSource,
                                                NullNullable, NullNullable, gCurrentAction.fabricIdx, gCurrentAction.nodeId);
    }
    else
    {
        LockOpCredentials userCredential[] = { { CredentialTypeEnum::kPin, gCurrentAction.credentialIndex } };
        auto userCredentials               = MakeNullable<List<const LockOpCredentials>>(userCredential);

        DoorLockServer::Instance().SetLockState(gCurrentAction.endpointId, gCurrentAction.lockState, gCurrentAction.opSource,
                                                gCurrentAction.userIndex, userCredentials, gCurrentAction.fabricIdx,
                                                gCurrentAction.nodeId);
    }

    // move back to Unlocked after Unlatch
    if (gCurrentAction.lockState == DlLockState::kUnlatched)
    {
        gCurrentAction.lockState = DlLockState::kUnlocked;

        // Do this async as a real lock would do too but use 0s delay to speed up CI tests
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(0), OnLockActionCompleteCallback, nullptr);
    }
    else
    {
        gCurrentAction.moving = false;
    }
}

bool LockEndpoint::weekDayScheduleForbidsAccess(uint16_t userIndex, bool * haveSchedule) const
{
    *haveSchedule = std::any_of(mWeekDaySchedules[userIndex].begin(), mWeekDaySchedules[userIndex].end(),
                                [](const WeekDaysScheduleInfo & s) { return s.status == DlScheduleStatus::kOccupied; });

    const auto & user = mLockUsers[userIndex];
    if (user.userType != UserTypeEnum::kScheduleRestrictedUser && user.userType != UserTypeEnum::kWeekDayScheduleUser)
    {
        // Weekday schedules don't apply to this user.
        return false;
    }

    if (user.userType == UserTypeEnum::kScheduleRestrictedUser && !*haveSchedule)
    {
        // It's valid to not have any schedules of a given type; on its own this
        // does not prevent access.
        return false;
    }

    chip::System::Clock::Milliseconds64 cTMs;
    auto chipError = chip::System::SystemClock().GetClock_RealTimeMS(cTMs);
    if (chipError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Lock App: unable to get current time to check user schedules [endpointId=%d (%s)]", mEndpointId,
                     chipError.AsString());
        return true;
    }
    time_t unixEpoch = std::chrono::duration_cast<chip::System::Clock::Seconds32>(cTMs).count();

    tm calendarTime{};
    localtime_r(&unixEpoch, &calendarTime);

    auto currentTime =
        calendarTime.tm_hour * chip::kSecondsPerHour + calendarTime.tm_min * chip::kSecondsPerMinute + calendarTime.tm_sec;

    // Now check whether any schedule allows the current time.  If it does,
    // access is not forbidden.
    return !std::any_of(
        mWeekDaySchedules[userIndex].begin(), mWeekDaySchedules[userIndex].end(),
        [currentTime, calendarTime](const WeekDaysScheduleInfo & s) {
            auto startTime = s.schedule.startHour * chip::kSecondsPerHour + s.schedule.startMinute * chip::kSecondsPerMinute;
            auto endTime   = s.schedule.endHour * chip::kSecondsPerHour + s.schedule.endMinute * chip::kSecondsPerMinute;

            return s.status == DlScheduleStatus::kOccupied && (to_underlying(s.schedule.daysMask) & (1 << calendarTime.tm_wday)) &&
                startTime <= currentTime && currentTime <= endTime;
        });
}

bool LockEndpoint::yearDayScheduleForbidsAccess(uint16_t userIndex, bool * haveSchedule) const
{
    *haveSchedule = std::any_of(mYearDaySchedules[userIndex].begin(), mYearDaySchedules[userIndex].end(),
                                [](const YearDayScheduleInfo & sch) { return sch.status == DlScheduleStatus::kOccupied; });

    const auto & user = mLockUsers[userIndex];
    if (user.userType != UserTypeEnum::kScheduleRestrictedUser && user.userType != UserTypeEnum::kYearDayScheduleUser)
    {
        return false;
    }

    if (user.userType == UserTypeEnum::kScheduleRestrictedUser && !*haveSchedule)
    {
        // It's valid to not have any schedules of a given type; on its own this
        // does not prevent access.
        return false;
    }

    chip::System::Clock::Milliseconds64 cTMs;
    auto chipError = chip::System::SystemClock().GetClock_RealTimeMS(cTMs);
    if (chipError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Lock App: unable to get current time to check user schedules [endpointId=%d (%s)]", mEndpointId,
                     chipError.AsString());
        return true;
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

    return !std::any_of(mYearDaySchedules[userIndex].begin(), mYearDaySchedules[userIndex].end(),
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
    case DlLockState::kUnlatched:
        return "Unlatched";
    case DlLockState::kUnknownEnumValue:
        break;
    }

    return "Unknown";
}
#endif // MATTER_DM_PLUGIN_DOOR_LOCK_SERVER
