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

#include <stdbool.h>
#include <stdint.h>

#include <algorithm>
#include <cstring>

#include "AppEvent.h"
#include "BaseApplication.h"
#include "CHIPProjectConfig.h"
#include <FreeRTOS.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <ble/Ble.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TypeTraits.h>
#include <platform/CHIPDeviceLayer.h>
#include <timers.h>

/**
 * @brief Door-lock application task.
 *
 * Customer code overrides AppTask behavior via `AppTaskImpl<>` `*Impl()` hooks;
 * see `AppTaskImpl.h` for the full override contract. This header exposes only
 * the entry points that the AppTask main loop, `BaseApplication`, and the
 * DoorLock cluster plugin layer need.
 */
class AppTask : public BaseApplication
{
public:
    AppTask() = default;

    /** @brief Returns the active app instance (`CustomerAppTask` via CRTP). */
    static AppTask & GetAppTask();

    /** @brief Door Lock resource limits applied in `InitLockDomain`. */
    struct LockParam
    {
        uint16_t numberOfUsers                  = 0;
        uint8_t numberOfCredentialsPerUser      = 0;
        uint8_t numberOfWeekdaySchedulesPerUser = 0;
        uint8_t numberOfYeardaySchedulesPerUser = 0;
        uint8_t numberOfHolidaySchedules        = 0;
    };

    /** @brief Builds a `LockParam`; setters return *this for chaining (e.g. from `InitLockImpl()`). */
    class LockParamBuilder
    {
    public:
        LockParamBuilder & SetNumberOfUsers(uint16_t numberOfUsers)
        {
            mLockParam.numberOfUsers = numberOfUsers;
            return *this;
        }
        LockParamBuilder & SetNumberOfCredentialsPerUser(uint8_t numberOfCredentialsPerUser)
        {
            mLockParam.numberOfCredentialsPerUser = numberOfCredentialsPerUser;
            return *this;
        }
        LockParamBuilder & SetNumberOfWeekdaySchedulesPerUser(uint8_t numberOfWeekdaySchedulesPerUser)
        {
            mLockParam.numberOfWeekdaySchedulesPerUser = numberOfWeekdaySchedulesPerUser;
            return *this;
        }
        LockParamBuilder & SetNumberOfYeardaySchedulesPerUser(uint8_t numberOfYeardaySchedulesPerUser)
        {
            mLockParam.numberOfYeardaySchedulesPerUser = numberOfYeardaySchedulesPerUser;
            return *this;
        }
        LockParamBuilder & SetNumberOfHolidaySchedules(uint8_t numberOfHolidaySchedules)
        {
            mLockParam.numberOfHolidaySchedules = numberOfHolidaySchedules;
            return *this;
        }
        LockParam GetLockParam() { return mLockParam; }

    private:
        LockParam mLockParam;
    };

    /** @brief Actuator motion: lock, unlock, or unlatch (unbolt path). */
    enum class LockAction : uint8_t
    {
        kLock = 0,
        kUnlock,
        kUnlatch,
        kInvalid,
    };

    /**
     * @brief Request routed on the AppTask thread (button, DM command, or auto-relock).
     *
     * Remote paths set `targetClusterState` and attribution fields; `isUnboltUnlatch`
     * selects unlatch then auto-unlock when `SupportsUnbolt` is true.
     */
    struct LockRequest
    {
        chip::EndpointId endpointId                                   = chip::kInvalidEndpointId;
        LockAction action                                             = LockAction::kInvalid;
        chip::app::Clusters::DoorLock::DlLockState targetClusterState = chip::app::Clusters::DoorLock::DlLockState::kLocked;
        chip::app::DataModel::Nullable<chip::FabricIndex> fabricIdx;
        chip::app::DataModel::Nullable<chip::NodeId> nodeId;
        chip::app::DataModel::Nullable<uint16_t> userIndex;
        LockOpCredentials credential{};
        bool hasCredential   = false;
        bool isUnboltUnlatch = false;
        bool isButtonAction  = false;
    };

    /**
     * @brief AppTask task main loop function.
     *
     * @param pvParameter FreeRTOS task parameter.
     */
    static void AppTaskMain(void * pvParameter);

    /** @brief Creates and starts the AppTask FreeRTOS thread. */
    CHIP_ERROR StartAppTask();

    /**
     * @brief Event handler when a button is pressed.
     *
     * @param button    APP_LOCK_SWITCH or APP_FUNCTION_BUTTON.
     * @param btnAction SL_SIMPLE_BUTTON_PRESSED, SL_SIMPLE_BUTTON_RELEASED, or SL_SIMPLE_BUTTON_DISABLED.
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /** @brief Door Lock attribute-change hook (`MatterPostAttributeChangeCallback`). */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    /** @brief Lock-switch handler; builds a button `LockRequest` for `HandleLockRequestOnAppTask`. */
    static void LockButtonActionHandler(AppEvent * aEvent);

