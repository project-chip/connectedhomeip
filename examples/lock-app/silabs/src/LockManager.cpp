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
#include "AppConfig.h"
#include "AppTask.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <cstring>
#include <lib/support/logging/CHIPLogging.h>

using chip::app::DataModel::MakeNullable;
using namespace ::chip::DeviceLayer::Internal;
using namespace SilabsDoorLock;
using namespace SilabsDoorLock::LockInitParams;

static constexpr uint16_t LockUserInfoSize        = sizeof(LockUserInfo);
static constexpr uint16_t LockCredentialInfoSize  = sizeof(LockCredentialInfo);
static constexpr uint16_t CredentialStructSize    = sizeof(CredentialStruct);
static constexpr uint16_t WeekDayScheduleInfoSize = sizeof(WeekDayScheduleInfo);
static constexpr uint16_t YearDayScheduleInfoSize = sizeof(YearDayScheduleInfo);
static constexpr uint16_t HolidayScheduleInfoSize = sizeof(HolidayScheduleInfo);

namespace {
LockManager sLock;
} // namespace

LockManager & LockMgr()
{
    return sLock;
}

CHIP_ERROR LockManager::Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state, LockParam lockParam,
                             PersistentStorageDelegate * storage)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mStorage = storage;

    LockParams = lockParam;

    if (LockParams.numberOfUsers > kMaxUsers)
    {
        SILABS_LOG("Max number of users is greater than %d, the maximum amount of users currently supported on this platform",
                   kMaxUsers);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfCredentialsPerUser > kMaxCredentialsPerUser)
    {
        SILABS_LOG("Max number of credentials per user is greater than %d, the maximum amount of users currently supported on this "
                   "platform",
                   kMaxCredentialsPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfWeekdaySchedulesPerUser > kMaxWeekdaySchedulesPerUser)
    {
        SILABS_LOG(
            "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxWeekdaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfYeardaySchedulesPerUser > kMaxYeardaySchedulesPerUser)
    {
        SILABS_LOG(
            "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxYeardaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfHolidaySchedules > kMaxHolidaySchedules)
    {
        SILABS_LOG(
            "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxHolidaySchedules);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    // Migrate legacy configuration, if needed
    MigrateConfig(lockParam);

    // Create cmsis os sw timer for lock timer.
    mLockTimer = osTimerNew(TimerEventHandler, // timer callback handler
                            osTimerOnce,       // no timer reload (one-shot timer)
                            (void *) this,     // pass the app task obj context
                            NULL               // No osTimerAttr_t to provide.
    );

    if (mLockTimer == NULL)
    {
        SILABS_LOG("mLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    if (state.Value() == DlLockState::kUnlocked)
        mState = kState_UnlockCompleted;
    else
        mState = kState_LockCompleted;

    return CHIP_NO_ERROR;
}

bool LockManager::IsValidUserIndex(uint16_t userIndex)
{
    return (userIndex < kMaxUsers);
}

bool LockManager::IsValidCredentialIndex(uint16_t credentialIndex, CredentialTypeEnum type)
{
    if (CredentialTypeEnum::kProgrammingPIN == type)
    {
        return (0 == credentialIndex); // 0 is required index for Programming PIN
    }

    // Other credential types are valid at all uint16_t indices, credential limit is based on available storage
    return (credentialIndex < UINT16_MAX);
}

bool LockManager::IsValidCredentialType(CredentialTypeEnum type)
{
    return (to_underlying(type) < kNumCredentialTypes);
}

bool LockManager::IsValidWeekdayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxWeekdaySchedulesPerUser);
}

bool LockManager::IsValidYeardayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxYeardaySchedulesPerUser);
}

bool LockManager::IsValidHolidayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxHolidaySchedules);
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

    // Initiate Turn Lock/Unlock Action only when the previous one is complete.
    if ((mState == kState_LockCompleted || mState == kState_UnlatchCompleted) && (aAction == UNLOCK_ACTION))
    {
        action_initiated = true;
        new_state        = kState_UnlockInitiated;
    }
    else if ((mState == kState_LockCompleted || mState == kState_UnlockCompleted) && (aAction == UNLATCH_ACTION))
    {
        action_initiated = true;
        new_state        = kState_UnlatchInitiated;
    }
    else if (mState == kState_UnlockCompleted && aAction == LOCK_ACTION)
    {
        action_initiated = true;
        new_state        = kState_LockInitiated;
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
    // Starts or restarts the function timer
    if (osTimerStart(mLockTimer, pdMS_TO_TICKS(aTimeoutMs)) != osOK)
    {
        SILABS_LOG("mLockTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void LockManager::CancelTimer(void)
{
    if (osTimerStop(mLockTimer) == osError)
    {
        SILABS_LOG("mLockTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void LockManager::TimerEventHandler(void * timerCbArg)
{
    // The callback argument is the light obj context assigned at timer creation.
    LockManager * lock = static_cast<LockManager *>(timerCbArg);

    // The timer event handler will be called in the context of the timer task
    // once mLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = lock;
    event.Handler            = ActuatorMovementTimerEventHandler;
    AppTask::GetAppTask().PostEvent(&event);
}
void LockManager::UnlockAfterUnlatch()
{
    // write the new lock value
    bool succes = false;
    if (mUnlatchContext.mEndpointId != kInvalidEndpointId)
    {
        Optional<chip::ByteSpan> pin = (mUnlatchContext.mPinLength)
            ? MakeOptional(chip::ByteSpan(mUnlatchContext.mPinBuffer, mUnlatchContext.mPinLength))
            : Optional<chip::ByteSpan>::Missing();
        succes = setLockState(mUnlatchContext.mEndpointId, mUnlatchContext.mFabricIdx, mUnlatchContext.mNodeId,
                              DlLockState::kUnlocked, pin, mUnlatchContext.mErr);
    }

    if (!succes)
    {
        SILABS_LOG("Failed to update the lock state after Unlatch");
    }

    InitiateAction(AppEvent::kEventType_Lock, LockManager::UNLOCK_ACTION);
}

void LockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    LockManager * lock = static_cast<LockManager *>(aEvent->TimerEvent.Context);

    if (lock->mState == kState_LockInitiated)
    {
        lock->mState    = kState_LockCompleted;
        actionCompleted = LOCK_ACTION;
    }
    else if (lock->mState == kState_UnlatchInitiated)
    {
        lock->mState    = kState_UnlatchCompleted;
        actionCompleted = UNLATCH_ACTION;
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
}

bool LockManager::Lock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                       const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kLocked, pin, err);
}

bool LockManager::Unlock(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                         const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    if (DoorLockServer::Instance().SupportsUnbolt(endpointId))
    {
        // TODO: Our current implementation does not support multiple endpoint. This needs to be fixed in the future.
        if (endpointId != mUnlatchContext.mEndpointId)
        {
            // If we get a request to unlock on a different endpoint while the current endpoint is in the middle of an action,
            // we return false for now. This needs to be fixed in the future.
            if (mState != kState_UnlockCompleted && mState != kState_LockCompleted)
            {
                ChipLogError(Zcl, "Cannot unlock while unlatch on another endpoint is in progress on  anotther endpoint");
                return false;
            }
            else
            {
                mUnlatchContext.Update(endpointId, fabricIdx, nodeId, pin, err);
                return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlatched, pin, err);
            }
        }
        else
        {
            return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlatched, pin, err);
        }
    }
    return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlocked, pin, err);
}

bool LockManager::Unbolt(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                         const Nullable<chip::NodeId> & nodeId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    return setLockState(endpointId, fabricIdx, nodeId, DlLockState::kUnlocked, pin, err);
}

bool LockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    CHIP_ERROR error;

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    ChipLogProgress(Zcl, "Door Lock App: LockManager::GetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

    // Get User struct from nvm3
    chip::StorageKeyName userKey = LockUserEndpoint(endpointId, userIndex);

    uint16_t size = LockUserInfoSize;

    LockUserInfo userInStorage;

    error = mStorage->SyncGetKeyValue(userKey.KeyName(), &userInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == LockUserInfoSize, false);

    // If no data is found at user key
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        user.userStatus = UserStatusEnum::kAvailable;

        ChipLogDetail(Zcl, "No user data found");
        return true;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {
        user.userStatus = userInStorage.userStatus;

        if (userInStorage.userStatus == UserStatusEnum::kAvailable)
        {
            ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d]", endpointId);
            return true;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return false;
    }

    VerifyOrReturnValue(userInStorage.currentCredentialCount <= kMaxCredentialsPerUser, false);

    VerifyOrReturnValue(userInStorage.userNameSize <= user.userName.size(), false);

    // Copy username data from storage to the output parameter
    memmove(user.userName.data(), userInStorage.userName, userInStorage.userNameSize);

    // Resize Span to match the actual username size retrieved from storage
    user.userName.reduce_size(userInStorage.userNameSize);

    user.userUniqueId   = userInStorage.userUniqueId;
    user.userType       = userInStorage.userType;
    user.credentialRule = userInStorage.credentialRule;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = userInStorage.createdBy;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = userInStorage.lastModifiedBy;

    // Get credential struct from nvm3
    chip::StorageKeyName credentialKey = LockUserCredentialMap(userIndex);

    uint16_t credentialSize = static_cast<uint16_t>(CredentialStructSize * userInStorage.currentCredentialCount);

    CredentialStruct credentials[kMaxCredentialsPerUser];

    error = mStorage->SyncGetKeyValue(credentialKey.KeyName(), credentials, credentialSize);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(credentialSize == static_cast<uint16_t>(CredentialStructSize * userInStorage.currentCredentialCount),
                        false);

    // If KVS read was successful
    if (error == CHIP_NO_ERROR)
    {

        // Copy credentials attached to user from storage to the output parameter
        memmove(user.credentials.data(), credentials, credentialSize);

        // Resize Span to match the actual size of credentials attached to user
        user.credentials.reduce_size(userInStorage.currentCredentialCount);
    }
    else if (error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "Error reading KVS key");
        return false;
    }

    ChipLogDetail(Zcl,
                  "Found occupied user "
                  "[endpoint=%d,name=\"%s\",credentialsCount=%u,uniqueId=%lx,type=%u,credentialRule=%u,"
                  "createdBy=%d,lastModifiedBy=%d]",
                  endpointId, NullTerminated(user.userName).c_str(), user.credentials.size(), user.userUniqueId,
                  to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy, user.lastModifiedBy);

    return true;
}

bool LockManager::SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                          const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                          CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
{

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

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

    LockUserInfo userInStorage;

    memmove(userInStorage.userName, userName.data(), userName.size());
    userInStorage.userNameSize           = userName.size();
    userInStorage.userUniqueId           = uniqueId;
    userInStorage.userStatus             = userStatus;
    userInStorage.userType               = usertype;
    userInStorage.credentialRule         = credentialRule;
    userInStorage.lastModifiedBy         = modifier;
    userInStorage.createdBy              = creator;
    userInStorage.currentCredentialCount = totalCredentials;

    // Save credential struct in nvm3
    chip::StorageKeyName credentialKey = LockUserCredentialMap(userIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(credentialKey.KeyName(), credentials, CredentialStructSize * totalCredentials) ==
                            CHIP_NO_ERROR,
                        false);

    // Save user in nvm3
    chip::StorageKeyName userKey = LockUserEndpoint(endpointId, userIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(userKey.KeyName(), &userInStorage, LockUserInfoSize) == CHIP_NO_ERROR, false);

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d]", endpointId, userIndex);

    return true;
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                EmberAfPluginDoorLockCredentialInfo & credential)
{
    CHIP_ERROR error;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);

    VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType), false);

    ChipLogProgress(Zcl, "Lock App: LockManager::GetCredential [credentialType=%u], credentialIndex=%d",
                    to_underlying(credentialType), credentialIndex);

    chip::StorageKeyName key = LockCredentialEndpoint(endpointId, credentialType, credentialIndex);

    LockCredentialInfo credentialInStorage;

    uint16_t size = LockCredentialInfoSize;

    error = mStorage->SyncGetKeyValue(key.KeyName(), &credentialInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == LockCredentialInfoSize, false);

    // If no data is found at credential key
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // No credentials found
        credential.status = DlCredentialStatus::kAvailable;

        return true;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {
        credential.status = credentialInStorage.status;
        ChipLogDetail(Zcl, "CredentialStatus: %d, CredentialIndex: %d ", (int) credential.status, credentialIndex);
        if (DlCredentialStatus::kAvailable == credential.status)
        {
            ChipLogDetail(Zcl, "Found unoccupied credential ");
            return true;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading KVS key");
        return false;
    }

    VerifyOrReturnValue(credentialInStorage.credentialDataSize <= credential.credentialData.size(), false);

    // Copy credential data from storage to the output parameter
    memmove(credential.credentialData.data(), credentialInStorage.credentialData, credentialInStorage.credentialDataSize);

    // Resize Span to match the actual size of the credential data retrieved from storage
    credential.credentialData.reduce_size(credentialInStorage.credentialDataSize);

    credential.credentialType = credentialInStorage.credentialType;
    credential.createdBy      = credentialInStorage.createdBy;
    credential.lastModifiedBy = credentialInStorage.lastModifiedBy;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    credential.creationSource     = DlAssetSource::kMatterIM;
    credential.modificationSource = DlAssetSource::kMatterIM;

    ChipLogDetail(Zcl, "Found occupied credential");

    return true;
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                const chip::ByteSpan & credentialData)
{
    CHIP_ERROR error;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    VerifyOrReturnValue(IsValidCredentialType(credentialType), false);

    VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType), false);

    VerifyOrReturnValue(credentialData.size() <= kMaxCredentialSize, false);

    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetCredential "
                    "[credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%d,modifier=%d]",
                    to_underlying(credentialStatus), to_underlying(credentialType), credentialData.size(), creator, modifier);

    LockCredentialInfo credentialInStorage;

    credentialInStorage.status             = credentialStatus;
    credentialInStorage.credentialType     = credentialType;
    credentialInStorage.createdBy          = creator;
    credentialInStorage.lastModifiedBy     = modifier;
    credentialInStorage.credentialDataSize = credentialData.size();

    // Copy credential data to the storage struct
    memmove(credentialInStorage.credentialData, credentialData.data(), credentialInStorage.credentialDataSize);

    chip::StorageKeyName key = LockCredentialEndpoint(endpointId, credentialType, credentialIndex);

    error = mStorage->SyncSetKeyValue(key.KeyName(), &credentialInStorage, LockCredentialInfoSize);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return false;
    }

    ChipLogProgress(Zcl, "Successfully set the credential [credentialType=%u]", to_underlying(credentialType));

    return true;
}

