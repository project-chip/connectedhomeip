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

#include "LockManager.h"
#include "wiced_bt_event.h"
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::DeviceLayer::Internal;

LockManager LockManager::sLock;

wiced_timer_t sLockTimer;

CHIP_ERROR LockManager::Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state,
                             uint8_t maxNumberOfCredentialsPerUser)
{
    // Allocate buffer for credentials
    if (!mCredentials.Alloc(maxNumberOfCredentialsPerUser))
    {
        printf("Failed to allocate array for lock credentials");
        return CHIP_ERROR_NO_MEMORY;
    }
    mMaxCredentialsPerUser = maxNumberOfCredentialsPerUser;

    wiced_result_t result;

    // Create wiced timer for lock timer.
    result = wiced_init_timer(&sLockTimer, TimerEventHandler, (WICED_TIMER_PARAM_TYPE) this, WICED_MILLI_SECONDS_TIMER);
    if (result != WICED_BT_SUCCESS)
    {
        printf("sLockTimer timer create failed");
        return APP_ERROR_INIT_TIMER_FAILED;
    }

    if (state.Value() == DlLockState::kUnlocked)
        mState = kState_UnlockCompleted;
    else
        mState = kState_LockCompleted;

    return CHIP_NO_ERROR;
}

bool LockManager::ReadConfigValues()
{
    size_t outLen;
    CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_LockUser, reinterpret_cast<uint8_t *>(&mLockUser),
                                       sizeof(EmberAfPluginDoorLockUserInfo), outLen);
    CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_Credential, reinterpret_cast<uint8_t *>(&mLockCredentials),
                                       sizeof(EmberAfPluginDoorLockCredentialInfo), outLen);

    CYW30739Config::ReadConfigValueStr(CYW30739Config::kConfigKey_LockUserName, mUserName, DOOR_LOCK_USER_NAME_BUFFER_SIZE, outLen);

    CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_CredentialData, mCredentialData, sizeof(mCredentialData), outLen);

    CYW30739Config::ReadConfigValueBin(CYW30739Config::kConfigKey_UserCredentials, reinterpret_cast<uint8_t *>(mCredentials.Get()),
                                       sizeof(DlCredential), outLen);

    return true;
}

void LockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LockManager::IsActionInProgress()
{
    return (mState == kState_LockInitiated || mState == kState_UnlockInitiated);
}

bool LockManager::NextState()
{
    return (mState == kState_UnlockCompleted);
}

bool LockManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Lock/Unlock Action only when the previous one is complete.
    if (mState == kState_LockCompleted && aAction == UNLOCK_ACTION)
    {
        action_initiated = true;

        new_state = kState_UnlockInitiated;
    }
    else if (mState == kState_UnlockCompleted && aAction == LOCK_ACTION)
    {
        action_initiated = true;

        new_state = kState_LockInitiated;
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

void LockManager::StartTimer(uint32_t aTimeoutMs)
{
    if (wiced_is_timer_in_use(&sLockTimer))
    {
        printf("app timer already started!\n");
        CancelTimer();
    }

    if (wiced_start_timer(&sLockTimer, aTimeoutMs) != WICED_BT_SUCCESS)
    {
        printf("sLockTimer timer start() failed\n");
        return;
    }
}

void LockManager::CancelTimer(void)
{
    if (wiced_stop_timer(&sLockTimer) != WICED_BT_SUCCESS)
    {
        printf("Lock timer timer stop() failed\n");
        return;
    }
}

void LockManager::TimerEventHandler(WICED_TIMER_PARAM_TYPE cb_params)
{
    LockManager * lock = reinterpret_cast<LockManager *>(cb_params);
    int (*fn)(void *);

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    fn = ActuatorMovementTimerEventHandler;

    if (wiced_app_event_serialize(fn, (void *) lock) != WICED_TRUE)
    {
        printf("wiced_app_event_serialize failed\n");
    }
}

int LockManager::ActuatorMovementTimerEventHandler(void * data)
{
    Action_t actionCompleted = INVALID_ACTION;

    LockManager * lock = reinterpret_cast<LockManager *>(data);

    if (lock->mState == kState_LockInitiated)
    {
        lock->mState    = kState_LockCompleted;
        actionCompleted = LOCK_ACTION;
    }
    else if (lock->mState == kState_UnlockInitiated)
    {
        lock->mState    = kState_UnlockCompleted;
        actionCompleted = UNLOCK_ACTION;
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (lock->mActionCompleted_CB)
        {
            lock->mActionCompleted_CB(actionCompleted);
        }
    }

    return 0;
}

bool LockManager::Lock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, DlOperationError & err)
{
    return setLockState(endpointId, DlLockState::kLocked, pin, err);
}

