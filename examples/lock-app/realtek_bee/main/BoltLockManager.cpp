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

#include "BoltLockManager.h"

#include "AppConfig.h"
#include "AppTask.h"
#include <FreeRTOS.h>
#include <app-common/zap-generated/attributes/Accessors.h>

using namespace chip;
using BeeConfig = chip::DeviceLayer::Internal::BeeConfig;

BoltLockManager BoltLockManager::sLock;

TimerHandle_t sLockTimer;

CHIP_ERROR BoltLockManager::Init()
{
    // Create FreeRTOS sw timer for lock timer.
    sLockTimer = xTimerCreate("lockTmr",        // Just a text name, not used by the RTOS kernel
                              1,                // == default timer period (mS)
                              false,            // no timer reload (==one-shot)
                              (void *) this,    // init timer id = lock obj context
                              TimerEventHandler // timer callback handler
    );
    if (sLockTimer == nullptr)
    {
        ChipLogProgress(NotSpecified, "sLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    mState = kState_LockingCompleted;

    return CHIP_NO_ERROR;
}

void BoltLockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool BoltLockManager::IsActionInProgress()
{
    return (mState == kState_LockingInitiated || mState == kState_UnlockingInitiated);
}

bool BoltLockManager::IsUnlocked()
{
    return (mState == kState_UnlockingCompleted);
}

bool BoltLockManager::ReadConfigValues()
{
    size_t outLen;
    BeeConfig::ReadConfigValueBin(BeeConfig::kConfigKey_LockUser, reinterpret_cast<uint8_t *>(mUsers),
                                  sizeof(EmberAfPluginDoorLockUserInfo) * ArraySize(mUsers), outLen);

    BeeConfig::ReadConfigValueBin(BeeConfig::kConfigKey_LockUserData, reinterpret_cast<uint8_t *>(mUserData),
                                  sizeof(UserData) * ArraySize(mUserData), outLen);

    BeeConfig::ReadConfigValueBin(BeeConfig::kConfigKey_Credential, reinterpret_cast<uint8_t *>(mCredentials),
                                  sizeof(EmberAfPluginDoorLockCredentialInfo) * ArraySize(mCredentials), outLen);

    BeeConfig::ReadConfigValueBin(BeeConfig::kConfigKey_CredentialData, reinterpret_cast<uint8_t *>(mCredentialData),
                                  sizeof(mCredentialData), outLen);

    return true;
}

bool BoltLockManager::GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const
{
    VerifyOrReturnError(userIndex > 0 && userIndex <= CONFIG_LOCK_NUM_USERS, false);

    user = mUsers[userIndex - 1];

    ChipLogProgress(Zcl, "Getting lock user %u: %s", static_cast<unsigned>(userIndex),
                    user.userStatus == UserStatusEnum::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::SetUser(uint16_t userIndex, FabricIndex creator, FabricIndex modifier, const CharSpan & userName,
                              uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum userType,
                              CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    VerifyOrReturnError(userIndex > 0 && userIndex <= CONFIG_LOCK_NUM_USERS, false);
    VerifyOrReturnError(userName.size() <= DOOR_LOCK_MAX_USER_NAME_SIZE, false);
    VerifyOrReturnError(totalCredentials <= CONFIG_LOCK_NUM_CREDENTIALS_PER_USER, false);

    UserData & userData = mUserData[userIndex - 1];
    auto & user         = mUsers[userIndex - 1];

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

    // Save user information in NVM flash
    BeeConfig::WriteConfigValueBin(BeeConfig::kConfigKey_LockUser, reinterpret_cast<const uint8_t *>(mUsers),
                                   sizeof(EmberAfPluginDoorLockUserInfo) * CONFIG_LOCK_NUM_USERS);

    BeeConfig::WriteConfigValueBin(BeeConfig::kConfigKey_LockUserData, reinterpret_cast<const uint8_t *>(mUserData),
                                   sizeof(UserData) * CONFIG_LOCK_NUM_USERS);

    ChipLogProgress(Zcl, "Successfully set the user [index=%d]", userIndex);

    return true;
}

bool BoltLockManager::GetCredential(uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                    EmberAfPluginDoorLockCredentialInfo & credential) const
{
    VerifyOrReturnError(credentialIndex > 0 && credentialIndex <= CONFIG_LOCK_NUM_CREDENTIALS, false);

    credential = mCredentials[credentialIndex - 1];

    ChipLogProgress(Zcl, "Getting lock credential %u: %s", static_cast<unsigned>(credentialIndex),
                    credential.status == DlCredentialStatus::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::SetCredential(uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                    DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType, const ByteSpan & secret)
{
    VerifyOrReturnError(credentialIndex > 0 && credentialIndex <= CONFIG_LOCK_NUM_CREDENTIALS, false);
    VerifyOrReturnError(secret.size() <= kMaxCredentialLength, false);

    auto & credentialData = mCredentialData[credentialIndex - 1];
    auto & credential     = mCredentials[credentialIndex - 1];

    memcpy(credentialData, secret.data(), secret.size());

    credential.status             = credentialStatus;
    credential.credentialType     = credentialType;
    credential.credentialData     = ByteSpan(credentialData, secret.size());
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.createdBy          = creator;
    credential.modificationSource = DlAssetSource::kMatterIM;
    credential.lastModifiedBy     = modifier;

    BeeConfig::WriteConfigValueBin(BeeConfig::kConfigKey_Credential, reinterpret_cast<const uint8_t *>(mCredentials),
                                   sizeof(EmberAfPluginDoorLockCredentialInfo) * CONFIG_LOCK_NUM_CREDENTIALS);

    BeeConfig::WriteConfigValueBin(BeeConfig::kConfigKey_CredentialData, reinterpret_cast<const uint8_t *>(mCredentialData),
                                   CONFIG_LOCK_NUM_CREDENTIALS * kMaxCredentialLength);

    ChipLogProgress(Zcl, "Setting lock credential %u: %s", static_cast<unsigned>(credentialIndex),
                    credential.status == DlCredentialStatus::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Lock/Unlock Action only when the previous one is complete.
    if (mState == kState_LockingCompleted && aAction == UNLOCK_ACTION)
    {
        action_initiated = true;

        new_state = kState_UnlockingInitiated;
    }
    else if (mState == kState_UnlockingCompleted && aAction == LOCK_ACTION)
    {
        action_initiated = true;

        new_state = kState_LockingInitiated;
    }

    if (action_initiated)
    {
        StartTimer(ACTUATOR_MOVEMENT_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction, aActor);
        }
    }

    return action_initiated;
}

void BoltLockManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sLockTimer))
    {
        ChipLogError(NotSpecified, "app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sLockTimer, (aTimeoutMs / portTICK_PERIOD_MS), 100) != pdPASS)
    {
        ChipLogError(NotSpecified, "sLockTimer timer start() failed");
    }
}

void BoltLockManager::CancelTimer(void)
{
    if (xTimerStop(sLockTimer, 0) == pdFAIL)
    {
        ChipLogError(NotSpecified, "Lock timer timer stop() failed");
    }
}

void BoltLockManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get lock obj context from timer id.
    BoltLockManager * lock = static_cast<BoltLockManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = lock;
    event.Handler            = ActuatorMovementTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);

    if (lock->mState == kState_LockingInitiated)
    {
        lock->mState    = kState_LockingCompleted;
        actionCompleted = LOCK_ACTION;
    }
    else if (lock->mState == kState_UnlockingInitiated)
    {
        lock->mState    = kState_UnlockingCompleted;
        actionCompleted = UNLOCK_ACTION;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (lock->mActionCompleted_CB)
        {
            lock->mActionCompleted_CB(actionCompleted);
        }
    }
}

bool BoltLockManager::setLockState(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx, const Nullable<NodeId> & nodeId,
                                   DlLockState lockState, const Optional<ByteSpan> & pinCode, OperationErrorEnum & err)
{
    // Assume pin is required until told otherwise
    bool requirePin = true;
    app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(endpointId, &requirePin);

    // Optionality of the PIN code is validated by the caller, so assume it is OK not to provide the PIN code.
    if (!pinCode.HasValue())
    {
        // If a pin code is not required
        if (!requirePin)
        {
            ChipLogDetail(Zcl, "PIN code is not required");

            DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, NullNullable, NullNullable,
                                                    fabricIdx, nodeId);

            return true;
        }

        ChipLogError(Zcl, "PIN code is not specified, but it is required");

        return false;
    }

    // Check the PIN code
    for (const auto & credential : mCredentials)
    {
        if (credential.status == DlCredentialStatus::kAvailable || credential.credentialType != CredentialTypeEnum::kPin)
        {
            continue;
        }

        if (credential.credentialData.data_equal(pinCode.Value()))
        {
            ChipLogDetail(Zcl, "Valid lock PIN code provided");

            DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, NullNullable, NullNullable,
                                                    fabricIdx, nodeId);

            return true;
        }
    }

    ChipLogDetail(Zcl, "Invalid lock PIN code provided");
    err = OperationErrorEnum::kInvalidCredential;

    return false;
}

bool BoltLockManager::Lock(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx, const Nullable<NodeId> & nodeId,
                           const Optional<ByteSpan> & pin, OperationErrorEnum & err)
{
    return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kLocked, pin, err);
}

bool BoltLockManager::Unlock(EndpointId endpointId, const Nullable<FabricIndex> & fabricIdx, const Nullable<NodeId> & nodeId,
                             const Optional<ByteSpan> & pin, OperationErrorEnum & err)
{
    if (DoorLockServer::Instance().SupportsUnbolt(endpointId))
    {
        // If Unbolt is supported Unlock is supposed to pull the latch
        return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlatched, pin, err);
    }
    return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlocked, pin, err);
}