DlStatus LockManager::GetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                         EmberAfPluginDoorLockWeekDaySchedule & schedule)
{
    CHIP_ERROR error;

    WeekDayScheduleInfo weekDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    chip::StorageKeyName scheduleDataKey = LockUserWeekDayScheduleEndpoint(endpointId, userIndex, weekdayIndex);

    uint16_t size = WeekDayScheduleInfoSize; // Create a non-const variable

    error = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &weekDayScheduleInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == WeekDayScheduleInfoSize, DlStatus::kFailure);

    // If no data is found at scheduleDataKey
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "No schedule data found for user");
        return DlStatus::kNotFound;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {
        if (weekDayScheduleInStorage.status == DlScheduleStatus::kAvailable)
        {
            return DlStatus::kNotFound;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return DlStatus::kFailure;
    }

    schedule = weekDayScheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockManager::SetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                         DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                         uint8_t endHour, uint8_t endMinute)
{

    WeekDayScheduleInfo weekDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    weekDayScheduleInStorage.schedule.daysMask    = daysMask;
    weekDayScheduleInStorage.schedule.startHour   = startHour;
    weekDayScheduleInStorage.schedule.startMinute = startMinute;
    weekDayScheduleInStorage.schedule.endHour     = endHour;
    weekDayScheduleInStorage.schedule.endMinute   = endMinute;
    weekDayScheduleInStorage.status               = status;

    // Save schedule data in nvm3
    chip::StorageKeyName scheduleDataKey = LockUserWeekDayScheduleEndpoint(endpointId, userIndex, weekdayIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &weekDayScheduleInStorage, WeekDayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

DlStatus LockManager::GetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                         EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    CHIP_ERROR error;

    YearDayScheduleInfo yearDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    chip::StorageKeyName scheduleDataKey = LockUserYearDayScheduleEndpoint(endpointId, userIndex, yearDayIndex);

    uint16_t size = YearDayScheduleInfoSize;

    error = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &yearDayScheduleInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == YearDayScheduleInfoSize, DlStatus::kFailure);

    // If no data is found at scheduleDataKey
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "No schedule data found for user");
        return DlStatus::kNotFound;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {

        if (yearDayScheduleInStorage.status == DlScheduleStatus::kAvailable)
        {
            return DlStatus::kNotFound;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return DlStatus::kFailure;
    }

    schedule = yearDayScheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockManager::SetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                         DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{

    YearDayScheduleInfo yearDayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    yearDayScheduleInStorage.schedule.localStartTime = localStartTime;
    yearDayScheduleInStorage.schedule.localEndTime   = localEndTime;
    yearDayScheduleInStorage.status                  = status;

    // Save schedule data in nvm3
    chip::StorageKeyName scheduleDataKey = LockUserYearDayScheduleEndpoint(endpointId, userIndex, yearDayIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &yearDayScheduleInStorage, YearDayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

    return DlStatus::kSuccess;
}

DlStatus LockManager::GetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                         EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    CHIP_ERROR error;

    HolidayScheduleInfo holidayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    // Get schedule data from nvm3
    chip::StorageKeyName scheduleDataKey = LockHolidayScheduleEndpoint(endpointId, holidayIndex);

    uint16_t size = HolidayScheduleInfoSize;

    error = mStorage->SyncGetKeyValue(scheduleDataKey.KeyName(), &holidayScheduleInStorage, size);

    // Check size out param matches what we expect to read
    VerifyOrReturnValue(size == HolidayScheduleInfoSize, DlStatus::kFailure);

    // If no data is found at scheduleDataKey
    if (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogError(Zcl, "No schedule data found for user");
        return DlStatus::kNotFound;
    }
    // Else if KVS read was successful
    else if (error == CHIP_NO_ERROR)
    {

        if (holidayScheduleInStorage.status == DlScheduleStatus::kAvailable)
        {
            return DlStatus::kNotFound;
        }
    }
    else
    {
        ChipLogError(Zcl, "Error reading from KVS key");
        return DlStatus::kFailure;
    }

    schedule = holidayScheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus LockManager::SetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                         uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{

    HolidayScheduleInfo holidayScheduleInStorage;

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, DlStatus::kFailure);

    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    holidayScheduleInStorage.schedule.localStartTime = localStartTime;
    holidayScheduleInStorage.schedule.localEndTime   = localEndTime;
    holidayScheduleInStorage.schedule.operatingMode  = operatingMode;
    holidayScheduleInStorage.status                  = status;

    // Save schedule data in nvm3
    chip::StorageKeyName scheduleDataKey = LockHolidayScheduleEndpoint(endpointId, holidayIndex);

    VerifyOrReturnValue(mStorage->SyncSetKeyValue(scheduleDataKey.KeyName(), &holidayScheduleInStorage, HolidayScheduleInfoSize) ==
                            CHIP_NO_ERROR,
                        DlStatus::kFailure);

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

bool LockManager::setLockState(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                               const Nullable<chip::NodeId> & nodeId, DlLockState lockState, const Optional<chip::ByteSpan> & pin,
                               OperationErrorEnum & err)
{

    VerifyOrReturnValue(kInvalidEndpointId != endpointId, false);

    // Assume pin is required until told otherwise
    bool requirePin = true;
    chip::app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(endpointId, &requirePin);

    // If a pin code is not given
    if (!pin.HasValue())
    {
        ChipLogDetail(Zcl, "Door Lock App: PIN code is not specified [endpointId=%d]", endpointId);

        // If a pin code is not required
        if (!requirePin)
        {
            ChipLogDetail(Zcl, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", lockStateToString(lockState),
                          endpointId);

            DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, NullNullable, NullNullable,
                                                    fabricIdx, nodeId);

            return true;
        }

        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", endpointId);

        return false;
    }

    // Check all pin codes associated to all users to see if this pin code exists
    for (uint32_t userIndex = 1; userIndex <= kMaxUsers; userIndex++)
    {
        EmberAfPluginDoorLockUserInfo user;

        if (!GetUser(endpointId, userIndex, user))
        {
            ChipLogError(Zcl, "Unable to get the user - internal error [endpointId=%d,userIndex=%lu]", endpointId, userIndex);
            return false;
        }

        if (user.userStatus == UserStatusEnum::kOccupiedEnabled)
        {
            // Loop through each credential attached to the user
            for (auto & userCredential : user.credentials)
            {
                // If the current credential is a pin type, then check it against pin input. Otherwise ignore
                if (userCredential.credentialType == CredentialTypeEnum::kPin)
                {
                    EmberAfPluginDoorLockCredentialInfo credential;

                    if (!GetCredential(endpointId, userCredential.credentialIndex, userCredential.credentialType, credential))
                    {
                        ChipLogError(Zcl,
                                     "Unable to get credential: app error [endpointId=%d,credentialType=%u,credentialIndex=%d]",
                                     endpointId, to_underlying(userCredential.credentialType), userCredential.credentialIndex);
                        return false;
                    }

                    if (credential.status == DlCredentialStatus::kOccupied)
                    {

                        // See if credential retrieved from storage matches the provided PIN
                        if (credential.credentialData.data_equal(pin.Value()))
                        {
                            ChipLogDetail(Zcl,
                                          "Lock App: specified PIN code was found in the database, setting lock state to "
                                          "\"%s\" [endpointId=%d]",
                                          lockStateToString(lockState), endpointId);

                            LockOpCredentials userCred[] = { { CredentialTypeEnum::kPin, userCredential.credentialIndex } };
                            auto userCredentials         = MakeNullable<List<const LockOpCredentials>>(userCred);

                            DoorLockServer::Instance().SetLockState(endpointId, lockState, OperationSourceEnum::kRemote, userIndex,
                                                                    userCredentials, fabricIdx, nodeId);

                            return true;
                        }
                    }
                }
            }
        }
    }

    ChipLogDetail(Zcl,
                  "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                  "[endpointId=%d]",
                  lockStateToString(lockState), endpointId);

    err = OperationErrorEnum::kInvalidCredential;
    return false;
}
