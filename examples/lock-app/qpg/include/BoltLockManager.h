/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdbool.h>
#include <stdint.h>

#include "AppConfig.h"
#include "AppEvent.h"

#include <app/clusters/door-lock-server/door-lock-server.h>

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>
#include <lib/core/ClusterEnums.h>

class BoltLockManager
{
public:
    static constexpr size_t kMaxCredentialLength = 128;
    enum Action_t
    {
        LOCK_ACTION = 0,
        UNLOCK_ACTION,

        INVALID_ACTION
    } Action;

    enum State_t
    {
        kState_LockingInitiated = 0,
        kState_LockingCompleted,
        kState_UnlockingInitiated,
        kState_UnlockingCompleted,
    } State;

    struct UserData
    {
        char mName[DOOR_LOCK_USER_NAME_BUFFER_SIZE];
        CredentialStruct mCredentials[CONFIG_LOCK_NUM_CREDENTIALS_PER_USER];
    };

    struct CredentialData
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> mSecret;
    };

    CHIP_ERROR Init();
    bool IsUnlocked();
    bool IsActionInProgress();
    bool InitiateAction(int32_t aActor, Action_t aAction);

    typedef void (*Callback_fn_initiated)(Action_t, int32_t aActor);
    typedef void (*Callback_fn_completed)(Action_t);
    void SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB);
    bool GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const;
    bool SetUser(uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier, const chip::CharSpan & userName,
                 uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType, CredentialRuleEnum credentialRule,
                 const CredentialStruct * credentials, size_t totalCredentials);

    bool GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential) const;
    bool SetCredential(uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                       DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType, const chip::ByteSpan & secret);

    bool ValidatePIN(const Optional<chip::ByteSpan> & pinCode, OperationErrorEnum & err) const;

private:
    friend BoltLockManager & BoltLockMgr(void);
    State_t mState;

    Callback_fn_initiated mActionInitiated_CB;
    Callback_fn_completed mActionCompleted_CB;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static void TimerEventHandler(TimerHandle_t xTimer);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);

    static BoltLockManager sLock;

    UserData mUserData[CONFIG_LOCK_NUM_USERS];
    EmberAfPluginDoorLockUserInfo mUsers[CONFIG_LOCK_NUM_USERS] = {};

    CredentialData mCredentialData[CONFIG_LOCK_NUM_CREDENTIALS];
    EmberAfPluginDoorLockCredentialInfo mCredentials[CONFIG_LOCK_NUM_CREDENTIALS] = {};
};

inline BoltLockManager & BoltLockMgr(void)
{
    return BoltLockManager::sLock;
}
