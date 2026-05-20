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

/** @brief Lock-app-specific application error codes*/
/** @{ */
#define APP_ERROR_ALLOCATION_FAILED CHIP_APPLICATION_ERROR(0x07)
#if defined(ENABLE_CHIP_SHELL)
#define APP_ERROR_TOO_MANY_SHELL_ARGUMENTS CHIP_APPLICATION_ERROR(0x08)
#endif // ENABLE_CHIP_SHELL
/** @} */

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
    /** @brief Default-constructed application task; use `GetAppTask()` for the live instance. */
    AppTask() = default;

    /** @brief Singleton accessor; returns the active `CustomerAppTask` instance. */
    static AppTask & GetAppTask();

    // ---------------------------------------------------------------------
    // Lock-init parameters (cluster resource limits read at AppInit).
    // Customers can use these from `InitLockImpl()` overrides to customize
    // resource limits without reaching into the door-lock cluster directly.
    // ---------------------------------------------------------------------

    /** @brief Cluster resource limits captured from ZAP attributes at AppInit time. */
    struct LockParam
    {
        uint16_t numberOfUsers                  = 0;
        uint8_t numberOfCredentialsPerUser      = 0;
        uint8_t numberOfWeekdaySchedulesPerUser = 0;
        uint8_t numberOfYeardaySchedulesPerUser = 0;
        uint8_t numberOfHolidaySchedules        = 0;
    };

    /** @brief Fluent builder for `LockParam`. */
    class ParamBuilder
    {
    public:
        /** @brief Sets `numberOfUsers`; returns `*this` for chaining. */
        ParamBuilder & SetNumberOfUsers(uint16_t numberOfUsers)
        {
            mLockParam.numberOfUsers = numberOfUsers;
            return *this;
        }
        /** @brief Sets `numberOfCredentialsPerUser`; returns `*this` for chaining. */
        ParamBuilder & SetNumberOfCredentialsPerUser(uint8_t numberOfCredentialsPerUser)
        {
            mLockParam.numberOfCredentialsPerUser = numberOfCredentialsPerUser;
            return *this;
        }
        /** @brief Sets `numberOfWeekdaySchedulesPerUser`; returns `*this` for chaining. */
        ParamBuilder & SetNumberOfWeekdaySchedulesPerUser(uint8_t numberOfWeekdaySchedulesPerUser)
        {
            mLockParam.numberOfWeekdaySchedulesPerUser = numberOfWeekdaySchedulesPerUser;
            return *this;
        }
        /** @brief Sets `numberOfYeardaySchedulesPerUser`; returns `*this` for chaining. */
        ParamBuilder & SetNumberOfYeardaySchedulesPerUser(uint8_t numberOfYeardaySchedulesPerUser)
        {
            mLockParam.numberOfYeardaySchedulesPerUser = numberOfYeardaySchedulesPerUser;
            return *this;
        }
        /** @brief Sets `numberOfHolidaySchedules`; returns `*this` for chaining. */
        ParamBuilder & SetNumberOfHolidaySchedules(uint8_t numberOfHolidaySchedules)
        {
            mLockParam.numberOfHolidaySchedules = numberOfHolidaySchedules;
            return *this;
        }
        /** @brief Returns the assembled `LockParam`. */
        LockParam GetLockParam() { return mLockParam; }

    private:
        LockParam mLockParam;
    };

    // ---------------------------------------------------------------------
    // Lock-action types (referenced by member functions below).
    // ---------------------------------------------------------------------

    /** @brief Door-lock actuator / cluster actions. */
    enum class LockAction : uint8_t
    {
        kLock = 0,
        kUnlock,
        kUnlatch,
        kInvalid,
    };

    /** @brief Work item for a lock/unlock/unlatch operation and its cluster attribution. */
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

    // ---------------------------------------------------------------------
    // Customer-facing entry points.
    // Overridable in `CustomerAppTask` via `AppTaskImpl<>::*Impl()` hooks.
    // ---------------------------------------------------------------------

    /** @brief AppTask task main loop function. */
    static void AppTaskMain(void * pvParameter);

    /** @brief Start the AppTask FreeRTOS thread; runs `AppTaskMain` as entry. */
    CHIP_ERROR StartAppTask();

    /** @brief Platform button callback; posts an `AppEvent` to the AppTask queue. */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /** @brief AppTask handler for lock-button presses; dispatches via `HandleLockRequestOnAppTask`. */
    static void LockButtonActionHandler(AppEvent * aEvent);

    /** @brief Unlatch timer callback; schedules `UnlockAfterUnlatch` on the chip thread. */
    static void UnlatchCallback(void * argument);

    /** @brief AppTask handler for actuator-movement completion; advances state and drains pending/staged requests. */
    static void ActuatorMovementEventHandler(AppEvent * aEvent);

    /** @brief AppTask handler for `kEventType_Lock`; invokes `InitiateLockAction`. */
    static void LockActionEventHandler(AppEvent * aEvent);

    /** @brief AppTask handler for `kEventType_LockRequest`; drains the staged `LockRequest` and routes it. */
    static void LockRequestEventHandler(AppEvent * aEvent);

    /** @brief AppTask router for `LockRequest`s; coalesces while busy, drives the actuator and cluster state. */
    void HandleLockRequestOnAppTask(const LockRequest & request);

    /** @brief Ember `MatterPostAttributeChangeCallback` hook; forwards DoorLock `LockState` changes to integrations. */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    // ---------------------------------------------------------------------
    // DoorLock cluster data-model entry points.
    // ---------------------------------------------------------------------

    /** @brief DoorLock cluster Lock command (chip thread); validates input and stages a `LockRequest`. */
    bool DMDoorLockOnDoorLockCommand(chip::EndpointId endpointId,
                                     const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                     const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                     const chip::Optional<chip::ByteSpan> & pinCode,
                                     chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief DoorLock cluster Unlock command; stages `kUnlatch` when `SupportsUnbolt`, else `kUnlock`. */
    bool DMDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief DoorLock cluster Unbolt command; stages a `kUnlock` `LockRequest`. */
    bool DMDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief Read a credential record from persistent storage. */
    bool DMDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                 EmberAfPluginDoorLockCredentialInfo & credential);

    /** @brief Persist a credential record. */
    bool DMDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                 chip::FabricIndex modifier, DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                                 const chip::ByteSpan & credentialData);

    /** @brief Read a user record (and its credential map) from persistent storage. */
    bool DMDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);

    /** @brief Persist a user record (and its credential map). */
    bool DMDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                           const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                           CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials);

    /** @brief Read a weekday schedule entry. */
    DlStatus DMDoorLockGetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule);
    /** @brief Persist a weekday schedule entry. */
    DlStatus DMDoorLockSetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute);

    /** @brief Read a year-day schedule entry. */
    DlStatus DMDoorLockGetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule);
    /** @brief Persist a year-day schedule entry. */
    DlStatus DMDoorLockSetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime);

    /** @brief Read a holiday schedule entry. */
    DlStatus DMDoorLockGetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & holidaySchedule);
    /** @brief Persist a holiday schedule entry. */
    DlStatus DMDoorLockSetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode);

    /** @brief Cluster auto-relock trigger; posts `kEventType_Lock` to re-lock the actuator. */
    void DMDoorLockOnAutoRelock(chip::EndpointId endpointId);

    /** @brief Validate a PIN against stored credentials; outputs match info or sets `err` on reject. */
    bool ValidatePin(chip::EndpointId endpointId, const chip::Optional<chip::ByteSpan> & pin,
                     chip::app::DataModel::Nullable<uint16_t> & outUserIndex, LockOpCredentials & outCred, bool & outHasCred,
                     chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    /** @brief Push a remote-sourced `LockState` to the cluster. Caller must hold the chip stack lock. */
    void PushClusterLockState(chip::EndpointId endpointId, chip::app::Clusters::DoorLock::DlLockState lockState,
                              const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                              const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                              const chip::app::DataModel::Nullable<uint16_t> & userIndex, const LockOpCredentials * cred,
                              bool hasCred);

    /** @brief Start an actuator transition; returns false if busy or invalid for current state. */
    bool InitiateLockAction(LockAction aAction, bool fromButton = false);

    /** @brief Chip-thread continuation after unlatch hold expires; transitions and posts the unlock leg. */
    static void UnlockAfterUnlatch(intptr_t context);

protected:
    // ---------------------------------------------------------------------
    // Persistent-storage record layouts.
    // ---------------------------------------------------------------------

    /** @brief Serialized weekday schedule record layout for persistent storage. */
    struct WeekDayScheduleInfo
    {
        DlScheduleStatus status;
        EmberAfPluginDoorLockWeekDaySchedule schedule;
    };

    /** @brief Serialized year-day schedule record layout for persistent storage. */
    struct YearDayScheduleInfo
    {
        DlScheduleStatus status;
        EmberAfPluginDoorLockYearDaySchedule schedule;
    };

    /** @brief Serialized holiday schedule record layout for persistent storage. */
    struct HolidayScheduleInfo
    {
        DlScheduleStatus status;
        EmberAfPluginDoorLockHolidaySchedule schedule;
    };

    /** @brief Serialized door-lock user record layout for persistent storage. */
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

    /** @brief Serialized credential record layout for persistent storage. */
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

    /** @brief Override of `BaseApplication::AppInit()`. */
    CHIP_ERROR AppInit() override;

    /** @brief Bring up the lock domain: cluster limits, storage, timers, LED. */
    CHIP_ERROR InitLock();

    /** @brief Actuator transition phase inside the lock state machine. */
    enum class LockActuatorState : uint8_t
    {
        kLockInitiated = 0,
        kLockCompleted,
        kUnlockInitiated,
        kUnlatchInitiated,
        kUnlockCompleted,
        kUnlatchCompleted,
    };

    /** @brief Current actuator state. */
    LockActuatorState GetActuatorState() const { return mLockActuatorState; }

    /** @brief True while a transition is in flight (any `*Initiated` plus `kUnlatchCompleted`). */
    bool IsActuatorBusy() const;

    /** @brief Chip-thread producer: stage `request` under mutex (newest-wins) and wake the AppTask consumer. */
    static void EnqueueLockRequest(const LockRequest & request);

    /** @brief Snapshot the staged `LockRequest` into `out` and clear it; returns true iff drained. */
    static bool TryDrainStagedLockRequest(LockRequest & out);

    /** @brief Attribution carried across unlatch hold timing into the unlock leg. */
    struct UnlatchContext
    {
        chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
        chip::app::DataModel::Nullable<chip::FabricIndex> mFabricIdx;
        chip::app::DataModel::Nullable<chip::NodeId> mNodeId;
        chip::app::DataModel::Nullable<uint16_t> mUserIndex;
        LockOpCredentials mCredential{};
        bool mHasCredential = false;

        /** @brief Copies attribution fields into this context. */
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

    /** @brief Attribution used when scheduling `UnlockAfterUnlatch` after unlatch. */
    UnlatchContext mUnlatchContext;
    /** @brief Lock request queued while the actuator is busy. */
    LockRequest mPendingRequest;
    /** @brief True when `mPendingRequest` contains a valid staged request. */
    bool mHasPendingRequest = false;
    /** @brief Context for the in-flight remote lock operation driving cluster updates. */
    LockRequest mActiveRemoteAction;
    /** @brief True when `mActiveRemoteAction` is valid for cluster attribution. */
    bool mHasActiveRemoteAction = false;
    /** @brief Current actuator phase for the lock hardware state machine. */
    LockActuatorState mLockActuatorState = LockActuatorState::kLockCompleted;

private:
    // ---- Lock-domain bring-up / state machine ----
    CHIP_ERROR InitLockDomain(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state, LockParam lockParam,
                              chip::PersistentStorageDelegate * storage);

    bool MigrateLockConfig(const LockParam & params);

    static void TimerEventHandler(void * timerCbArg);

    static void UpdateClusterState(intptr_t context);

    static void PostLockActionEvent(int32_t actor, LockAction action);

    static LockRequest sStagedLockRequest;
    static bool sStagedLockRequestValid;

    // Guards cross-thread shared state for the lock app:
    //   - `sStagedLockRequest` / `sStagedLockRequestValid` (chip-thread producer,
    //     AppTask-thread consumer; newest-wins coalescing).
    //   - `mActiveRemoteAction` / `mHasActiveRemoteAction` (written by both
    //     `UnlockAfterUnlatch` on the chip thread and
    //     `HandleLockRequestOnAppTask` / `ActuatorMovementEventHandler` on the AppTask thread).
    // Non-recursive; never held across the chip stack lock from the AppTask thread.
    static osMutexId_t sLockSharedStateMutex;

    osTimerId_t mLockTimer = nullptr;

    LockParam mLockParams;
    chip::PersistentStorageDelegate * mStorage = nullptr;
};