    /** @brief One-shot unlatch timer callback; schedules `UnlockAfterUnlatch` on the chip thread. */
    static void UnlatchCallback(void * argument);

    /** @brief AppTask handler for actuator-movement timer completion. */
    static void ActuatorMovementEventHandler(AppEvent * aEvent);

    /** @brief Handler for `kEventType_Lock`; calls `InitiateLockAction`. */
    static void LockActionEventHandler(AppEvent * aEvent);

    /** @brief Handler for `kEventType_LockRequest`; drains chip-thread staged request. */
    static void LockRequestEventHandler(AppEvent * aEvent);

    /** @brief Single router for button and DM requests (queue, remote preamble, actuator). */
    void HandleLockRequestOnAppTask(const LockRequest & request);

    /** @brief Lock command (chip thread): validate PIN, `EnqueueLockRequest`. */
    bool DMDoorLockOnDoorLockCommand(chip::EndpointId endpointId,
                                     const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                     const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                     const chip::Optional<chip::ByteSpan> & pinCode,
                                     chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief Unlock command (chip thread): validate PIN; unlatch if `SupportsUnbolt`. */
    bool DMDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief Unbolt command (chip thread): validate PIN, enqueue unlock `LockRequest`. */
    bool DMDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief Door Lock plugin: read credential from KVS. */
    bool DMDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                 EmberAfPluginDoorLockCredentialInfo & credential);

    /** @brief Door Lock plugin: write credential to KVS. */
    bool DMDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                 chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                 const chip::ByteSpan & credentialData);

    /** @brief Door Lock plugin: read user and credential index list from KVS. */
    bool DMDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);

    /** @brief Door Lock plugin: write user and credential index list to KVS. */
    bool DMDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                           const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                           CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials);

    /** @brief Door Lock plugin: read weekday schedule from KVS. */
    DlStatus DMDoorLockGetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule);

    /** @brief Door Lock plugin: write weekday schedule to KVS. */
    DlStatus DMDoorLockSetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute);

    /** @brief Door Lock plugin: read year-day schedule from KVS. */
    DlStatus DMDoorLockGetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule);

    /** @brief Door Lock plugin: write year-day schedule to KVS. */
    DlStatus DMDoorLockSetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime);

    /** @brief Door Lock plugin: read holiday schedule from KVS. */
    DlStatus DMDoorLockGetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & holidaySchedule);

    /** @brief Door Lock plugin: write holiday schedule to KVS. */
    DlStatus DMDoorLockSetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode);

    /** @brief Auto-relock hook; posts `kEventType_Lock` with `LockAction::kLock`. */
    void DMDoorLockOnAutoRelock(chip::EndpointId endpointId);

    /** @brief PIN lookup across users/credentials; does not change cluster state. */
    bool ValidatePin(chip::EndpointId endpointId, const chip::Optional<chip::ByteSpan> & pin,
                     chip::app::DataModel::Nullable<uint16_t> & outUserIndex, LockOpCredentials & outCred, bool & outHasCred,
                     chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief `SetLockState` with `kRemote` and controller attribution; hold chip stack lock. */
    void PushClusterLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState lockState,
                              const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                              const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                              const chip::app::DataModel::Nullable<uint16_t> & userIndex, const LockOpCredentials * cred,
                              bool hasCred);

    /** @brief Starts an actuator transition; returns false if busy or invalid. */
    bool InitiateLockAction(LockAction aAction, bool fromButton = false);

    /** @brief Chip-thread work after unlatch timer: transitional cluster state, stage unlock, post lock event. */
    static void UnlockAfterUnlatch(intptr_t context);

