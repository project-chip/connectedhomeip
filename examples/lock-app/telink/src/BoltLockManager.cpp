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

#include "BoltLockManager.h"

#include "AppConfig.h"
#include "AppEventCommon.h"
#include "AppTask.h"
#include <cstring>

using namespace chip;
using chip::to_underlying;

BoltLockManager BoltLockManager::sLock;

void BoltLockManager::Init(StateChangeCallback callback)
{
    mStateChangeCallback = callback;

    k_timer_init(&mActuatorTimer, &BoltLockManager::ActuatorTimerEventHandler, nullptr);
    k_timer_user_data_set(&mActuatorTimer, this);
}

bool BoltLockManager::GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const
{
    // userIndex is guaranteed by the caller to be between 1 and CONFIG_LOCK_NUM_USERS
    user = mUsers[userIndex - 1];

    ChipLogProgress(Zcl, "Getting lock user %u: %s", static_cast<unsigned>(userIndex),
                    user.userStatus == UserStatusEnum::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::SetUser(uint16_t userIndex, FabricIndex creator, FabricIndex modifier, const CharSpan & userName,
                              uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType,
                              CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    // userIndex is guaranteed by the caller to be between 1 and CONFIG_LOCK_NUM_USERS
    UserData & userData = mUserData[userIndex - 1];
    auto & user         = mUsers[userIndex - 1];

    VerifyOrReturnError(userName.size() <= DOOR_LOCK_MAX_USER_NAME_SIZE, false);
    VerifyOrReturnError(totalCredentials <= CONFIG_LOCK_NUM_CREDENTIALS_PER_USER, false);

    Platform::CopyString(userData.mName, userName);
    memcpy(userData.mCredentials, credentials, totalCredentials * sizeof(CredentialStruct));

    user.userName           = CharSpan(userData.mName, userName.size());
    user.credentials        = Span<const CredentialStruct>(userData.mCredentials, totalCredentials);
    user.userUniqueId       = uniqueId;
    user.userStatus         = userStatus;
    user.userType           = userType;
    user.credentialRule     = credentialRule;
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = creator;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = modifier;

    ChipLogProgress(Zcl, "Setting lock user %u: %s", static_cast<unsigned>(userIndex),
                    userStatus == UserStatusEnum::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                    EmberAfPluginDoorLockCredentialInfo & credential) const
{
    ChipLogProgress(Zcl, "Lock App: LockEndpoint::GetCredential [credentialIndex=%u,credentialType=%u]", credentialIndex,
                    to_underlying(credentialType));

    if (to_underlying(credentialType) >= mCredentials.size())
    {
        ChipLogError(Zcl, "Cannot get the credential - index out of range [index=%d]", credentialIndex);
        return false;
    }

    if (credentialIndex >= mCredentials.at(to_underlying(credentialType)).size() ||
        (0 == credentialIndex && CredentialTypeEnum::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot get the credential - index out of range [index=%d]", credentialIndex);
        return false;
    }

    const auto & credentialInStorage = mCredentials[to_underlying(credentialType)][credentialIndex];

    credential.status = credentialInStorage.status;
    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogDetail(Zcl, "Found unoccupied credential [index=%u]", credentialIndex);
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

    ChipLogDetail(Zcl, "Found occupied credential [index=%u,type=%u,dataSize=%u,createdBy=%u,modifiedBy=%u]", credentialIndex,
                  to_underlying(credential.credentialType), static_cast<unsigned int>(credential.credentialData.size()),
                  credential.createdBy, credential.lastModifiedBy);

    return true;
}

bool BoltLockManager::SetCredential(uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                    DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                    const ByteSpan & credentialData)
{
    ChipLogProgress(Zcl,
                    "Lock App: LockEndpoint::SetCredential "
                    "[credentialIndex=%u,credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%u,modifier=%u]",
                    credentialIndex, to_underlying(credentialStatus), to_underlying(credentialType),
                    static_cast<unsigned int>(credentialData.size()), creator, modifier);

    if (to_underlying(credentialType) >= mCredentials.capacity())
    {
        ChipLogError(Zcl, "Cannot set the credential - type out of range [type=%d]", to_underlying(credentialType));
        return false;
    }

    if (credentialIndex >= mCredentials.at(to_underlying(credentialType)).size() ||
        (0 == credentialIndex && CredentialTypeEnum::kProgrammingPIN != credentialType))
    {
        ChipLogError(Zcl, "Cannot set the credential - index out of range [index=%d]", credentialIndex);
        return false;
    }

    auto & credentialInStorage = mCredentials[to_underlying(credentialType)][credentialIndex];
    if (credentialData.size() > CONFIG_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE)
    {
        ChipLogError(Zcl,
                     "Cannot get the credential - data size exceeds limit "
                     "index=%d,dataSize=%u,maxDataSize=%u]",
                     credentialIndex, static_cast<unsigned int>(credentialData.size()),
                     static_cast<unsigned int>(CONFIG_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE));
        return false;
    }
    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    credentialInStorage.createdBy      = creator;
    credentialInStorage.modifiedBy     = modifier;
    std::memcpy(credentialInStorage.credentialData, credentialData.data(), credentialData.size());
    credentialInStorage.credentialDataSize = credentialData.size();

    ChipLogProgress(Zcl, "Successfully set the credential [index=%d,credentialType=%u,creator=%u,modifier=%u]", credentialIndex,
                    to_underlying(credentialType), credentialInStorage.createdBy, credentialInStorage.modifiedBy);

    return true;
}

bool BoltLockManager::ValidatePIN(const Optional<ByteSpan> & pinCode, OperationErrorEnum & err) const
{
    // Optionality of the PIN code is validated by the caller, so assume it is OK not to provide the PIN code.
    if (!pinCode.HasValue())
    {
        return true;
    }

    // Find the credential so we can make sure it is not absent right away
    auto & pinCredentials = mCredentials[to_underlying(CredentialTypeEnum::kPin)];
    auto credential       = std::find_if(pinCredentials.begin(), pinCredentials.end(), [&pinCode](const LockCredentialInfo & c) {
        return (c.status != DlCredentialStatus::kAvailable) &&
            chip::ByteSpan{ c.credentialData, c.credentialDataSize }.data_equal(pinCode.Value());
    });

    if (credential == pinCredentials.end())
    {
        ChipLogDetail(Zcl, "Lock App: specified PIN code was not found in the database");

        err = OperationErrorEnum::kInvalidCredential;
        return false;
    }

    ChipLogDetail(Zcl, "Invalid lock PIN code provided");
    err = OperationErrorEnum::kInvalidCredential;

    return false;
}

void BoltLockManager::Lock(OperationSource source)
{
    VerifyOrReturn(mState != State::kLockingCompleted);
    SetState(State::kLockingInitiated, source);

    mActuatorOperationSource = source;
    k_timer_start(&mActuatorTimer, K_MSEC(kActuatorMovementTimeMs), K_NO_WAIT);
}

void BoltLockManager::Unlock(OperationSource source)
{
    VerifyOrReturn(mState != State::kUnlockingCompleted);
    SetState(State::kUnlockingInitiated, source);

    mActuatorOperationSource = source;
    k_timer_start(&mActuatorTimer, K_MSEC(kActuatorMovementTimeMs), K_NO_WAIT);
}

void BoltLockManager::ActuatorTimerEventHandler(k_timer * timer)
{
    // The timer event handler is called in the context of the system clock ISR.
    // Post an event to the application task queue to process the event in the
    // context of the application thread.

    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = static_cast<BoltLockManager *>(k_timer_user_data_get(timer));
    event.Handler            = (EventHandler) BoltLockManager::ActuatorAppEventHandler;
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::ActuatorAppEventHandler(const AppEvent & event)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(event.TimerEvent.Context);

    if (!lock)
    {
        return;
    }

    switch (lock->mState)
    {
    case State::kLockingInitiated:
        lock->SetState(State::kLockingCompleted, lock->mActuatorOperationSource);
        break;
    case State::kUnlockingInitiated:
        lock->SetState(State::kUnlockingCompleted, lock->mActuatorOperationSource);
        break;
    default:
        break;
    }
}

void BoltLockManager::SetState(State state, OperationSource source)
{
    mState = state;

    if (mStateChangeCallback != nullptr)
    {
        mStateChangeCallback(state, source);
    }
}
