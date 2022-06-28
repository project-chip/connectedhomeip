/*
 *
 *    Copyright (c) 2019 Google LLC.
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

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

using namespace ::chip;

// up to 10 users are supported on the EFR32 platform
#define DOOR_LOCK_MAX_USERS 10
// max credential size supported is 8
#define DOOR_LOCK_MAX_CREDENTIAL_SIZE 8
// up to 10 schedules of each type supported
#define DOOR_LOCK_MAX_SCHEDULES 10

// up to 10 credentials are supported per user
#define MAX_CREDENTIAL_PER_USER 10

// up to 10 schedules are supported per user
#define MAX_SCHEDULE_PER_USER 10

// user, credential and schedule indices are 1-indexed
#define MIN_USER_INDEX 1
#define MIN_CREDENTIAL_INDEX 1
#define MIN_SCHEDULE_INDEX 1

// 10 users with 10 credentials each max
#define MAX_CREDENTIALS 100

class LockManager
{
public:
    enum Action_t
    {
        LOCK_ACTION = 0,
        UNLOCK_ACTION,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_LockInitiated = 0,
        kState_LockCompleted,
        kState_UnlockInitiated,
        kState_UnlockCompleted,
    } State;

    CHIP_ERROR Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state,
                    uint8_t maxNumberOfCredentialsPerUser, uint16_t numberOfSupportedUsers,
                    uint8_t numberOfWeekdaySchedulesPerUserSupported, uint8_t numberOfYeardaySchedulesPerUserSupported,
                    uint8_t numberOfHolidaySchedulesPerUserSupported);
    bool NextState();
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action_t aAction);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    bool Lock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, DlOperationError & err);
    bool Unlock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, DlOperationError & err);

    bool GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const;
    bool SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                 const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                 DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials);

    bool GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential) const;

    bool SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                       DlCredentialStatus credentialStatus, DlCredentialType credentialType, const chip::ByteSpan & credentialData);

    DlStatus GetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                EmberAfPluginDoorLockWeekDaySchedule & schedule);

    DlStatus SetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex, DlScheduleStatus status,
                                DlDaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute, uint8_t endHour, uint8_t endMinute);

    DlStatus GetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                EmberAfPluginDoorLockYearDaySchedule & schedule);

    DlStatus SetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex, DlScheduleStatus status,
                                uint32_t localStartTime, uint32_t localEndTime);

    DlStatus GetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, EmberAfPluginDoorLockHolidaySchedule & schedule);

    DlStatus SetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status, uint32_t localStartTime,
                                uint32_t localEndTime, DlOperatingMode operatingMode);

    bool setLockState(chip::EndpointId endpointId, DlLockState lockState, const Optional<chip::ByteSpan> & pin,
                      DlOperationError & err);
    const char * lockStateToString(DlLockState lockState) const;

    bool ReadConfigValues();

private:
    friend LockManager & LockMgr();
    chip::EndpointId mEndpointId;
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void AutoLockTimerEventHandler(AppEvent * aEvent);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    EmberAfPluginDoorLockUserInfo mLockUsers[DOOR_LOCK_MAX_USERS];
    EmberAfPluginDoorLockCredentialInfo mLockCredentials[MAX_CREDENTIALS];
    EmberAfPluginDoorLockWeekDaySchedule mWeekdaySchedule[DOOR_LOCK_MAX_USERS][DOOR_LOCK_MAX_SCHEDULES];
    EmberAfPluginDoorLockYearDaySchedule mYeardaySchedule[DOOR_LOCK_MAX_USERS][DOOR_LOCK_MAX_SCHEDULES];
    EmberAfPluginDoorLockHolidaySchedule mHolidaySchedule[DOOR_LOCK_MAX_SCHEDULES];

    uint8_t mMaxCredentialsPerUser;
    uint16_t mMaxUsers;
    uint8_t mNumberOfWeekdaySchedulesPerUserSupported;
    uint8_t mNumberOfYeardaySchedulesPerUserSupported;
    uint8_t mNumberOfHolidaySchedulesSupported;

    char mUserNames[ArraySize(mLockUsers)][DOOR_LOCK_MAX_USER_NAME_SIZE];
    uint8_t mCredentialData[MAX_CREDENTIALS][DOOR_LOCK_MAX_CREDENTIAL_SIZE];
    DlCredential mCredentials[DOOR_LOCK_MAX_USERS][MAX_CREDENTIAL_PER_USER];

    static LockManager sLock;
};

inline LockManager & LockMgr()
{
    return LockManager::sLock;
}
