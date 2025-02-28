/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#pragma once
#include <app/clusters/door-lock-server/door-lock-server.h>

#include <stdbool.h>
#include <stdint.h>
#include <zephyr/kernel.h>

#include <AppConfig.h>
#include <AppEventCommon.h>

#include <lib/core/CHIPError.h>

struct WeekDaysScheduleInfo
{
    DlScheduleStatus status = DlScheduleStatus::kAvailable;
    EmberAfPluginDoorLockWeekDaySchedule schedule;
};

struct YearDayScheduleInfo
{
    DlScheduleStatus status = DlScheduleStatus::kAvailable;
    EmberAfPluginDoorLockYearDaySchedule schedule;
};

struct HolidayScheduleInfo
{
    DlScheduleStatus status = DlScheduleStatus::kAvailable;
    EmberAfPluginDoorLockHolidaySchedule schedule;
};

namespace TelinkDoorLock {
namespace ResourceRanges {
// Used to size arrays
static constexpr uint8_t kMaxCredentialSize  = 20;
static constexpr uint8_t kNumCredentialTypes = 6;

} // namespace ResourceRanges

namespace LockInitParams {

struct LockParam
{
    // Read from zap attributes
    uint16_t numberOfUsers                  = 0;
    uint8_t numberOfCredentialsPerUser      = 0;
    uint8_t numberOfWeekdaySchedulesPerUser = 0;
    uint8_t numberOfYeardaySchedulesPerUser = 0;
    uint8_t numberOfHolidaySchedules        = 0;
};

class ParamBuilder
{
public:
    ParamBuilder & SetNumberOfUsers(uint16_t numberOfUsers)
    {
        lockParam_.numberOfUsers = numberOfUsers;
        return *this;
    }
    ParamBuilder & SetNumberOfCredentialsPerUser(uint8_t numberOfCredentialsPerUser)
    {
        lockParam_.numberOfCredentialsPerUser = numberOfCredentialsPerUser;
        return *this;
    }
    ParamBuilder & SetNumberOfWeekdaySchedulesPerUser(uint8_t numberOfWeekdaySchedulesPerUser)
    {
        lockParam_.numberOfWeekdaySchedulesPerUser = numberOfWeekdaySchedulesPerUser;
        return *this;
    }
    ParamBuilder & SetNumberOfYeardaySchedulesPerUser(uint8_t numberOfYeardaySchedulesPerUser)
    {
        lockParam_.numberOfYeardaySchedulesPerUser = numberOfYeardaySchedulesPerUser;
        return *this;
    }
    ParamBuilder & SetNumberOfHolidaySchedules(uint8_t numberOfHolidaySchedules)
    {
        lockParam_.numberOfHolidaySchedules = numberOfHolidaySchedules;
        return *this;
    }
    LockParam GetLockParam() { return lockParam_; }

private:
    LockParam lockParam_;
};

} // namespace LockInitParams
} // namespace TelinkDoorLock

using namespace ::chip;
using namespace TelinkDoorLock::ResourceRanges;

class LockManager
{
public:
    enum Action_t
    {
        LOCK_ACTION = 0,
        UNLOCK_ACTION,
        UNBOLT_ACTION,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_LockInitiated = 0,
        kState_LockCompleted,
        kState_UnlockInitiated,
        kState_UnlockCompleted,
        kState_UnlatchInitiated,
        kState_UnlatchCompleted,
        kState_NotFulyLocked
    } State;

    using StateChangeCallback = void (*)(State_t);
    using OperationSource     = chip::app::Clusters::DoorLock::OperationSourceEnum;

    CHIP_ERROR Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state,
                    TelinkDoorLock::LockInitParams::LockParam lockParam, StateChangeCallback callback);

    bool LockAction(int32_t appSource, Action_t aAction, OperationSource source, chip::EndpointId endpointId);

    bool LockAction(int32_t appSource, Action_t aAction, OperationSource source, chip::EndpointId endpointId,
                    OperationErrorEnum & err, const Nullable<chip::FabricIndex> & fabricIdx = NullNullable,
                    const Nullable<chip::NodeId> & nodeId = NullNullable, const Optional<chip::ByteSpan> & pinCode = NullNullable);

    bool IsLocked() const { return mState == State_t::kState_LockCompleted; }

    State_t getLockState() { return mState; }

    bool GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);
    bool SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                 const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                 CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials);

    bool GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential);

    bool SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                       DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                       const chip::ByteSpan & credentialData);

    DlStatus GetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                EmberAfPluginDoorLockWeekDaySchedule & schedule);

    DlStatus SetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
                                DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute);

    DlStatus GetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                EmberAfPluginDoorLockYearDaySchedule & schedule);

    DlStatus SetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                uint32_t localStartTime, uint32_t localEndTime);

    DlStatus GetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule & schedule);

    DlStatus SetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime,
                                uint32_t localEndTime, OperatingModeEnum operatingMode);

    bool IsValidUserIndex(uint16_t userIndex);
    bool IsValidCredentialIndex(uint16_t credentialIndex, CredentialTypeEnum type);
    bool IsValidCredentialType(CredentialTypeEnum type);
    bool IsValidWeekdayScheduleIndex(uint8_t scheduleIndex);
    bool IsValidYeardayScheduleIndex(uint8_t scheduleIndex);
    bool IsValidHolidayScheduleIndex(uint8_t scheduleIndex);

    const char * lockStateToString(DlLockState lockState) const;

private:
    friend LockManager & LockMgr();
    State_t mState                           = kState_NotFulyLocked;
    StateChangeCallback mStateChangeCallback = nullptr;
    OperationSource mActuatorOperationSource = OperationSource::kButton;
    k_timer mActuatorTimer                   = {};

    bool setLockState(chip::EndpointId endpointId, DlLockState lockState, OperationSource source, OperationErrorEnum & err,
                      const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                      const Optional<chip::ByteSpan> & pin);

    static void ActuatorTimerEventHandler(k_timer * timer);
    static void ActuatorAppEventHandler(const AppEvent & event);

#if !LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    EmberAfPluginDoorLockUserInfo mLockUsers[APP_MAX_USERS];
    EmberAfPluginDoorLockCredentialInfo mLockCredentials[kNumCredentialTypes][APP_MAX_CREDENTIAL];
    WeekDaysScheduleInfo mWeekdaySchedule[APP_MAX_USERS][APP_MAX_WEEKDAY_SCHEDULE_PER_USER];
    YearDayScheduleInfo mYeardaySchedule[APP_MAX_USERS][APP_MAX_YEARDAY_SCHEDULE_PER_USER];
    HolidayScheduleInfo mHolidaySchedule[APP_MAX_HOLYDAY_SCHEDULE_PER_USER];

    char mUserNames[MATTER_ARRAY_SIZE(mLockUsers)][DOOR_LOCK_MAX_USER_NAME_SIZE];
    uint8_t mCredentialData[kNumCredentialTypes][APP_MAX_CREDENTIAL][kMaxCredentialSize];
    CredentialStruct mCredentials[APP_MAX_USERS][APP_MAX_CREDENTIAL];
#endif

    static LockManager sLock;
    TelinkDoorLock::LockInitParams::LockParam LockParams;
};

inline LockManager & LockMgr()
{
    return LockManager::sLock;
}
