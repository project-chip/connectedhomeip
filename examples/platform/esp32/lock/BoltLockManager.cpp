/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <app-common/zap-generated/attributes/Accessors.h>
#include <cstring>
#include <esp_log.h>
#include <lib/support/logging/CHIPLogging.h>

BoltLockManager BoltLockManager::sLock;

TimerHandle_t sLockTimer;

using namespace ::chip::DeviceLayer::Internal;
using namespace ESP32DoorLock::LockInitParams;

CHIP_ERROR BoltLockManager::Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state,
                                 LockParam lockParam)
{

    LockParams = lockParam;

    if (LockParams.numberOfUsers > kMaxUsers)
    {
        ChipLogDetail(Zcl,
                      "Max number of users is greater than %d, the maximum amount of users currently supported on this platform",
                      kMaxUsers);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfCredentialsPerUser > kMaxCredentialsPerUser)
    {
        ChipLogDetail(Zcl,
                      "Max number of credentials per user is greater than %d, the maximum amount of users currently supported on "
                      "this platform",
                      kMaxCredentialsPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfWeekdaySchedulesPerUser > kMaxWeekdaySchedulesPerUser)
    {
        ChipLogDetail(
            Zcl,
            " Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxWeekdaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfYeardaySchedulesPerUser > kMaxYeardaySchedulesPerUser)
    {
        ChipLogDetail(
            Zcl, "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxYeardaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfHolidaySchedules > kMaxHolidaySchedules)
    {
        ChipLogDetail(
            Zcl, "Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
            kMaxHolidaySchedules);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    // Create FreeRTOS sw timer for lock timer.
    sLockTimer = xTimerCreate("lockTmr",        // Just a text name, not used by the RTOS kernel
                              1,                // == default timer period (mS)
                              false,            // no timer reload (==one-shot)
                              (void *) this,    // init timer id = lock obj context
                              TimerEventHandler // timer callback handler
    );

    if (sLockTimer == NULL)
    {
        ChipLogDetail(Zcl, "sLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    if (state.Value() == DlLockState::kUnlocked)
        mState = kState_UnlockCompleted;
    else
        mState = kState_LockCompleted;

    return CHIP_NO_ERROR;
}

bool BoltLockManager::IsValidUserIndex(uint16_t userIndex)
{
    return (userIndex < kMaxUsers);
}

bool BoltLockManager::IsValidCredentialIndex(uint16_t credentialIndex, CredentialTypeEnum type)
{
    if (CredentialTypeEnum::kProgrammingPIN == type)
    {
        return (0 == credentialIndex); // 0 is required index for Programming PIN
    }
    return (credentialIndex < kMaxCredentialsPerUser);
}

bool BoltLockManager::IsValidWeekdayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxWeekdaySchedulesPerUser);
}

bool BoltLockManager::IsValidYeardayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxYeardaySchedulesPerUser);
}

bool BoltLockManager::IsValidHolidayScheduleIndex(uint8_t scheduleIndex)
{
    return (scheduleIndex < kMaxHolidaySchedules);
}

bool BoltLockManager::ReadConfigValues()
{
    size_t outLen;
    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_LockUser, reinterpret_cast<uint8_t *>(&mLockUsers),
                                    sizeof(EmberAfPluginDoorLockUserInfo) * MATTER_ARRAY_SIZE(mLockUsers), outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_Credential, reinterpret_cast<uint8_t *>(&mLockCredentials),
                                    sizeof(EmberAfPluginDoorLockCredentialInfo) * MATTER_ARRAY_SIZE(mLockCredentials), outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_LockUserName, reinterpret_cast<uint8_t *>(mUserNames),
                                    sizeof(mUserNames), outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_CredentialData, reinterpret_cast<uint8_t *>(mCredentialData),
                                    sizeof(mCredentialData), outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_UserCredentials, reinterpret_cast<uint8_t *>(mCredentials),
                                    sizeof(CredentialStruct) * LockParams.numberOfUsers * LockParams.numberOfCredentialsPerUser,
                                    outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_WeekDaySchedules, reinterpret_cast<uint8_t *>(mWeekdaySchedule),
                                    sizeof(EmberAfPluginDoorLockWeekDaySchedule) * LockParams.numberOfWeekdaySchedulesPerUser *
                                        LockParams.numberOfUsers,
                                    outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_YearDaySchedules, reinterpret_cast<uint8_t *>(mYeardaySchedule),
                                    sizeof(EmberAfPluginDoorLockYearDaySchedule) * LockParams.numberOfYeardaySchedulesPerUser *
                                        LockParams.numberOfUsers,
                                    outLen);

    ESP32Config::ReadConfigValueBin(ESP32Config::kConfigKey_HolidaySchedules, reinterpret_cast<uint8_t *>(&(mHolidaySchedule)),
                                    sizeof(EmberAfPluginDoorLockHolidaySchedule) * LockParams.numberOfHolidaySchedules, outLen);

    return true;
}

void BoltLockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool BoltLockManager::IsActionInProgress()
{
    return (mState == kState_LockInitiated || mState == kState_UnlockInitiated);
}

bool BoltLockManager::IsUnlocked()
{
    return (mState == kState_UnlockCompleted);
}

void BoltLockManager::SetAutoLockDuration(uint32_t aDurationInSecs)
{
    mAutoLockDuration = aDurationInSecs;
}

bool BoltLockManager::NextState()
{
    return (mState == kState_UnlockCompleted);
}

bool BoltLockManager::InitiateAction(int32_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Turn Lock/Unlock Action only when the previous one is complete.
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
        if (mAutoLockTimerArmed && new_state == kState_LockInitiated)
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
        ESP_LOGI(TAG, "app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sLockTimer, (aTimeoutMs / portTICK_PERIOD_MS), 100) != pdPASS)
    {
        ESP_LOGI(TAG, "sLockTimer timer start() failed : %s", ErrorStr(APP_ERROR_START_TIMER_FAILED));
    }
}

void BoltLockManager::CancelTimer(void)
{
    if (xTimerStop(sLockTimer, 0) == pdFAIL)
    {
        ESP_LOGI(TAG, "sLockTimer stop() failed : %s", ErrorStr(APP_ERROR_STOP_TIMER_FAILED));
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
    event.mType                = AppEvent::kEventType_Timer;
    event.mTimerEvent.mContext = lock;
    if (lock->mAutoLockTimerArmed)
    {
        event.mHandler = AutoReLockTimerEventHandler;
    }
    else
    {
        event.mHandler = ActuatorMovementTimerEventHandler;
    }
    GetAppTask().PostEvent(&event);
}

void BoltLockManager::AutoReLockTimerEventHandler(AppEvent * aEvent)
{
    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->mTimerEvent.mContext);
    int32_t actor          = 0;

    // Make sure auto lock timer is still armed.
    if (!lock->mAutoLockTimerArmed)
    {
        return;
    }

    lock->mAutoLockTimerArmed = false;

    ESP_LOGI(TAG, "Auto Re-Lock has been triggered!");

    lock->InitiateAction(actor, LOCK_ACTION);
}

void BoltLockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    BoltLockManager * lock = static_cast<BoltLockManager *>(aEvent->mTimerEvent.mContext);

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

        if (lock->mAutoRelock && actionCompleted == UNLOCK_ACTION)
        {
            // Start the timer for auto relock
            lock->StartTimer(lock->mAutoLockDuration * 1000);

            lock->mAutoLockTimerArmed = true;

            ESP_LOGI(TAG, "Auto Re-lock enabled. Will be triggered in %" PRIu32 " seconds", lock->mAutoLockDuration);
        }
    }
}

bool BoltLockManager::Lock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    return setLockState(endpointId, DlLockState::kLocked, pin, err);
}

bool BoltLockManager::Unlock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    return setLockState(endpointId, DlLockState::kUnlocked, pin, err);
}

