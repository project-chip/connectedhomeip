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

#include <LockManager.h>

#include <AppConfig.h>
#include <AppTask.h>
#include <LockSettingsStorage.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <cstring>
#include <lib/support/logging/CHIPLogging.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

LockManager LockManager::sLock;

using namespace ::chip::DeviceLayer::Internal;
using namespace TelinkDoorLock::LockInitParams;

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
__attribute__((section(".data"))) EmberAfPluginDoorLockUserInfo mCurrentLockUsers;

__attribute__((section(".data"))) char mCurrentUserNames[DOOR_LOCK_MAX_USER_NAME_SIZE];
__attribute__((section(".data"))) uint8_t mCurrentCredentialData[kMaxCredentialSize];
__attribute__((section(".data"))) CredentialStruct mCurrentCredentials[APP_MAX_CREDENTIAL];
__attribute__((section(".data"))) HolidayScheduleInfo mHolidaySchedule;
__attribute__((section(".data"))) WeekDaysScheduleInfo mWeekdaySchedule;
__attribute__((section(".data"))) YearDayScheduleInfo mYeardaySchedule;
#endif

CHIP_ERROR LockManager::Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state, LockParam lockParam,
                             StateChangeCallback callback)
{
    LockParams           = lockParam;
    mStateChangeCallback = callback;

    if (LockParams.numberOfUsers > APP_MAX_USERS)
    {
        ChipLogError(Zcl,
                     "Max number of users %d is greater than %d, the maximum amount of users currently supported on this platform",
                     LockParams.numberOfUsers, APP_MAX_USERS);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfCredentialsPerUser > APP_MAX_CREDENTIAL)
    {
        ChipLogError(
            Zcl,
            "Max number of credentials per user %d is greater than %d, the maximum amount of users currently supported on this "
            "platform",
            LockParams.numberOfCredentialsPerUser, APP_MAX_CREDENTIAL);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfWeekdaySchedulesPerUser > APP_MAX_WEEKDAY_SCHEDULE_PER_USER)
    {
        ChipLogError(
            Zcl,
            "Max number of schedules %d is greater than %d, the maximum amount of schedules currently supported on this platform",
            LockParams.numberOfWeekdaySchedulesPerUser, APP_MAX_WEEKDAY_SCHEDULE_PER_USER);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfYeardaySchedulesPerUser > APP_MAX_YEARDAY_SCHEDULE_PER_USER)
    {
        ChipLogError(
            Zcl,
            "Max number of schedules %d is greater than %d, the maximum amount of schedules currently supported on this platform",
            LockParams.numberOfYeardaySchedulesPerUser, APP_MAX_YEARDAY_SCHEDULE_PER_USER);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfHolidaySchedules > APP_MAX_HOLYDAY_SCHEDULE_PER_USER)
    {
        ChipLogError(
            Zcl,
            "Max number of schedules %d is greater than %d, the maximum amount of schedules currently supported on this platform",
            LockParams.numberOfHolidaySchedules, APP_MAX_HOLYDAY_SCHEDULE_PER_USER);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    k_timer_init(&mActuatorTimer, &LockManager::ActuatorTimerEventHandler, nullptr);
    k_timer_user_data_set(&mActuatorTimer, this);

    return CHIP_NO_ERROR;
}

/* Action related to mechanical operation. Called from button */
bool LockManager::LockAction(int32_t appSource, Action_t aAction, OperationSource source, chip::EndpointId endpointId)
{
    bool status = false;
    switch (aAction)
    {
    case LOCK_ACTION:
        if (mState != kState_LockCompleted)
        {
            mState = kState_LockInitiated;
            status = DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked, source, NullNullable, NullNullable,
                                                             NullNullable, NullNullable);
            if (status)
            {
                LOG_INF("Lock Action: Lock action initiated successfully. Waiting for actuator");
                k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
            }
            else
            {
                LOG_INF("Lock Action: Lock action failed to initiate. No action performed");
                mState = kState_NotFulyLocked;
            }
            if (mStateChangeCallback)
                mStateChangeCallback(mState);
        }
        else
        {
            status = true;
            LOG_INF("Lock Action: Lock is already locked. No action performed");
        }
        break;
    case UNLOCK_ACTION:
        if (mState != kState_UnlockCompleted)
        {
            if (DoorLockServer::Instance().SupportsUnbolt(kExampleEndpointId))
            {
                status = DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlatched, source, NullNullable,
                                                                 NullNullable, NullNullable, NullNullable);
                if (status)
                {
                    LOG_INF("Unlock Action: Step 1: Unbolt completed");
                    mState = kState_UnlatchInitiated;
                    if (mStateChangeCallback)
                        mStateChangeCallback(mState);
                    status = DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked, source, NullNullable,
                                                                     NullNullable, NullNullable, NullNullable);
                    if (status)
                    {
                        LOG_INF("Unlock Action: Step 2: Unlock completed");
                        mState = kState_UnlockInitiated;
                        if (mStateChangeCallback)
                            mStateChangeCallback(mState);
                        k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
                    }
                    else
                    {
                        LOG_INF("Unlock Action: Step 2: Unlock failed. no action performed");
                        mState = kState_NotFulyLocked;
                    }
                }
                else
                {
                    LOG_INF("Unlock Action: Step 1: Unbolt failed. no action performed");
                    mState = kState_NotFulyLocked;
                }
                if (mStateChangeCallback)
                    mStateChangeCallback(mState);
            }
            else
            {
                status = DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked, source, NullNullable,
                                                                 NullNullable, NullNullable, NullNullable);
                if (status)
                {
                    LOG_INF("Unlock Action: Unlock initiated");
                    mState = kState_UnlockInitiated;
                    k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
                }
                else
                {
                    LOG_INF("Unlock Action: Unlock failed. no action performed");
                    mState = kState_NotFulyLocked;
                }
                if (mStateChangeCallback)
                    mStateChangeCallback(mState);
            }
        }
        else
        {
            status = true;
            LOG_INF("Unlock Action: Lock is already unlocked. no action performed");
        }
        break;
    case UNBOLT_ACTION:
        if (mState != kState_UnlatchCompleted)
        {
            status = DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlatched, source, NullNullable,
                                                             NullNullable, NullNullable, NullNullable);
            if (status)
            {
                LOG_INF("Unbolt Action: Unbolt initiated");
                mState = kState_UnlatchInitiated;
                k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
            }
            else
            {
                LOG_INF("Unbolt Action: Unbolt failed. no action performed");
                mState = kState_NotFulyLocked;
            }
            if (mStateChangeCallback)
                mStateChangeCallback(mState);
        }
        else
        {
            status = true;
            LOG_INF("Unbolt Action: Lock is already in unbolt state. no action performed");
        }
        break;
    default:
        LOG_INF("Unknown lock state. no action performed");
        mState = kState_NotFulyLocked;
        break;
    }
    return status;
}