bool LockManager::Unlock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, DlOperationError & err)
{
    return setLockState(endpointId, DlLockState::kUnlocked, pin, err);
}

bool LockManager::GetUser(uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user) const
{
    // chip::ByteSpan credentialData(mLockCredentials.credentialData, mLockCredentials.credentialDataSize);
    ChipLogProgress(Zcl, "Door Lock App: LockManager::GetUser [endpoint=%d,userIndex=%hu]", mEndpointId, userIndex);

    const auto & userInDb = mLockUser;
    user.userStatus       = userInDb.userStatus;
    if (DlUserStatus::kAvailable == user.userStatus)
    {
        ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d]", mEndpointId);
        return true;
    }

    user.userName       = chip::CharSpan(userInDb.userName.data(), userInDb.userName.size());
    user.credentials    = chip::Span<const DlCredential>(userInDb.credentials.data(), userInDb.credentials.size());
    user.userUniqueId   = userInDb.userUniqueId;
    user.userType       = userInDb.userType;
    user.credentialRule = userInDb.credentialRule;
    user.createdBy      = userInDb.createdBy;
    user.lastModifiedBy = userInDb.lastModifiedBy;

    ChipLogDetail(Zcl,
                  "Found occupied user "
                  "[endpoint=%d,name=\"%.*s\",credentialsCount=%u,uniqueId=%lx,type=%u,credentialRule=%u,"
                  "createdBy=%d,lastModifiedBy=%d]",
                  mEndpointId, static_cast<int>(user.userName.size()), user.userName.data(), user.credentials.size(),
                  user.userUniqueId, to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy,
                  user.lastModifiedBy);

    return true;
}