bool BoltLockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    ESP_LOGI(TAG, "Door Lock App: BoltLockManager::GetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

    const auto & userInDb = mLockUsers[userIndex];

    user.userStatus = userInDb.userStatus;
    if (UserStatusEnum::kAvailable == user.userStatus)
    {
        ESP_LOGI(TAG, "Found unoccupied user [endpoint=%d]", endpointId);
        return true;
    }

    user.userName       = chip::CharSpan(userInDb.userName.data(), userInDb.userName.size());
    user.credentials    = chip::Span<const CredentialStruct>(mCredentials[userIndex], userInDb.credentials.size());
    user.userUniqueId   = userInDb.userUniqueId;
    user.userType       = userInDb.userType;
    user.credentialRule = userInDb.credentialRule;
    // So far there's no way to actually create the credential outside Matter, so here we always set the creation/modification
    // source to Matter
    user.creationSource     = DlAssetSource::kMatterIM;
    user.createdBy          = userInDb.createdBy;
    user.modificationSource = DlAssetSource::kMatterIM;
    user.lastModifiedBy     = userInDb.lastModifiedBy;

    ESP_LOGI(TAG,
             "Found occupied user [endpoint=%d,name=\"%.*s\",credentialsCount=%u,uniqueId=%" PRIu32
             ",type=%u,credentialRule=%u,createdBy=%d,lastModifiedBy=%d]",
             endpointId, static_cast<int>(user.userName.size()), user.userName.data(), user.credentials.size(), user.userUniqueId,
             to_underlying(user.userType), to_underlying(user.credentialRule), user.createdBy, user.lastModifiedBy);

    return true;
}