/* Action related to mechanical operation. Called from ZCL */
bool LockManager::LockAction(int32_t appSource, Action_t aAction, OperationSource source, chip::EndpointId endpointId,
                             OperationErrorEnum & err, const Nullable<chip::FabricIndex> & fabricIdx,
                             const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pinCode)
{
    bool status = false;
    switch (aAction)
    {
    case LOCK_ACTION:
        if (mState != kState_LockCompleted)
        {
            mState = kState_LockInitiated;
            status = setLockState(kExampleEndpointId, DlLockState::kLocked, source, err, fabricIdx, nodeId, pinCode);
            if (status)
            {
                LOG_INF("Lock Action: Lock action initiated successfully. Waiting for actuator");
                k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
            }
            else
            {
                LOG_INF("Lock Action: Lock action failed to initiate. No action performed");
                mState = kState_NotFulyLocked;
            }
            if (mStateChangeCallback)
                mStateChangeCallback(mState);
        }
        else
        {
            status = true;
            LOG_INF("Lock Action: Lock is already locked. No action performed");
        }
        break;
    case UNLOCK_ACTION:
        if (mState != kState_UnlockCompleted)
        {
            if (DoorLockServer::Instance().SupportsUnbolt(kExampleEndpointId))
            {
                status = setLockState(kExampleEndpointId, DlLockState::kUnlatched, source, err, fabricIdx, nodeId, pinCode);
                if (status)
                {
                    LOG_INF("Unlock Action: Step 1: Unbolt completed");
                    mState = kState_UnlatchInitiated;
                    if (mStateChangeCallback)
                        mStateChangeCallback(mState);
                    status = setLockState(kExampleEndpointId, DlLockState::kUnlocked, source, err, fabricIdx, nodeId, pinCode);
                    if (status)
                    {
                        LOG_INF("Unlock Action: Step 2: Unlock completed");
                        mState = kState_UnlockInitiated;
                        if (mStateChangeCallback)
                            mStateChangeCallback(mState);
                        k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
                    }
                    else
                    {
                        LOG_INF("Unlock Action: Step 2: Unlock failed. no action performed");
                        mState = kState_NotFulyLocked;
                    }
                }
                else
                {
                    LOG_INF("Unlock Action: Step 1: Unbolt failed. no action performed");
                    mState = kState_NotFulyLocked;
                }
                if (mStateChangeCallback)
                    mStateChangeCallback(mState);
            }
            else
            {
                status = setLockState(kExampleEndpointId, DlLockState::kUnlocked, source, err, fabricIdx, nodeId, pinCode);
                if (status)
                {
                    LOG_INF("Unlock Action: Unlock initiated");
                    mState = kState_UnlockInitiated;
                    k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
                }
                else
                {
                    LOG_INF("Unlock Action: Unlock failed. no action performed");
                    mState = kState_NotFulyLocked;
                }
                if (mStateChangeCallback)
                    mStateChangeCallback(mState);
            }
        }
        else
        {
            status = true;
            LOG_INF("Unlock Action: Lock is already unlocked. no action performed");
        }
        break;
    case UNBOLT_ACTION:
        if (mState != kState_UnlatchCompleted)
        {
            status = setLockState(kExampleEndpointId, DlLockState::kUnlatched, source, err, fabricIdx, nodeId, pinCode);
            if (status)
            {
                LOG_INF("Unbolt Action: Unbolt initiated");
                mState = kState_UnlatchInitiated;
                k_timer_start(&mActuatorTimer, K_MSEC(LOCK_MANAGER_ACTUATOR_MOVEMENT_TIME_MS), K_NO_WAIT);
            }
            else
            {
                LOG_INF("Unbolt Action: Unbolt failed. no action performed");
                mState = kState_NotFulyLocked;
            }
            if (mStateChangeCallback)
                mStateChangeCallback(mState);
        }
        else
        {
            status = true;
            LOG_INF("Unbolt Action: Lock is already in unbolt state. no action performed");
        }
        break;
    default:
        LOG_INF("Unknown lock state. no action performed");
        mState = kState_NotFulyLocked;
        break;
    }
    return status;
}