bool LockManager::SetUser(uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype,
                          DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetUser "
                    "[endpoint=%d,userIndex=%d,creator=%d,modifier=%d,userName=%s,uniqueId=%ld "
                    "userStatus=%u,userType=%u,credentialRule=%u,credentials=%p,totalCredentials=%u]",
                    mEndpointId, userIndex, creator, modifier, userName.data(), uniqueId, to_underlying(userStatus),
                    to_underlying(usertype), to_underlying(credentialRule), credentials, totalCredentials);

    auto & userInStorage = mLockUser;

    if (userName.size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        ChipLogError(Zcl, "Cannot set user - user name is too long [endpoint=%d,index=%d]", mEndpointId, userIndex);
        return false;
    }

    if (totalCredentials > mMaxCredentialsPerUser)
    {
        ChipLogError(Zcl, "Cannot set user - total number of credentials is too big [endpoint=%d,index=%d,totalCredentials=%u]",
                     mEndpointId, userIndex, totalCredentials);
        return false;
    }

    chip::Platform::CopyString(mUserName, userName);
    mUserName[userName.size()]   = 0;
    userInStorage.userName       = chip::CharSpan(mUserName, userName.size());
    userInStorage.userUniqueId   = uniqueId;
    userInStorage.userStatus     = userStatus;
    userInStorage.userType       = usertype;
    userInStorage.credentialRule = credentialRule;
    userInStorage.lastModifiedBy = modifier;
    userInStorage.createdBy      = creator;

    for (size_t i = 0; i < totalCredentials; ++i)
    {
        mCredentials[i]                 = credentials[i];
        mCredentials[i].CredentialType  = 1;
        mCredentials[i].CredentialIndex = i + 1;
    }

    userInStorage.credentials = chip::Span<const DlCredential>(mCredentials.Get(), totalCredentials);

    // Save user information in NVM flash
    CYW30739Config::WriteConfigValueBin(CYW30739Config::kConfigKey_LockUser, reinterpret_cast<const uint8_t *>(&userInStorage),
                                        sizeof(EmberAfPluginDoorLockUserInfo));

    CYW30739Config::WriteConfigValueBin(CYW30739Config::kConfigKey_UserCredentials,
                                        reinterpret_cast<const uint8_t *>(mCredentials.Get()), sizeof(DlCredential));

    CYW30739Config::WriteConfigValueStr(CYW30739Config::kConfigKey_LockUserName, mUserName, sizeof(userName.size()));

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d]", mEndpointId, userIndex);

    return true;
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialType credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential) const
{
    ChipLogProgress(Zcl, "Lock App: LockManager::GetCredential [credentialType=%u]", to_underlying(credentialType));

    const auto & credentialInStorage = mLockCredentials;

    credential.status = credentialInStorage.status;
    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogDetail(Zcl, "Found unoccupied credential ");
        return true;
    }
    credential.credentialType = credentialInStorage.credentialType;
    credential.credentialData = credentialInStorage.credentialData;

    ChipLogDetail(Zcl, "Found occupied credential [type=%u,dataSize=%u]", to_underlying(credential.credentialType),
                  credential.credentialData.size());

    return true;
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, DlCredentialStatus credentialStatus,
                                DlCredentialType credentialType, const chip::ByteSpan & credentialData)
{
    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetCredential "
                    "[credentialStatus=%u,credentialType=%u,credentialDataSize=%u]",
                    to_underlying(credentialStatus), to_underlying(credentialType), credentialData.size());

    auto & credentialInStorage = mLockCredentials;
    if (credentialData.size() > DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE)
    {
        ChipLogError(Zcl,
                     "Cannot get the credential - data size exceeds limit "
                     "[dataSize=%u,maxDataSize=%u]",
                     credentialData.size(), DOOR_LOCK_CREDENTIAL_INFO_MAX_DATA_SIZE);
        return false;
    }
    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;

    memcpy(mCredentialData, credentialData.data(), credentialData.size());
    mCredentialData[credentialData.size()] = 0;

    credentialInStorage.credentialData = chip::ByteSpan{ mCredentialData, credentialData.size() };

    // Save user information in NVM flash
    CYW30739Config::WriteConfigValueBin(CYW30739Config::kConfigKey_Credential,
                                        reinterpret_cast<const uint8_t *>(&credentialInStorage),
                                        sizeof(EmberAfPluginDoorLockCredentialInfo));

    CYW30739Config::WriteConfigValueBin(CYW30739Config::kConfigKey_CredentialData,
                                        reinterpret_cast<const uint8_t *>(&mCredentialData), credentialData.size());

    ChipLogProgress(Zcl, "Successfully set the credential [credentialType=%u]", to_underlying(credentialType));

    return true;
}

const char * LockManager::lockStateToString(DlLockState lockState) const
{
    switch (lockState)
    {
    case DlLockState::kNotFullyLocked:
        return "Not Fully Locked";
    case DlLockState::kLocked:
        return "Locked";
    case DlLockState::kUnlocked:
        return "Unlocked";
    }

    return "Unknown";
}

bool LockManager::setLockState(chip::EndpointId endpointId, DlLockState lockState, const Optional<chip::ByteSpan> & pin,
                               DlOperationError & err)
{
    DlLockState curState = DlLockState::kLocked;
    if (mState == kState_UnlockCompleted)
        curState = DlLockState::kUnlocked;

    if (curState == lockState)
    {
        ChipLogDetail(Zcl, "Door Lock App: door is already locked, set lock state to \"%s\" [endpointId=%d]",
                      lockStateToString(lockState), mEndpointId);
    }

    if (!pin.HasValue())
    {
        ChipLogDetail(Zcl, "Door Lock App: PIN code is not specified, setting door lock state to \"%s\" [endpointId=%d]",
                      lockStateToString(lockState), mEndpointId);
        curState = lockState;

        return true;
    }

    // Check the PIN code
    for (uint8_t i; i < 10; i++)
    {
        if (mLockCredentials.credentialType != DlCredentialType::kPin || mLockCredentials.status == DlCredentialStatus::kAvailable)
        {
            continue;
        }

        if (mLockCredentials.credentialData.data_equal(pin.Value()))
        {
            ChipLogDetail(Zcl,
                          "Lock App: specified PIN code was found in the database, setting lock state to \"%s\" [endpointId=%d]",
                          lockStateToString(lockState), mEndpointId);

            curState = lockState;

            return true;
        }
    }

    ChipLogDetail(Zcl,
                  "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                  "[endpointId=%d]",
                  lockStateToString(lockState), mEndpointId);

    err = DlOperationError::kInvalidCredential;
    return false;
}