bool BoltLockManager::SetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                              chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                              UserStatusEnum userStatus, UserTypeEnum usertype, CredentialRuleEnum credentialRule,
                              const CredentialStruct * credentials, size_t totalCredentials)
{
    ESP_LOGI(TAG,
             "Door Lock App: BoltLockManager::SetUser "
             "[endpoint=%d,userIndex=%d,creator=%d,modifier=%d,userName=%s,uniqueId=%" PRIu32 ""
             "userStatus=%u,userType=%u,credentialRule=%u,credentials=%p,totalCredentials=%u]",
             endpointId, userIndex, creator, modifier, userName.data(), uniqueId, to_underlying(userStatus),
             to_underlying(usertype), to_underlying(credentialRule), credentials, totalCredentials);

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    auto & userInStorage = mLockUsers[userIndex];

    if (userName.size() > DOOR_LOCK_MAX_USER_NAME_SIZE)
    {
        ESP_LOGE(TAG, "Cannot set user - user name is too long [endpoint=%d,index=%d]", endpointId, userIndex);
        return false;
    }

    if (totalCredentials > LockParams.numberOfCredentialsPerUser)
    {
        ESP_LOGE(TAG, "Cannot set user - total number of credentials is too big [endpoint=%d,index=%d,totalCredentials=%u]",
                 endpointId, userIndex, totalCredentials);
        return false;
    }

    chip::Platform::CopyString(mUserNames[userIndex], userName);
    userInStorage.userName       = chip::CharSpan(mUserNames[userIndex], userName.size());
    userInStorage.userUniqueId   = uniqueId;
    userInStorage.userStatus     = userStatus;
    userInStorage.userType       = usertype;
    userInStorage.credentialRule = credentialRule;
    userInStorage.lastModifiedBy = modifier;
    userInStorage.createdBy      = creator;

    for (size_t i = 0; i < totalCredentials; ++i)
    {
        mCredentials[userIndex][i] = credentials[i];
    }
    userInStorage.credentials = chip::Span<const CredentialStruct>(mCredentials[userIndex], totalCredentials);

    // Save user information in NVM flash
    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_LockUser, reinterpret_cast<const uint8_t *>(&mLockUsers),
                                     sizeof(EmberAfPluginDoorLockUserInfo) * LockParams.numberOfUsers);

    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_UserCredentials, reinterpret_cast<const uint8_t *>(mCredentials),
                                     sizeof(CredentialStruct) * LockParams.numberOfUsers * LockParams.numberOfCredentialsPerUser);

    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_LockUserName, reinterpret_cast<const uint8_t *>(mUserNames),
                                     sizeof(mUserNames));

    ESP_LOGI(TAG, "Successfully set the user [mEndpointId=%d,index=%d]", endpointId, userIndex);

    return true;
}

bool BoltLockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                    EmberAfPluginDoorLockCredentialInfo & credential)
{

    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType),
                            false); // programming pin index is only index allowed to contain 0
    }
    else
    {
        VerifyOrReturnValue(IsValidCredentialIndex(--credentialIndex, credentialType), false); // otherwise, indices are one-indexed
    }

    ESP_LOGI(TAG, "Lock App: BoltLockManager::GetCredential [credentialType=%u], credentialIndex=%d", to_underlying(credentialType),
             credentialIndex);

    const auto & credentialInStorage = mLockCredentials[credentialIndex];

    credential.status = credentialInStorage.status;
    ESP_LOGI(TAG, "CredentialStatus: %d, CredentialIndex: %d ", (int) credential.status, credentialIndex);

    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ESP_LOGI(TAG, "Found unoccupied credential ");
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

    ESP_LOGI(TAG, "Found occupied credential [type=%u,dataSize=%u]", to_underlying(credential.credentialType),
             credential.credentialData.size());

    return true;
}