void LockManager::ActuatorTimerEventHandler(k_timer * timer)
{
    // The timer event handler is called in the context of the system clock ISR.
    // Post an event to the application task queue to process the event in the
    // context of the application thread.

    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = static_cast<LockManager *>(k_timer_user_data_get(timer));
    event.Handler            = (EventHandler) LockManager::ActuatorAppEventHandler;
    GetAppTask().PostEvent(&event);
}

void LockManager::ActuatorAppEventHandler(const AppEvent & event)
{
    LockManager * lock = static_cast<LockManager *>(event.TimerEvent.Context);

    if (!lock)
    {
        return;
    }

    switch (lock->mState)
    {
    case kState_LockInitiated:
        LOG_INF("Lock action completed");
        lock->mState = kState_LockCompleted;
        if (lock->mStateChangeCallback)
            lock->mStateChangeCallback(lock->mState);
        break;
    case kState_UnlockInitiated:
        LOG_INF("Unlock action completed");
        lock->mState = kState_UnlockCompleted;
        if (lock->mStateChangeCallback)
            lock->mStateChangeCallback(lock->mState);
        break;
    case kState_UnlatchInitiated:
        LOG_INF("Unbolt action completed");
        lock->mState = kState_UnlatchCompleted;
        if (lock->mStateChangeCallback)
            lock->mStateChangeCallback(lock->mState);
        break;

    default:
        LOG_INF("Unexpected action occures");
        break;
    }
}

bool LockManager::IsValidUserIndex(uint16_t userIndex)
{
    return (userIndex < APP_MAX_USERS);
}

bool LockManager::IsValidCredentialIndex(uint16_t credentialIndex, CredentialTypeEnum type)
{
    if (CredentialTypeEnum::kProgrammingPIN == type)
    {
        return (0 == credentialIndex); // 0 is required index for Programming PIN
    }
    return (credentialIndex < APP_MAX_CREDENTIAL);
}

bool LockManager::IsValidCredentialType(CredentialTypeEnum type)
{
    return (to_underlying(type) < kNumCredentialTypes);
}

bool LockManager::IsValidWeekdayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < APP_MAX_WEEKDAY_SCHEDULE_PER_USER);
}

bool LockManager::IsValidYeardayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < APP_MAX_YEARDAY_SCHEDULE_PER_USER);
}

bool LockManager::IsValidHolidayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < APP_MAX_HOLYDAY_SCHEDULE_PER_USER);
}

