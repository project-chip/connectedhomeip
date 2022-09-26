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

using namespace chip;

BoltLockManager BoltLockManager::sLock;

TimerHandle_t sLockTimer;
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
StaticTimer_t sLockTimerBuffer;
#endif

CHIP_ERROR BoltLockManager::Init()
{
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
    sLockTimer = xTimerCreateStatic("lockTmr",         // Just a text name, not used by the RTOS kernel
                                    1,                 // == default timer period (mS)
                                    false,             // no timer reload (==one-shot)
                                    (void *) this,     // init timer id = ble obj context
                                    TimerEventHandler, // timer callback handler
                                    &sLockTimerBuffer  // static buffer for timer

    );
#else
    // Create FreeRTOS sw timer for lock timer.
    sLockTimer = xTimerCreate("lockTmr",        // Just a text name, not used by the RTOS kernel
                              1,                // == default timer period (mS)
                              false,            // no timer reload (==one-shot)
                              (void *) this,    // init timer id = lock obj context
                              TimerEventHandler // timer callback handler
    );
#endif
    if (sLockTimer == NULL)
    {
        ChipLogProgress(NotSpecified, "sLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    mState              = kState_LockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;

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

void BoltLockManager::EnableAutoRelock(bool aOn)
{
    mAutoRelock = aOn;
}

void BoltLockManager::SetAutoLockDuration(uint32_t aDurationInSecs)
{
    mAutoLockDuration = aDurationInSecs;
}

bool BoltLockManager::GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const
{
    user = mUsers[userIndex - 1];

    ChipLogProgress(Zcl, "Getting lock user %u: %s", static_cast<unsigned>(userIndex),
                    user.userStatus == DlUserStatus::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::SetUser(uint16_t userIndex, FabricIndex creator, FabricIndex modifier, const CharSpan & userName,
                              uint32_t uniqueId, DlUserStatus userStatus, DlUserType userType, DlCredentialRule credentialRule,
                              const DlCredential * credentials, size_t totalCredentials)
{
    UserData & userData = mUserData[userIndex - 1];
    auto & user         = mUsers[userIndex - 1];

    VerifyOrReturnError(userName.size() <= DOOR_LOCK_MAX_USER_NAME_SIZE, false);
    VerifyOrReturnError(totalCredentials <= CONFIG_LOCK_NUM_CREDENTIALS_PER_USER, false);

    Platform::CopyString(userData.mName, userName);
    memcpy(userData.mCredentials, credentials, totalCredentials * sizeof(DlCredential));

    user.userName           = CharSpan(userData.mName, userName.size());
    user.credentials        = Span<const DlCredential>(userData.mCredentials, totalCredentials);
    user.userUniqueId       = uniqueId;
    user.userStatus         = userStatus;
    user.userType           = userType;
    user.credentialRule     = credentialRule;
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = creator;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = modifier;

    ChipLogProgress(Zcl, "Setting lock user %u: %s", static_cast<unsigned>(userIndex),
                    userStatus == DlUserStatus::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::GetCredential(uint16_t credentialIndex, DlCredentialType credentialType,
                                    EmberAfPluginDoorLockCredentialInfo & credential) const
{
    VerifyOrReturnError(credentialIndex > 0 && credentialIndex <= CONFIG_LOCK_NUM_CREDENTIALS, false);

    credential = mCredentials[credentialIndex - 1];

    ChipLogProgress(Zcl, "Getting lock credential %u: %s", static_cast<unsigned>(credentialIndex),
                    credential.status == DlCredentialStatus::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::SetCredential(uint16_t credentialIndex, FabricIndex creator, FabricIndex modifier,
                                    DlCredentialStatus credentialStatus, DlCredentialType credentialType, const ByteSpan & secret)
{
    VerifyOrReturnError(credentialIndex > 0 && credentialIndex <= CONFIG_LOCK_NUM_CREDENTIALS, false);
    VerifyOrReturnError(secret.size() <= kMaxCredentialLength, false);

    CredentialData & credentialData = mCredentialData[credentialIndex - 1];
    auto & credential               = mCredentials[credentialIndex - 1];

    if (!secret.empty())
    {
        memcpy(credentialData.mSecret.Alloc(secret.size()).Get(), secret.data(), secret.size());
    }

    credential.status             = credentialStatus;
    credential.credentialType     = credentialType;
    credential.credentialData     = ByteSpan(credentialData.mSecret.Get(), secret.size());
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.createdBy          = creator;
    credential.modificationSource = DlAssetSource::kMatterIM;
    credential.lastModifiedBy     = modifier;

    ChipLogProgress(Zcl, "Setting lock credential %u: %s", static_cast<unsigned>(credentialIndex),
                    credential.status == DlCredentialStatus::kAvailable ? "available" : "occupied");

    return true;
}

bool BoltLockManager::ValidatePIN(const Optional<ByteSpan> & pinCode, DlOperationError & err) const
{
    // Optionality of the PIN code is validated by the caller, so assume it is OK not to provide the PIN code.
    if (!pinCode.HasValue())
    {
        return true;
    }
    ChipLogProgress(Zcl, "ValidatePIN %.*s", static_cast<int>(pinCode.Value().size()), pinCode.Value().data());

    // Check the PIN code
    for (const auto & credential : mCredentials)
    {
        if (credential.status == DlCredentialStatus::kAvailable || credential.credentialType != DlCredentialType::kPin)
        {
            continue;
        }

        if (credential.credentialData.data_equal(pinCode.Value()))
        {
            ChipLogDetail(Zcl, "Valid lock PIN code provided");
            return true;
        }
    }

    ChipLogDetail(Zcl, "Invalid lock PIN code provided");
    err = DlOperationError::kInvalidCredential;

    return false;
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
        if (mAutoLockTimerArmed && new_state == kState_LockingInitiated)
        {
            // If auto lock timer has been armed and someone initiates locking,
            // cancel the timer and continue as normal.
            mAutoLockTimerArmed = false;

            CancelTimer();
        }

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
        // appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BoltLockManager::CancelTimer(void)
{
    if (xTimerStop(sLockTimer, 0) == pdFAIL)
    {
        ChipLogError(NotSpecified, "Lock timer timer stop() failed");
        // appError(APP_ERROR_STOP_TIMER_FAILED);
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
    if (lock->mAutoLockTimerArmed)
    {
        event.Handler = AutoReLockTimerEventHandler;
    }
    else
    {
        event.Handler = ActuatorMovementTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::AutoReLockTimerEventHandler(AppEvent * aEvent)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->TimerEvent.Context);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
    {
        return;
    }

    lock->mAutoLockTimerArmed = false;

    ChipLogProgress(NotSpecified, "Auto Re-Lock has been triggered!");

    lock->InitiateAction(actor, LOCK_ACTION);
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

        if (lock->mAutoRelock && actionCompleted == UNLOCK_ACTION)
        {
            // Start the timer for auto relock
            lock->StartTimer(lock->mAutoLockDuration * 1000);

            lock->mAutoLockTimerArmed = true;

            ChipLogProgress(NotSpecified, "Auto Re-lock enabled. Will be triggered in %" PRIu32 " seconds",
                            lock->mAutoLockDuration);
        }
    }
}
