/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <lib/core/ClusterEnums.h>
#include <lib/support/ScopedBuffer.h>

#include <zephyr/kernel.h>

#include <cstdint>

class AppEvent;

class BoltLockManager
{
public:
    static constexpr size_t kMaxCredentialLength = 128;

    enum class State : uint8_t
    {
        kLockingInitiated = 0,
        kLockingCompleted,
        kUnlockingInitiated,
        kUnlockingCompleted,
    };

    struct UserData
    {
        char mName[DOOR_LOCK_USER_NAME_BUFFER_SIZE];
        CredentialStruct mCredentials[CONFIG_LOCK_NUM_CREDENTIALS_PER_USER];
    };

    struct CredentialData
    {
        chip::Platform::ScopedMemoryBuffer<uint8_t> mSecret;
    };

    using OperationSource     = chip::app::Clusters::DoorLock::OperationSourceEnum;
    using StateChangeCallback = void (*)(State, OperationSource);

    static constexpr uint32_t kActuatorMovementTimeMs = 2000;

    void Init(StateChangeCallback callback);

    State GetState() const { return mState; }
    bool IsLocked() const { return mState == State::kLockingCompleted; }

    bool GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const;
    bool SetUser(uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier, const chip::CharSpan & userName,
                 uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType, CredentialRuleEnum credentialRule,
                 const CredentialStruct * credentials, size_t totalCredentials);

    bool GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential) const;
    bool SetCredential(uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                       DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType, const chip::ByteSpan & secret);

    bool ValidatePIN(const Optional<chip::ByteSpan> & pinCode, OperationErrorEnum & err) const;

    void Lock(OperationSource source);
    void Unlock(OperationSource source);

private:
    void SetState(State state, OperationSource source);

    static void ActuatorTimerEventHandler(k_timer * timer);
    static void ActuatorAppEventHandler(const AppEvent & aEvent);
    friend BoltLockManager & BoltLockMgr();

    State mState                             = State::kLockingCompleted;
    StateChangeCallback mStateChangeCallback = nullptr;
    OperationSource mActuatorOperationSource = OperationSource::kButton;
    k_timer mActuatorTimer                   = {};

    UserData mUserData[CONFIG_LOCK_NUM_USERS];
    EmberAfPluginDoorLockUserInfo mUsers[CONFIG_LOCK_NUM_USERS] = {};

    CredentialData mCredentialData[CONFIG_LOCK_NUM_CREDENTIALS];
    EmberAfPluginDoorLockCredentialInfo mCredentials[CONFIG_LOCK_NUM_CREDENTIALS] = {};

    static BoltLockManager sLock;
};

inline BoltLockManager & BoltLockMgr()
{
    return BoltLockManager::sLock;
}