bool LockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    ChipLogProgress(Zcl, "Door Lock App: LockManager::GetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    auto & userInDb             = mCurrentLockUsers;
    auto & credentialsInStorage = mCurrentCredentials;
    EmberAfPluginDoorLockUserInfo lockUser;
    char userNames[DOOR_LOCK_MAX_USER_NAME_SIZE];

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_LockUser[userIndex], reinterpret_cast<uint8_t *>(&lockUser),
                                     sizeof(EmberAfPluginDoorLockUserInfo), outLen);
    userInDb = lockUser;

    CredentialStruct credentialsUsers[APP_MAX_CREDENTIAL];
    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_UserCredentials[userIndex],
                                     reinterpret_cast<uint8_t *>(credentialsUsers),
                                     sizeof(CredentialStruct) * LockParams.numberOfCredentialsPerUser, outLen);
    for (size_t i = 0; i < userInDb.credentials.size(); ++i)
    {
        credentialsInStorage[i] = credentialsUsers[i];
    }
    userInDb.credentials = chip::Span<const CredentialStruct>(credentialsInStorage, userInDb.credentials.size());

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_LockUserName[userIndex],
                                     reinterpret_cast<uint8_t *>(userNames), sizeof(userNames), outLen);

    chip::Platform::CopyString(mCurrentUserNames, userNames);
    userInDb.userName = chip::CharSpan(mCurrentUserNames, userInDb.userName.size());

#else
    const auto & userInDb             = mLockUsers[userIndex];
    const auto & credentialsInStorage = mCredentials[userIndex];
#endif

    user.userStatus = userInDb.userStatus;
    if (UserStatusEnum::kAvailable == user.userStatus)
    {
        ChipLogProgress(Zcl, "Found unoccupied user [endpoint=%d]", endpointId);
        return true;
    }

    user.userName       = chip::CharSpan(userInDb.userName.data(), userInDb.userName.size());
    user.credentials    = chip::Span<const CredentialStruct>(credentialsInStorage, userInDb.credentials.size());
    user.userUniqueId   = userInDb.userUniqueId;
    user.userType       = userInDb.userType;
    user.credentialRule = userInDb.credentialRule;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = userInDb.createdBy;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = userInDb.lastModifiedBy;

    ChipLogProgress(Zcl,
                    "Found occupied user "
                    "[endpoint=%d,name=\"%.*s\",credentialsCount=%u,uniqueId=%x,type=%u,credentialRule=%u,"
                    "createdBy=%d,lastModifiedBy=%d]",
                    endpointId, static_cast<int>(user.userName.size()), user.userName.data(), user.credentials.size(),
                    user.userUniqueId, to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy,
                    user.lastModifiedBy);
    return true;
}

bool LockManager::SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                          CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{
    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetUser "
                    "[endpoint=%d,userIndex=%d,creator=%d,modifier=%d,userName=%s,uniqueId=%u "
                    "userStatus=%u,userType=%u,credentialRule=%u,credentials=%p,totalCredentials=%u]",
                    endpointId, userIndex, creator, modifier, userName.data(), uniqueId, to_underlying(userStatus),
                    to_underlying(usertype), to_underlying(credentialRule), credentials, totalCredentials);

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;
    EmberAfPluginDoorLockUserInfo lockUser;
    CredentialStruct credentialsUsers[APP_MAX_CREDENTIAL];
    char userNames[DOOR_LOCK_MAX_USER_NAME_SIZE];

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_LockUser[userIndex], reinterpret_cast<uint8_t *>(&lockUser),
                                     sizeof(EmberAfPluginDoorLockUserInfo), outLen);

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_UserCredentials[userIndex],
                                     reinterpret_cast<uint8_t *>(credentialsUsers),
                                     sizeof(CredentialStruct) * LockParams.numberOfCredentialsPerUser, outLen);

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_LockUserName[userIndex],
                                     reinterpret_cast<uint8_t *>(userNames), sizeof(userNames), outLen);

    auto & userInStorage        = lockUser;
    auto & credentialsInStorage = credentialsUsers;
    auto & userNamesInStorage   = userNames;
#else
    auto & userInStorage              = mLockUsers[userIndex];
    auto & credentialsInStorage       = mCredentials[userIndex];
    auto & userNamesInStorage         = mUserNames[userIndex];