protected:
    /** @brief Override of `BaseApplication::AppInit()`. */
    CHIP_ERROR AppInit() override;

    /** @brief Lock domain setup: limits, KVS, timers, LED (CRTP `InitLockImpl`). */
    CHIP_ERROR InitLock();

    /** @brief Chip-thread work item: push `LockState` with `kManual` (schedule via `PlatformMgr().ScheduleWork`). */
    static void UpdateClusterState(intptr_t context);

    /** @brief KVS blob: weekday schedule status plus schedule fields. */
    struct WeekDayScheduleInfo
    {
        DlScheduleStatus status;
        EmberAfPluginDoorLockWeekDaySchedule schedule;
    };

    /** @brief KVS blob: year-day schedule status plus schedule fields. */
    struct YearDayScheduleInfo
    {
        DlScheduleStatus status;
        EmberAfPluginDoorLockYearDaySchedule schedule;
    };

    /** @brief KVS blob: holiday schedule status plus schedule fields. */
    struct HolidayScheduleInfo
    {
        DlScheduleStatus status;
        EmberAfPluginDoorLockHolidaySchedule schedule;
    };

    /** @brief KVS blob: user fields; credential index list uses `LockUserCredentialMap`. */
    struct LockUserInfo
    {
        char userName[DOOR_LOCK_MAX_USER_NAME_SIZE];
        uint32_t userNameSize;
        uint32_t userUniqueId;
        UserStatusEnum userStatus;
        UserTypeEnum userType;
        CredentialRuleEnum credentialRule;
        chip::EndpointId endpointId;
        chip::FabricIndex createdBy;
        chip::FabricIndex lastModifiedBy;
        uint16_t currentCredentialCount;
    };

    /** @brief KVS blob: credential status, type, fabric IDs, and payload. */
    struct LockCredentialInfo
    {
        DlCredentialStatus status;
        CredentialTypeEnum credentialType;
        chip::FabricIndex createdBy;
        chip::FabricIndex lastModifiedBy;
        uint8_t credentialData[SilabsDoorLockConfig::ResourceRanges::kMaxCredentialSize];
        uint32_t credentialDataSize;
    };

    static constexpr uint16_t kLockUserInfoSize        = sizeof(LockUserInfo);
    static constexpr uint16_t kLockCredentialInfoSize  = sizeof(LockCredentialInfo);
    static constexpr uint16_t kCredentialStructSize    = sizeof(CredentialStruct);
    static constexpr uint16_t kWeekDayScheduleInfoSize = sizeof(WeekDayScheduleInfo);
    static constexpr uint16_t kYearDayScheduleInfoSize = sizeof(YearDayScheduleInfo);
    static constexpr uint16_t kHolidayScheduleInfoSize = sizeof(HolidayScheduleInfo);

    /** @brief Actuator FSM phase (Initiated = motion timer running). */
    enum class LockActuatorState : uint8_t
    {
        kLockInitiated = 0,
        kLockCompleted,
        kUnlockInitiated,
        kUnlatchInitiated,
        kUnlockCompleted,
        kUnlatchCompleted,
    };

    LockActuatorState GetActuatorState() const { return mLockActuatorState; }

    /** @brief True during actuator motion or while `kUnlatchCompleted` (auto-unlock pending). */
    bool IsActuatorBusy() const;

    /** @brief `kLockCompleted` or `kUnlockCompleted` (idle at locked/unlocked; safe to replay coalesced requests). */
    bool IsAtTerminalLockOrUnlock() const;

    /** @brief May start unlock from `kLockCompleted` or `kUnlatchCompleted` (auto-unlock leg). */
    bool CanInitiateUnlockFromCurrentState() const;

    /** @brief Chip-thread: overwrite staged `LockRequest` (newest-wins) and post `kEventType_LockRequest`. */
    static void EnqueueLockRequest(const LockRequest & request);

    /** @brief AppTask-thread: move staged request to `out` if present; clears the slot. */
    static bool TryDrainStagedLockRequest(LockRequest & out);

    /** @brief Controller attribution for the follow-on unlock after unlatch. */
    struct UnlatchContext
    {
        chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
        chip::app::DataModel::Nullable<chip::FabricIndex> mFabricIdx;
        chip::app::DataModel::Nullable<chip::NodeId> mNodeId;
        chip::app::DataModel::Nullable<uint16_t> mUserIndex;
        LockOpCredentials mCredential{};
        bool mHasCredential = false;

        void Update(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                    const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                    const chip::app::DataModel::Nullable<uint16_t> & userIndex, const LockOpCredentials & cred, bool hasCred)
        {
            mEndpointId    = endpointId;
            mFabricIdx     = fabricIdx;
            mNodeId        = nodeId;
            mUserIndex     = userIndex;
            mCredential    = cred;
            mHasCredential = hasCred;
        }
    };

    UnlatchContext mUnlatchContext;              /**< Set when a remote unbolt/unlatch unlock starts. */
    LockRequest mPendingRequest;               /**< Newest-wins slot while `IsActuatorBusy()`. */
    bool mHasPendingRequest = false;
    LockRequest mActiveRemoteAction;           /**< Terminal remote push after actuator completes. */
    bool mHasActiveRemoteAction = false;
    LockActuatorState mLockActuatorState = LockActuatorState::kLockCompleted;

private:
    /** @brief Applies `lockParam`, migrates KVS if needed, creates timers/mutex. */
    CHIP_ERROR InitLockDomain(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state, LockParam lockParam,
                              chip::PersistentStorageDelegate * storage);

    /** @brief Re-keys KVS when configured resource limits change. */
    bool MigrateLockConfig(const LockParam & params);

    /** @brief `mLockTimer` callback; posts `kEventType_Timer` to AppTask. */
    static void TimerEventHandler(void * timerCbArg);

    /** @brief Posts `kEventType_Lock` (auto-relock or unlock after unlatch). */
    static void PostLockActionEvent(int32_t actor, LockAction action);

    static LockRequest sStagedLockRequest;     /**< Chip-thread → AppTask handoff (mutex). */
    static bool sStagedLockRequestValid;

    static osMutexId_t sLockSharedStateMutex;  /**< Protects staged and active remote slots. */

    osTimerId_t mLockTimer = nullptr;

    LockParam mLockParams;
    chip::PersistentStorageDelegate * mStorage = nullptr;
};