bool BoltLockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                    chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                    CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData)
{

    if (CredentialTypeEnum::kProgrammingPIN == credentialType)
    {
        VerifyOrReturnValue(IsValidCredentialIndex(credentialIndex, credentialType),
                            false); // programming pin index is only index allowed to contain 0
    }
    else
    {
        VerifyOrReturnValue(IsValidCredentialIndex(--credentialIndex, credentialType), false); // otherwise, indices are one-indexed
    }

    ESP_LOGI(TAG,
             "Door Lock App: BoltLockManager::SetCredential "
             "[credentialStatus=%u,credentialType=%u,credentialDataSize=%u,creator=%d,modifier=%d]",
             to_underlying(credentialStatus), to_underlying(credentialType), credentialData.size(), creator, modifier);

    auto & credentialInStorage = mLockCredentials[credentialIndex];

    credentialInStorage.status         = credentialStatus;
    credentialInStorage.credentialType = credentialType;
    credentialInStorage.createdBy      = creator;
    credentialInStorage.lastModifiedBy = modifier;

    memcpy(mCredentialData[credentialIndex], credentialData.data(), credentialData.size());
    credentialInStorage.credentialData = chip::ByteSpan{ mCredentialData[credentialIndex], credentialData.size() };

    // Save credential information in NVM flash
    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_Credential, reinterpret_cast<const uint8_t *>(&mLockCredentials),
                                     sizeof(EmberAfPluginDoorLockCredentialInfo) * LockParams.numberOfCredentialsPerUser);

    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_CredentialData, reinterpret_cast<const uint8_t *>(&mCredentialData),
                                     sizeof(mCredentialData));

    ESP_LOGI(TAG, "Successfully set the credential [credentialType=%u]", to_underlying(credentialType));

    return true;
}

DlStatus BoltLockManager::GetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                             EmberAfPluginDoorLockWeekDaySchedule & schedule)
{

    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    const auto & scheduleInStorage = mWeekdaySchedule[userIndex][weekdayIndex];
    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus BoltLockManager::SetWeekdaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                             DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                             uint8_t endHour, uint8_t endMinute)
{

    VerifyOrReturnValue(weekdayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    weekdayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidWeekdayScheduleIndex(weekdayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    auto & scheduleInStorage = mWeekdaySchedule[userIndex][weekdayIndex];

    scheduleInStorage.schedule.daysMask    = daysMask;
    scheduleInStorage.schedule.startHour   = startHour;
    scheduleInStorage.schedule.startMinute = startMinute;
    scheduleInStorage.schedule.endHour     = endHour;
    scheduleInStorage.schedule.endMinute   = endMinute;
    scheduleInStorage.status               = status;

    // Save schedule information in NVM flash
    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_WeekDaySchedules, reinterpret_cast<const uint8_t *>(mWeekdaySchedule),
                                     sizeof(EmberAfPluginDoorLockWeekDaySchedule) * LockParams.numberOfWeekdaySchedulesPerUser *
                                         LockParams.numberOfUsers);

    return DlStatus::kSuccess;
}

DlStatus BoltLockManager::GetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                             EmberAfPluginDoorLockYearDaySchedule & schedule)
{
    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    const auto & scheduleInStorage = mYeardaySchedule[userIndex][yearDayIndex];
    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus BoltLockManager::SetYeardaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                             DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime)
{
    VerifyOrReturnValue(yearDayIndex > 0, DlStatus::kFailure); // indices are one-indexed
    VerifyOrReturnValue(userIndex > 0, DlStatus::kFailure);    // indices are one-indexed

    yearDayIndex--;
    userIndex--;

    VerifyOrReturnValue(IsValidYeardayScheduleIndex(yearDayIndex), DlStatus::kFailure);
    VerifyOrReturnValue(IsValidUserIndex(userIndex), DlStatus::kFailure);

    auto & scheduleInStorage = mYeardaySchedule[userIndex][yearDayIndex];

    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.status                  = status;

    // Save schedule information in NVM flash
    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_YearDaySchedules, reinterpret_cast<const uint8_t *>(mYeardaySchedule),
                                     sizeof(EmberAfPluginDoorLockYearDaySchedule) * LockParams.numberOfYeardaySchedulesPerUser *
                                         LockParams.numberOfUsers);

    return DlStatus::kSuccess;
}

DlStatus BoltLockManager::GetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                             EmberAfPluginDoorLockHolidaySchedule & schedule)
{
    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    const auto & scheduleInStorage = mHolidaySchedule[holidayIndex];
    if (DlScheduleStatus::kAvailable == scheduleInStorage.status)
    {
        return DlStatus::kNotFound;
    }

    schedule = scheduleInStorage.schedule;

    return DlStatus::kSuccess;
}