#endif

    if (userName.size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        ChipLogError(Zcl, "Cannot set user - user name is too long [endpoint=%d,index=%d]", endpointId, userIndex);
        return false;
    }

    if (totalCredentials > LockParams.numberOfCredentialsPerUser)
    {
        ChipLogError(Zcl, "Cannot set user - total number of credentials is too big [endpoint=%d,index=%d,totalCredentials=%u]",
                     endpointId, userIndex, totalCredentials);
        return false;
    }

    chip::Platform::CopyString(userNamesInStorage, userName);
    userInStorage.userName       = chip::CharSpan(userNamesInStorage, userName.size());
    userInStorage.userUniqueId   = uniqueId;
    userInStorage.userStatus     = userStatus;
    userInStorage.userType       = usertype;
    userInStorage.credentialRule = credentialRule;
    userInStorage.lastModifiedBy = modifier;
    userInStorage.createdBy      = creator;

    for (size_t i = 0; i < totalCredentials; ++i)
    {
        credentialsInStorage[i] = credentials[i];
    }

    userInStorage.credentials = chip::Span<const CredentialStruct>(credentialsInStorage, totalCredentials);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    CHIP_ERROR err =
        ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_LockUser[userIndex],
                                          reinterpret_cast<const uint8_t *>(&lockUser), sizeof(EmberAfPluginDoorLockUserInfo));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to write kConfigKey_LockUser to NVM\n");
        return false;
    }

    err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_UserCredentials[userIndex],
                                            reinterpret_cast<const uint8_t *>(credentialsUsers),
                                            sizeof(CredentialStruct) * LockParams.numberOfCredentialsPerUser);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to write kConfigKey_UserCredentials to NVM\n");
        return false;
    }

    err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_LockUserName[userIndex],
                                            reinterpret_cast<const uint8_t *>(userNames), sizeof(userNames));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to write kConfigKey_LockUserName to NVM\n");
        return false;
    }
