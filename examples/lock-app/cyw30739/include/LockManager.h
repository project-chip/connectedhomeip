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

#include "AppEvent.h"
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <chip_lock.h>
#include <lib/core/CHIPError.h>
#include <lib/support/logging/CHIPLogging.h>
#include <stdint.h>
#include <wiced_timer.h>

using namespace ::chip;

// Currently up to 10 users are support on the CYW30739 platform
#define DOOR_LOCK_MAX_USERS 10
#define DOOR_LOCK_MAX_CREDENTIAL_SIZE 8
#define MININUM_USER_INDEX 1
#define MINIMUM_CREDENTIAL_INDEX 1
#define MAX_CREDENTIAL_PER_USER 10
#define MAX_CREDENTIALS 50

static constexpr size_t DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE = 20;

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

    enum Actor_t
    {
        ACTOR_ZCL_CMD = 0,
        ACTOR_APP_CMD,
        ACTOR_BUTTON,
    } Actor;

    CHIP_ERROR Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state,
                    uint8_t maxNumberOfCredentialsPerUser, uint16_t numberOfSupportedUsers);
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

    static void TimerEventHandler(WICED_TIMER_PARAM_TYPE cb_params);
    static int ActuatorMovementTimerEventHandler(void * aEvent);

    EmberAfPluginDoorLockUserInfo mLockUsers[DOOR_LOCK_MAX_USERS];
    EmberAfPluginDoorLockCredentialInfo mLockCredentials[MAX_CREDENTIALS];

    uint8_t mMaxCredentialsPerUser;
    uint16_t mMaxUsers;

    char mUserNames[ArraySize(mLockUsers)][DOOR_LOCK_MAX_USER_NAME_SIZE];
    uint8_t mCredentialData[MAX_CREDENTIALS][DOOR_LOCK_MAX_CREDENTIAL_SIZE];
    chip::Platform::ScopedMemoryBuffer<DlCredential> mCredentials[MAX_CREDENTIAL_PER_USER];

    static LockManager sLock;
};

inline LockManager & LockMgr()
{
    return LockManager::sLock;
}