DlStatus BoltLockManager::SetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                             uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode)
{
    VerifyOrReturnValue(holidayIndex > 0, DlStatus::kFailure); // indices are one-indexed

    holidayIndex--;

    VerifyOrReturnValue(IsValidHolidayScheduleIndex(holidayIndex), DlStatus::kFailure);

    auto & scheduleInStorage = mHolidaySchedule[holidayIndex];

    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.schedule.operatingMode  = operatingMode;
    scheduleInStorage.status                  = status;

    // Save schedule information in NVM flash
    ESP32Config::WriteConfigValueBin(ESP32Config::kConfigKey_HolidaySchedules,
                                     reinterpret_cast<const uint8_t *>(&(mHolidaySchedule)),
                                     sizeof(EmberAfPluginDoorLockHolidaySchedule) * LockParams.numberOfHolidaySchedules);

    return DlStatus::kSuccess;
}

const char * BoltLockManager::lockStateToString(DlLockState lockState) const
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

bool BoltLockManager::setLockState(chip::EndpointId endpointId, DlLockState lockState, const Optional<chip::ByteSpan> & pin,
                                   OperationErrorEnum & err)
{

    // Assume pin is required until told otherwise
    bool requirePin = true;
    chip::app::Clusters::DoorLock::Attributes::RequirePINforRemoteOperation::Get(endpointId, &requirePin);

    // If a pin code is not given
    if (!pin.HasValue())
    {
        ESP_LOGI(TAG, "Door Lock App: PIN code is not specified [endpointId=%d]", endpointId);

        // If a pin code is not required
        if (!requirePin)
        {
            ESP_LOGI(TAG, "Door Lock App: setting door lock state to \"%s\" [endpointId=%d]", lockStateToString(lockState),
                     endpointId);

            DoorLockServer::Instance().SetLockState(endpointId, lockState);

            return true;
        }

        ESP_LOGI(TAG, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", endpointId);

        return false;
    }

    // Check the PIN code
    for (uint8_t i = 0; i < kMaxCredentials; i++)
    {
        if (mLockCredentials[i].credentialType != CredentialTypeEnum::kPin ||
            mLockCredentials[i].status == DlCredentialStatus::kAvailable)
        {
            continue;
        }

        if (mLockCredentials[i].credentialData.data_equal(pin.Value()))
        {
            ESP_LOGI(TAG, "Lock App: specified PIN code was found in the database, setting lock state to \"%s\" [endpointId=%d]",
                     lockStateToString(lockState), endpointId);

            DoorLockServer::Instance().SetLockState(endpointId, lockState);

            return true;
        }
    }

    ESP_LOGI(TAG,
             "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
             "[endpointId=%d]",
             lockStateToString(lockState), endpointId);

    err = OperationErrorEnum::kInvalidCredential;
    return false;
}

CHIP_ERROR BoltLockManager::InitLockState()
{

    // Initial lock state
    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ 1 };
    chip::app::Clusters::DoorLock::Attributes::LockState::Get(endpointId, state);

    uint8_t numberOfCredentialsPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfCredentialsSupportedPerUser(endpointId, numberOfCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of credentials supported per user when initializing lock endpoint, defaulting to 5 "
                     "[endpointId=%d]",
                     endpointId);
        numberOfCredentialsPerUser = 5;
    }

    uint16_t numberOfUsers = 0;
    if (!DoorLockServer::Instance().GetNumberOfUserSupported(endpointId, numberOfUsers))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported users when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
                     endpointId);
        numberOfUsers = 10;
    }

    uint8_t numberOfWeekdaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, numberOfWeekdaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported weekday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfWeekdaySchedulesPerUser = 10;
    }

    uint8_t numberOfYeardaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfYearDaySchedulesPerUserSupported(endpointId, numberOfYeardaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported yearday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfYeardaySchedulesPerUser = 10;
    }

    uint8_t numberOfHolidaySchedules = 0;
    if (!DoorLockServer::Instance().GetNumberOfHolidaySchedulesSupported(endpointId, numberOfHolidaySchedules))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported holiday schedules when initializing lock endpoint, defaulting to 10 [endpointId=%d]",
            endpointId);
        numberOfHolidaySchedules = 10;
    }

    CHIP_ERROR err = BoltLockMgr().Init(state,
                                        ParamBuilder()
                                            .SetNumberOfUsers(numberOfUsers)
                                            .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                                            .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                                            .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                                            .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                                            .GetLockParam());
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "BoltLockMgr().Init() failed");
        return err;
    }

    return err;
}