#endif

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d]", endpointId, userIndex);

    return true;
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential)
{
    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);

    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType),
                            false); // programming pin index is only index allowed to contain 0
    }
    else
    {
        VerifyOrReturnValue(IsValidCredentialIndex(--credentialIndex, credentialType), false); // otherwise, indices are one-indexed
    }

    ChipLogProgress(Zcl, "Lock App: LockManager::GetCredential [credentialType=%u], credentialIndex=%d",
                    to_underlying(credentialType), credentialIndex);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;
    EmberAfPluginDoorLockCredentialInfo lockCredentials;
    uint8_t lockCredentialsData[kMaxCredentialSize] = { 0 };

    auto & credentialInStorage = lockCredentials;

    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        ChipLogError(Zcl, "CredentialTypeEnum::kProgrammingPIN must not be specified as a new credential\n");
        break;
    case CredentialTypeEnum::kPin:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialPin[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataPin[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kFace:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFace[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFace[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kFingerprint:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFingerprint[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFingerprint[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kFingerVein:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFingervein[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFingerVein[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kRfid:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialRfid[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataRfid[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    default:
        ChipLogError(Zcl,
                     "Lock App: LockManager::GetCredential [credentialType=%u], credentialIndex=%d : Credential Type not specified",
                     to_underlying(credentialType), credentialIndex);
        break;
    }

    memcpy(mCurrentCredentialData, lockCredentialsData, credentialInStorage.credentialData.size());
    credentialInStorage.credentialData = chip::ByteSpan{ mCurrentCredentialData, credentialInStorage.credentialData.size() };
#else
    const auto & credentialInStorage  = mLockCredentials[to_underlying(credentialType)][credentialIndex];
#endif

    credential.status = credentialInStorage.status;
    ChipLogProgress(Zcl, "CredentialStatus: %d, CredentialIndex: %d ", (int) credential.status, credentialIndex);

    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogProgress(Zcl, "Found unoccupied credential ");
        return true;
    }
    credential.credentialType = credentialInStorage.credentialType;
    credential.credentialData = credentialInStorage.credentialData;
    credential.createdBy      = credentialInStorage.createdBy;
    credential.lastModifiedBy = credentialInStorage.lastModifiedBy;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.modificationSource = DlAssetSource::kMatterIM;

    ChipLogProgress(Zcl, "Found occupied credential [type=%u,dataSize=%u]", to_underlying(credential.credentialType),
                    credential.credentialData.size());

    return true;
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                const chip::ByteSpan & credentialData)
{

    // TODO: It may be a need to perform a check, if the credential type already exists.
    // The credential setter doesn't contain the user index, so the amount of 10 credentials per user can be satisfied only in case,
    // if each credential type exists only once per one user. Otherwise, it can be the situation, when one user creates 10 pin
    // codes, so the next one user can't create any pincode, but can create the other types of credentials.

    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);

    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType),
                            false); // programming pin index is only index allowed to contain 0
    }
    else
    {
        VerifyOrReturnValue(IsValidCredentialIndex(--credentialIndex, credentialType), false); // otherwise, indices are one-indexed
    }

    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetCredential "
                    "[credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%d,modifier=%d]",
                    to_underlying(credentialStatus), to_underlying(credentialType), credentialData.size(), creator, modifier);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;
    EmberAfPluginDoorLockCredentialInfo lockCredentials;
    uint8_t lockCredentialsData[kMaxCredentialSize] = { 0 };

    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        ChipLogError(Zcl, "CredentialTypeEnum::kProgrammingPIN must not be specified as a new credential\n");
        break;
    case CredentialTypeEnum::kPin:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialPin[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataPin[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kFace:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFace[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFace[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kFingerprint:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFingerprint[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFingerprint[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kFingerVein:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFingervein[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFingerVein[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    case CredentialTypeEnum::kRfid:
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialRfid[credentialIndex],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataRfid[credentialIndex],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);
        break;
    default:
        ChipLogError(Zcl,
                     "Lock App: LockManager::GetCredential [credentialType=%u], credentialIndex=%d : Credential Type not specified",
                     to_underlying(credentialType), credentialIndex);
        break;
    }

    auto & credentialInStorage     = lockCredentials;
    auto & credentialDataInStorage = lockCredentialsData;
#else
    auto & credentialInStorage        = mLockCredentials[to_underlying(credentialType)][credentialIndex];
    auto & credentialDataInStorage    = mCredentialData[to_underlying(credentialType)][credentialIndex];
#endif

    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    credentialInStorage.createdBy      = creator;
    credentialInStorage.lastModifiedBy = modifier;

    memcpy(credentialDataInStorage, credentialData.data(), credentialData.size());
    credentialInStorage.credentialData = chip::ByteSpan{ credentialDataInStorage, credentialData.size() };

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (credentialType)
    {
    case CredentialTypeEnum::kProgrammingPIN:
        ChipLogError(Zcl, "CredentialTypeEnum::kProgrammingPIN must not be specified as a new credential\n");
        break;
    case CredentialTypeEnum::kPin:
        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialPin[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(&lockCredentials),
                                                sizeof(EmberAfPluginDoorLockCredentialInfo));
        if (err != CHIP_NO_ERROR)
            break;

        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataPin[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(lockCredentialsData), kMaxCredentialSize);
        break;
    case CredentialTypeEnum::kFace:
        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFace[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(&lockCredentials),
                                                sizeof(EmberAfPluginDoorLockCredentialInfo));
        if (err != CHIP_NO_ERROR)
            break;

        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFace[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(lockCredentialsData), kMaxCredentialSize);
        break;
    case CredentialTypeEnum::kFingerprint:
        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFingerprint[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(&lockCredentials),
                                                sizeof(EmberAfPluginDoorLockCredentialInfo));
        if (err != CHIP_NO_ERROR)
            break;

        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFingerprint[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(lockCredentialsData), kMaxCredentialSize);
        break;
    case CredentialTypeEnum::kFingerVein:
        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialFingervein[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(&lockCredentials),
                                                sizeof(EmberAfPluginDoorLockCredentialInfo));
        if (err != CHIP_NO_ERROR)
            break;

        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataFingerVein[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(lockCredentialsData), kMaxCredentialSize);
        break;
    case CredentialTypeEnum::kRfid:
        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialRfid[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(&lockCredentials),
                                                sizeof(EmberAfPluginDoorLockCredentialInfo));
        if (err != CHIP_NO_ERROR)
            break;

        err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataRfid[credentialIndex],
                                                reinterpret_cast<const uint8_t *>(lockCredentialsData), kMaxCredentialSize);
        break;
    default:
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(
            Zcl,
            "Failed to write kConfigKey_CredentialData. User data will be resetted during reboot. Not enough storage space \n");
        return false;
    }
#endif

    ChipLogProgress(Zcl, "Successfully set the credential [credentialType=%u]", to_underlying(credentialType));

    return true;
}

DlStatus LockManager::GetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                         EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    CHIP_ERROR err =
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_WeekDaySchedules[userIndex][weekdayIndex],
                                         reinterpret_cast<uint8_t *>(&mWeekdaySchedule), sizeof(WeekDaysScheduleInfo), outLen);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "GetWeekdaySchedule: No schedule found for user: %d at index %d\n", userIndex, weekdayIndex);
        return DlStatus::kNotFound;
    }

    const auto & scheduleInStorage = mWeekdaySchedule;
#else
    const auto & scheduleInStorage    = mWeekdaySchedule[userIndex][weekdayIndex];
#endif

    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockManager::SetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                         DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                         uint8_t endHour, uint8_t endMinute)
{
    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_WeekDaySchedules[userIndex][weekdayIndex],
                                     reinterpret_cast<uint8_t *>(&mWeekdaySchedule), sizeof(WeekDaysScheduleInfo), outLen);

    auto & scheduleInStorage = mWeekdaySchedule;
#else
    auto & scheduleInStorage          = mWeekdaySchedule[userIndex][weekdayIndex];
#endif

    scheduleInStorage.schedule.daysMask    = daysMask;
    scheduleInStorage.schedule.startHour   = startHour;
    scheduleInStorage.schedule.startMinute = startMinute;
    scheduleInStorage.schedule.endHour     = endHour;
    scheduleInStorage.schedule.endMinute   = endMinute;
    scheduleInStorage.status               = status;

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    CHIP_ERROR err =
        ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_WeekDaySchedules[userIndex][weekdayIndex],
                                          reinterpret_cast<uint8_t *>(&mWeekdaySchedule), sizeof(WeekDaysScheduleInfo));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to write kConfigKey_WeekDaySchedules to NVM\n");
        return DlStatus::kFailure;
    }
#endif

    return DlStatus::kSuccess;
}

