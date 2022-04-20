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

static constexpr size_t DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE = 20;


struct LockUserInfo;
struct LockCredentialInfo;
struct WeekDaysScheduleInfo;
struct YearDayScheduleInfo;

struct LockUserInfo
{
    char userName[DOOR_LOCK_USER_NAME_BUFFER_SIZE];
    DlCredential credentials[DOOR_LOCK_MAX_CREDENTIALS_PER_USER];
    size_t totalCredentials;
    uint32_t userUniqueId;
    DlUserStatus userStatus;
    DlUserType userType;
    DlCredentialRule credentialRule;
    chip::FabricIndex createdBy;
    chip::FabricIndex lastModifiedBy;
};

struct LockCredentialInfo
{
    DlCredentialStatus status;
    DlCredentialType credentialType;
    uint8_t credentialData[DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE];
    size_t credentialDataSize;
};

struct WeekDaysScheduleInfo
{
    DlScheduleStatus status;
    EmberAfPluginDoorLockWeekDaySchedule schedule;
};

struct YearDayScheduleInfo
{
    DlScheduleStatus status;
    EmberAfPluginDoorLockYearDaySchedule schedule;
};
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

    CHIP_ERROR Init();
    bool IsUnlocked();
    void EnableAutoTurnOff(bool aOn);
    void SetAutoTurnOffDuration(uint32_t aDurationInSecs);
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action_t aAction);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);

    bool Lock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, DlOperationError & err);
    bool Unlock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, DlOperationError & err);

    bool GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const;
    bool SetUser(uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier, const chip::CharSpan & userName,
                 uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype, DlCredentialRule credentialRule,
                 const DlCredential * credentials, size_t totalCredentials);

    bool GetCredential(chip::EndpointId endpointId, DlCredentialType credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential) const;

    bool SetCredential(chip::EndpointId endpointId, DlCredentialStatus credentialStatus, DlCredentialType credentialType,
                       const chip::ByteSpan & credentialData);
private:
    bool setLockState(DlLockState lockState, const Optional<chip::ByteSpan> & pin, DlOperationError & err);
    const char * lockStateToString(DlLockState lockState) const;

    friend LockManager & LockMgr();
    chip::EndpointId mEndpointId;
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    bool mAutoTurnOff;
    uint32_t mAutoTurnOffDuration;
    bool mAutoTurnOffTimerArmed;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void AutoTurnOffTimerEventHandler(AppEvent * aEvent);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    LockUserInfo mLockUser;
    LockCredentialInfo mLockCredentials;

    static LockManager sLock;
};

inline LockManager & LockMgr()
{
    return LockManager::sLock;
}