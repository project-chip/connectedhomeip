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
#include "platform/KeyValueStoreManager.h"
#include <FreeRTOS.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <cstring>
#include <lib/support/logging/CHIPLogging.h>

LockManager LockManager::sLock;

TimerHandle_t sLockTimer;

using namespace CC13X4_26X4DoorLock::LockInitParams;
using namespace chip::DeviceLayer::PersistedStorage;

// persisted storage keys
const char LockUser[]         = "a/lu";
const char Credential[]       = "a/c";
const char LockUserName[]     = "a/lun";
const char CredentialData[]   = "a/cd";
const char UserCredentials[]  = "a/uc";
const char WeekDaySchedules[] = "a/wds";
const char YearDaySchedules[] = "a/yds";
const char HolidaySchedules[] = "a/hs";

CHIP_ERROR LockManager::Init(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state, LockParam lockParam)
{

    LockParams = lockParam;

    if (LockParams.numberOfUsers > kMaxUsers)
    {
        PLAT_LOG("Max number of users is greater than %d, the maximum amount of users currently supported on this platform",
                 kMaxUsers);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfCredentialsPerUser > kMaxCredentialsPerUser)
    {
        PLAT_LOG("Max number of credentials per user is greater than %d, the maximum amount of users currently supported on this "
                 "platform",
                 kMaxCredentialsPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfWeekdaySchedulesPerUser > kMaxWeekdaySchedulesPerUser)
    {
        PLAT_LOG("Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
                 kMaxWeekdaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfYeardaySchedulesPerUser > kMaxYeardaySchedulesPerUser)
    {
        PLAT_LOG("Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
                 kMaxYeardaySchedulesPerUser);
        return APP_ERROR_ALLOCATION_FAILED;
    }

    if (LockParams.numberOfHolidaySchedules > kMaxHolidaySchedules)
    {
        PLAT_LOG("Max number of schedules is greater than %d, the maximum amount of schedules currently supported on this platform",
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
        PLAT_LOG("sLockTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

    if (state.Value() == DlLockState::kUnlocked)
    {
        mState = kState_UnlockCompleted;
    }
    else
    {
        mState = kState_LockCompleted;
    }

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
    return (credentialIndex < kMaxCredentialsPerUser);
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

bool LockManager::ReadConfigValues()
{
    size_t outLen;
    KeyValueStoreMgr().Get(LockUser, reinterpret_cast<void *>(&mLockUsers),
                           sizeof(EmberAfPluginDoorLockUserInfo) * MATTER_ARRAY_SIZE(mLockUsers), &outLen);

    KeyValueStoreMgr().Get(Credential, reinterpret_cast<void *>(&mLockCredentials),
                           sizeof(EmberAfPluginDoorLockCredentialInfo) * MATTER_ARRAY_SIZE(mLockCredentials), &outLen);

    KeyValueStoreMgr().Get(LockUserName, reinterpret_cast<void *>(mUserNames), sizeof(mUserNames), &outLen);

    KeyValueStoreMgr().Get(CredentialData, reinterpret_cast<void *>(mCredentialData), sizeof(mCredentialData), &outLen);

    KeyValueStoreMgr().Get(UserCredentials, reinterpret_cast<void *>(mCredentials),
                           sizeof(CredentialStruct) * LockParams.numberOfUsers * LockParams.numberOfCredentialsPerUser, &outLen);

    KeyValueStoreMgr().Get(WeekDaySchedules, reinterpret_cast<void *>(mWeekdaySchedule),
                           sizeof(EmberAfPluginDoorLockWeekDaySchedule) * LockParams.numberOfWeekdaySchedulesPerUser *
                               LockParams.numberOfUsers,
                           &outLen);

    KeyValueStoreMgr().Get(YearDaySchedules, reinterpret_cast<void *>(mYeardaySchedule),
                           sizeof(EmberAfPluginDoorLockYearDaySchedule) * LockParams.numberOfYeardaySchedulesPerUser *
                               LockParams.numberOfUsers,
                           &outLen);

    KeyValueStoreMgr().Get(HolidaySchedules, reinterpret_cast<void *>(&(mHolidaySchedule)),
                           sizeof(EmberAfPluginDoorLockHolidaySchedule) * LockParams.numberOfHolidaySchedules, &outLen);

    return true;
}

void LockManager::SetCallbacks(Callback_fn_initiated aActionInitiated_CB, Callback_fn_completed aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

bool LockManager::InitiateAction(Action_t aAction)
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

        StartTimer(ACTUATOR_MOVEMENT_PERIOD_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;

        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction);
        }
    }

    return action_initiated;
}

void LockManager::StartTimer(uint32_t aTimeoutMs)
{
    if (xTimerIsTimerActive(sLockTimer))
    {
        PLAT_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sLockTimer, pdMS_TO_TICKS(aTimeoutMs), 100) != pdPASS)
    {
        PLAT_LOG("sLockTimer timer start() failed");
    }
}

void LockManager::CancelTimer(void)
{
    if (xTimerStop(sLockTimer, 0) == pdFAIL)
    {
        PLAT_LOG("sLockTimer stop() failed");
    }
}

void LockManager::TimerEventHandler(TimerHandle_t xTimer)
{
    // Get lock obj context from timer id.
    LockManager * lock = static_cast<LockManager *>(pvTimerGetTimerID(xTimer));

    // The timer event handler will be called in the context of the timer task
    // once sLockTimer expires. Post an event to apptask queue with the actual handler
    // so that the event can be handled in the context of the apptask.
    AppEvent event;
    event.Type                  = AppEvent::kEventType_AppEvent;
    event.BoltLockEvent.Context = lock;
    event.Handler               = ActuatorMovementTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void LockManager::ActuatorMovementTimerEventHandler(AppEvent * aEvent)
{
    Action_t actionCompleted = INVALID_ACTION;

    LockManager * lock = static_cast<LockManager *>(aEvent->BoltLockEvent.Context);

    if (lock->mState == kState_LockInitiated)
    {
        lock->mState    = kState_LockCompleted;
        actionCompleted = LOCK_ACTION;
        DoorLockServer::Instance().SetLockState(1, DlLockState::kLocked);
    }
    else if (lock->mState == kState_UnlockInitiated)
    {
        lock->mState    = kState_UnlockCompleted;
        actionCompleted = UNLOCK_ACTION;
        DoorLockServer::Instance().SetLockState(1, DlLockState::kUnlocked);
    }

    if (actionCompleted != INVALID_ACTION)
    {
        if (lock->mActionCompleted_CB)
        {
            lock->mActionCompleted_CB(actionCompleted);
        }
    }
}

bool LockManager::Lock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    return checkPin(endpointId, DlLockState::kLocked, pin, err);
}

bool LockManager::Unlock(chip::EndpointId endpointId, const Optional<chip::ByteSpan> & pin, OperationErrorEnum & err)
{
    return checkPin(endpointId, DlLockState::kUnlocked, pin, err);
}

bool LockManager::GetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    ChipLogProgress(Zcl, "Door Lock App: LockManager::GetUser [endpoint=%d,userIndex=%hu]", endpointId, userIndex);

    const auto & userInDb = mLockUsers[userIndex];

    user.userStatus = userInDb.userStatus;
    if (UserStatusEnum::kAvailable == user.userStatus)
    {
        ChipLogDetail(Zcl, "Found unoccupied user [endpoint=%d]", endpointId);
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

    ChipLogDetail(Zcl,
                  "Found occupied user "
                  "[endpoint=%d,name=\"%.*s\",credentialsCount=%u,uniqueId=%lx,type=%u,credentialRule=%u,"
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
                    "[endpoint=%d,userIndex=%d,creator=%d,modifier=%d,userName=%s,uniqueId=%ld "
                    "userStatus=%u,userType=%u,credentialRule=%u,credentials=%p,totalCredentials=%u]",
                    endpointId, userIndex, creator, modifier, userName.data(), uniqueId, to_underlying(userStatus),
                    to_underlying(usertype), to_underlying(credentialRule), credentials, totalCredentials);

    VerifyOrReturnValue(userIndex > 0, false); // indices are one-indexed

    userIndex--;

    VerifyOrReturnValue(IsValidUserIndex(userIndex), false);

    auto & userInStorage = mLockUsers[userIndex];

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
        mCredentials[userIndex][i]                 = credentials[i];
        mCredentials[userIndex][i].credentialType  = CredentialTypeEnum::kPin;
        mCredentials[userIndex][i].credentialIndex = i + 1;
    }

    userInStorage.credentials = chip::Span<const CredentialStruct>(mCredentials[userIndex], totalCredentials);

    // Save user information in NVM flash
    KeyValueStoreMgr().Put(LockUser, reinterpret_cast<const void *>(&mLockUsers),
                           sizeof(EmberAfPluginDoorLockUserInfo) * LockParams.numberOfUsers);

    KeyValueStoreMgr().Put(UserCredentials, reinterpret_cast<const void *>(mCredentials),
                           sizeof(CredentialStruct) * LockParams.numberOfUsers * LockParams.numberOfCredentialsPerUser);

    KeyValueStoreMgr().Put(LockUserName, reinterpret_cast<const void *>(mUserNames), sizeof(mUserNames));

    ChipLogProgress(Zcl, "Successfully set the user [mEndpointId=%d,index=%d]", endpointId, userIndex);

    return true;
}

bool LockManager::GetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
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

    ChipLogProgress(Zcl, "Lock App: LockManager::GetCredential [credentialType=%u], credentialIndex=%d",
                    to_underlying(credentialType), credentialIndex);

    const auto & credentialInStorage = mLockCredentials[credentialIndex];

    credential.status = credentialInStorage.status;
    ChipLogDetail(Zcl, "CredentialStatus: %d, CredentialIndex: %d ", (int) credential.status, credentialIndex);

    if (DlCredentialStatus::kAvailable == credential.status)
    {
        ChipLogDetail(Zcl, "Found unoccupied credential ");
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

    ChipLogDetail(Zcl, "Found occupied credential [type=%u,dataSize=%u]", to_underlying(credential.credentialType),
                  credential.credentialData.size());

    return true;
}

bool LockManager::SetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                const chip::ByteSpan & credentialData)
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

    ChipLogProgress(Zcl,
                    "Door Lock App: LockManager::SetCredential "
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
    KeyValueStoreMgr().Put(Credential, reinterpret_cast<const void *>(&mLockCredentials),
                           sizeof(EmberAfPluginDoorLockCredentialInfo) * LockParams.numberOfCredentialsPerUser);

    KeyValueStoreMgr().Put(CredentialData, reinterpret_cast<const void *>(&mCredentialData), sizeof(mCredentialData));

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

    const auto & scheduleInStorage = mWeekdaySchedule[userIndex][weekdayIndex];
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

    auto & scheduleInStorage = mWeekdaySchedule[userIndex][weekdayIndex];

    scheduleInStorage.schedule.daysMask    = daysMask;
    scheduleInStorage.schedule.startHour   = startHour;
    scheduleInStorage.schedule.startMinute = startMinute;
    scheduleInStorage.schedule.endHour     = endHour;
    scheduleInStorage.schedule.endMinute   = endMinute;
    scheduleInStorage.status               = status;

    // Save schedule information in NVM flash
    KeyValueStoreMgr().Put(WeekDaySchedules, reinterpret_cast<const void *>(mWeekdaySchedule),
                           sizeof(EmberAfPluginDoorLockWeekDaySchedule) * LockParams.numberOfWeekdaySchedulesPerUser *
                               LockParams.numberOfUsers);

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

    const auto & scheduleInStorage = mYeardaySchedule[userIndex][yearDayIndex];
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

    auto & scheduleInStorage = mYeardaySchedule[userIndex][yearDayIndex];

    scheduleInStorage.schedule.localStartTime = localStartTime;
    scheduleInStorage.schedule.localEndTime   = localEndTime;
    scheduleInStorage.status                  = status;

    // Save schedule information in NVM flash
    KeyValueStoreMgr().Put(YearDaySchedules, reinterpret_cast<const void *>(mYeardaySchedule),
                           sizeof(EmberAfPluginDoorLockYearDaySchedule) * LockParams.numberOfYeardaySchedulesPerUser *
                               LockParams.numberOfUsers);

    return DlStatus::kSuccess;
}

DlStatus LockManager::GetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
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

DlStatus LockManager::SetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
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
    KeyValueStoreMgr().Put(HolidaySchedules, reinterpret_cast<const void *>(&(mHolidaySchedule)),
                           sizeof(EmberAfPluginDoorLockHolidaySchedule) * LockParams.numberOfHolidaySchedules);

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
    case DlLockState::kUnknownEnumValue:
        break;
    default:
        break;
    }

    return "Unknown";
}

bool LockManager::checkPin(chip::EndpointId endpointId, DlLockState lockState, const Optional<chip::ByteSpan> & pin,
                           OperationErrorEnum & err)
{
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

            return true;
        }

        ChipLogError(Zcl, "Door Lock App: PIN code is not specified, but it is required [endpointId=%d]", endpointId);

        err = OperationErrorEnum::kInvalidCredential;
        DoorLockServer::Instance().HandleWrongCodeEntry(endpointId);
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
            ChipLogDetail(Zcl,
                          "Lock App: specified PIN code was found in the database, setting lock state to \"%s\" [endpointId=%d]",
                          lockStateToString(lockState), endpointId);

            return true;
        }
    }

    ChipLogDetail(Zcl,
                  "Door Lock App: specified PIN code was not found in the database, ignoring command to set lock state to \"%s\" "
                  "[endpointId=%d]",
                  lockStateToString(lockState), endpointId);

    err = OperationErrorEnum::kInvalidCredential;
    DoorLockServer::Instance().HandleWrongCodeEntry(endpointId);
    return false;
}