DlStatus LockManager::GetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                         EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    CHIP_ERROR err =
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_YearDaySchedules[userIndex][yearDayIndex],
                                         reinterpret_cast<uint8_t *>(&mYeardaySchedule), sizeof(YearDayScheduleInfo), outLen);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "GetYeardaySchedule: No schedule found for user: %d at index %d\n", userIndex, yearDayIndex);
        return DlStatus::kNotFound;
    }

    const auto & scheduleInStorage = mYeardaySchedule;
#else
    const auto & scheduleInStorage    = mYeardaySchedule[userIndex][yearDayIndex];
#endif

    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockManager::SetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                         DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_YearDaySchedules[userIndex][yearDayIndex],
                                     reinterpret_cast<uint8_t *>(&mYeardaySchedule), sizeof(YearDayScheduleInfo), outLen);

    auto & scheduleInStorage = mYeardaySchedule;
#else
    auto & scheduleInStorage          = mYeardaySchedule[userIndex][yearDayIndex];
#endif

    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.status                  = status;

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    CHIP_ERROR err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_YearDaySchedules[userIndex][yearDayIndex],
                                                       reinterpret_cast<uint8_t *>(&mYeardaySchedule), sizeof(YearDayScheduleInfo));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to write kConfigKey_YearDaySchedules to NVM\n");
        return DlStatus::kFailure;
    }
#endif

    return DlStatus::kSuccess;
}

DlStatus LockManager::GetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                         EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    CHIP_ERROR err =
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_HolidaySchedules[holidayIndex],
                                         reinterpret_cast<uint8_t *>(&mHolidaySchedule), sizeof(HolidayScheduleInfo), outLen);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(Zcl, "GetHolidaySchedule: No schedule found at index %d\n", holidayIndex);
        return DlStatus::kNotFound;
    }

    const auto & scheduleInStorage = mHolidaySchedule;
#else
    const auto & scheduleInStorage    = mHolidaySchedule[holidayIndex];
#endif

    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockManager::SetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                         uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;

    ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_HolidaySchedules[holidayIndex],
                                     reinterpret_cast<uint8_t *>(&mHolidaySchedule), sizeof(HolidayScheduleInfo), outLen);

    auto & scheduleInStorage = mHolidaySchedule;
#else
    auto & scheduleInStorage          = mHolidaySchedule[holidayIndex];
#endif

    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.schedule.operatingMode  = operatingMode;
    scheduleInStorage.status                  = status;

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    CHIP_ERROR err = ZephyrConfig::WriteConfigValueBin(LockSettingsStorage::kConfigKey_HolidaySchedules[holidayIndex],
                                                       reinterpret_cast<uint8_t *>(&mHolidaySchedule), sizeof(HolidayScheduleInfo));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to write kConfigKey_HolidaySchedules to NVM\n");
        return DlStatus::kFailure;
    }
#endif

    return DlStatus::kSuccess;
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
    case DlLockState::kUnlatched:
        return "Unlatched";
    case DlLockState::kUnknownEnumValue:
        break;
    }

    return "Unknown";
}

bool LockManager::setLockState(chip::EndpointId endpointId, DlLockState lockState, OperationSource source, OperationErrorEnum & err,
                               const Nullable<chip::FabricIndex> & fabricIdx, const Nullable<chip::NodeId> & nodeId,
                               const Optional<chip::ByteSpan> & pin)
{
    // Assume pin is required until told otherwise
    bool requirePin          = true;
    uint16_t userIndex       = 0;
    uint16_t credentialIndex = 0;
    chip::app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(endpointId, &requirePin);

    // If a pin code is not given
    if (!pin.HasValue())
    {
        ChipLogProgress(Zcl, "Door Lock App: PIN code is not specified [endpointId=%d]", endpointId);

        // If a pin code is not required
        if (!requirePin)
        {
            ChipLogProgress(Zcl, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", lockStateToString(lockState),
                            endpointId);

            DoorLockServer::Instance().SetLockState(endpointId, lockState, source, NullNullable, NullNullable, fabricIdx, nodeId);

            return true;
        }

        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", endpointId);
        err = OperationErrorEnum::kRestricted;
        return false;
    }

#if LOCK_MANAGER_CONFIG_USE_NVM_CREDENTIAL_STORAGE
    size_t outLen;
    EmberAfPluginDoorLockCredentialInfo lockCredentials;
    uint8_t lockCredentialsData[kMaxCredentialSize];

    for (uint8_t credentialDataIdx = 0; credentialDataIdx < APP_MAX_CREDENTIAL; credentialDataIdx++)
    {
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialPin[credentialDataIdx],
                                         reinterpret_cast<uint8_t *>(&lockCredentials), sizeof(EmberAfPluginDoorLockCredentialInfo),
                                         outLen);
        ZephyrConfig::ReadConfigValueBin(LockSettingsStorage::kConfigKey_CredentialDataPin[credentialDataIdx],
                                         reinterpret_cast<uint8_t *>(lockCredentialsData), kMaxCredentialSize, outLen);

        auto & currentCredential         = lockCredentials;
        auto & credentialDataInStorage   = lockCredentialsData;
        currentCredential.credentialData = chip::ByteSpan{ credentialDataInStorage, currentCredential.credentialData.size() };
#else
    for (const auto & currentCredential : mLockCredentials[to_underlying(CredentialTypeEnum::kPin)])
    {
#endif
        if (currentCredential.status == DlCredentialStatus::kAvailable)
        {
            continue;
        }

        if (currentCredential.credentialData.data_equal(pin.Value()))
        {

            for (uint16_t i = 1; i <= APP_MAX_USERS; ++i)
            {
                EmberAfPluginDoorLockUserInfo user;
                if (!emberAfPluginDoorLockGetUser(endpointId, i, user))
                {
                    ChipLogError(Zcl, "[setLockState] Unable to get user: app error [userIndex=%d]", i);
                    err = OperationErrorEnum::kInvalidCredential;
                    return false;
                }

                // Go through occupied users only
                if (UserStatusEnum::kAvailable == user.userStatus)
                {
                    continue;
                }

                for (const auto & credential : user.credentials)
                {
                    if (credential.credentialType != CredentialTypeEnum::kPin)
                    {
                        continue;
                    }

                    EmberAfPluginDoorLockCredentialInfo credentialInfo;
                    if (!emberAfPluginDoorLockGetCredential(endpointId, credential.credentialIndex, CredentialTypeEnum::kPin,
                                                            credentialInfo))
                    {
                        ChipLogError(Zcl,
                                     "[setLockState] Unable to get credential: app error "
                                     "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                                     i, credential.credentialIndex, to_underlying(CredentialTypeEnum::kPin));
                        err = OperationErrorEnum::kInvalidCredential;
                        return false;
                    }

                    if (credentialInfo.status != DlCredentialStatus::kOccupied)
                    {
                        ChipLogError(Zcl,
                                     "[setLockState] Users/Credentials database error: credential index attached to user is "
                                     "not occupied "
                                     "[userIndex=%d,credentialIndex=%d,credentialType=%u]",
                                     i, credential.credentialIndex, to_underlying(CredentialTypeEnum::kPin));
                        err = OperationErrorEnum::kInvalidCredential;
                        return false;
                    }

                    if (credentialInfo.credentialData.data_equal(currentCredential.credentialData))
                    {
                        userIndex       = i;
                        credentialIndex = credential.credentialIndex;
                        ChipLogProgress(
                            Zcl,
                            "Lock App: specified PIN code was found in the database, setting lock state to \"%s\" [endpointId=%d]",
                            lockStateToString(lockState), endpointId);

                        LockOpCredentials userCredential[] = { { CredentialTypeEnum::kPin, credentialIndex } };
                        auto userCredentials = chip::app::DataModel::MakeNullable<List<const LockOpCredentials>>(userCredential);

                        DoorLockServer::Instance().SetLockState(
                            endpointId, lockState, source, chip::app::DataModel::MakeNullable(static_cast<uint16_t>(userIndex)),
                            userCredentials, fabricIdx, nodeId);
                        return true;
                    }
                }
            }
        }
    }

    ChipLogProgress(Zcl,
                    "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                    "[endpointId=%d]",
                    lockStateToString(lockState), endpointId);

    err = OperationErrorEnum::kInvalidCredential;
    return false;
}
