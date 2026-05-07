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

#include <cstring>

#include "AppEvent.h"
#include "BaseApplication.h"
#include "SilabsLockTypes.h"
#include <app/ConcreteAttributePath.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <ble/Ble.h>
#include <cmsis_os2.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TypeTraits.h>
#include <platform/CHIPDeviceLayer.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)
#define APP_ERROR_ALLOCATION_FAILED CHIP_APPLICATION_ERROR(0x07)
#if defined(ENABLE_CHIP_SHELL)
#define APP_ERROR_TOO_MANY_SHELL_ARGUMENTS CHIP_APPLICATION_ERROR(0x08)
#endif // ENABLE_CHIP_SHELL

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

    static AppTask & GetAppTask();

    // ---------------------------------------------------------------------
    // Customer-facing entry points.
    // Overridable in `CustomerAppTask` via `AppTaskImpl<>::*Impl()` hooks.
    // ---------------------------------------------------------------------

    /** @brief AppTask task main loop function. */
    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

    /**
     * @brief Button ISR callback: `APP_LOCK_SWITCH` or `APP_FUNCTION_BUTTON`.
     *
     * @param button    APP_LOCK_SWITCH or APP_FUNCTION_BUTTON.
     * @param btnAction SL_SIMPLE_BUTTON_PRESSED / RELEASED / DISABLED.
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /**
     * @brief AppTask-thread handler for a queued button-driven lock toggle
     *        (`kEventType_Button`, posted by `ButtonEventHandler` when
     *        `APP_LOCK_SWITCH` is pressed). Resolves direction via
     *        `NextState()` and drives `InitiateLockAction`.
     */
    static void LockButtonActionHandler(AppEvent * aEvent);

    /**
     * @brief DoorLock-cluster post-attribute-change callback.
     *        Routes e.g. `LockState` updates to telemetry / UI side-effects.
     */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    // ---------------------------------------------------------------------
    // DoorLock cluster data-model entry points.
    //
    // Each `DM*` mirrors the corresponding `emberAfPluginDoorLock*` weak
    // callback. The free callbacks in `AppTask.cpp` are 1-line forwarders
    // into these hooks, so customers can intercept data-model behavior by
    // overriding `AppTaskImpl<>::DM*Impl()` in `CustomerAppTask`.
    // ---------------------------------------------------------------------

    bool DMDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                     const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                     const chip::Optional<chip::ByteSpan> & pinCode,
                                     chip::app::Clusters::DoorLock::OperationErrorEnum & err);
    bool DMDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err);
    bool DMDoorLockOnDoorUnboltCommand(chip::EndpointId endpointId,
                                       const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                                       const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                                       const chip::Optional<chip::ByteSpan> & pinCode,
                                       chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    bool DMDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                 EmberAfPluginDoorLockCredentialInfo & credential);
    bool DMDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                 chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                 CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData);

    bool DMDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user);
    bool DMDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                           const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                           CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials);

    DlStatus DMDoorLockGetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockWeekDaySchedule & schedule);
    DlStatus DMDoorLockSetWeekDaySchedule(chip::EndpointId endpointId, uint8_t weekdayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, DaysMaskMap daysMask, uint8_t startHour, uint8_t startMinute,
                                          uint8_t endHour, uint8_t endMinute);

    DlStatus DMDoorLockGetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          EmberAfPluginDoorLockYearDaySchedule & schedule);
    DlStatus DMDoorLockSetYearDaySchedule(chip::EndpointId endpointId, uint8_t yearDayIndex, uint16_t userIndex,
                                          DlScheduleStatus status, uint32_t localStartTime, uint32_t localEndTime);

    DlStatus DMDoorLockGetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex,
                                          EmberAfPluginDoorLockHolidaySchedule & holidaySchedule);
    DlStatus DMDoorLockSetHolidaySchedule(chip::EndpointId endpointId, uint8_t holidayIndex, DlScheduleStatus status,
                                          uint32_t localStartTime, uint32_t localEndTime, OperatingModeEnum operatingMode);

    void DMDoorLockOnAutoRelock(chip::EndpointId endpointId);

    // ---------------------------------------------------------------------
    // Lock-domain actuator API. `InitiateLockAction` is invoked from the
    // `DM*` defaults above and from `LockMigration.cpp` when seeding state
    // from the legacy NVM3 layout.
    // ---------------------------------------------------------------------

    /** Door-lock actuator / cluster actions. */
    enum class LockAction : uint8_t
    {
        kLock = 0,
        kUnlock,
        kUnlatch,
        kInvalid,
    };

    bool InitiateLockAction(int32_t aActor, LockAction aAction);

    /**
     * @brief Finalize the unlatch sequence: push `kUnlocked` to the cluster
     *        using `mUnlatchContext`, then initiate the `kUnlock` actuator
     *        action. Static + addressable so it can be scheduled directly by
     *        the unlatch FreeRTOS timer helper (anonymous namespace, no
     *        private-member access).
     */
    static void UnlockAfterUnlatch(intptr_t context = 0);

protected:
    /** Override of `BaseApplication::AppInit()`. */
    CHIP_ERROR AppInit() override;

    /** Bring up the lock domain: cluster limits, storage, timers, LED. */
    CHIP_ERROR InitLock();

private:
    enum class LockActuatorState : uint8_t
    {
        kLockInitiated = 0,
        kLockCompleted,
        kUnlockInitiated,
        kUnlatchInitiated,
        kUnlockCompleted,
        kUnlatchCompleted,
    };

    struct UnlatchContext
    {
        static constexpr uint8_t kMaxPinLength = UINT8_MAX;
        uint8_t mPinBuffer[kMaxPinLength];
        uint8_t mPinLength           = 0;
        chip::EndpointId mEndpointId = chip::kInvalidEndpointId;
        chip::app::DataModel::Nullable<chip::FabricIndex> mFabricIdx;
        chip::app::DataModel::Nullable<chip::NodeId> mNodeId;
        chip::app::Clusters::DoorLock::OperationErrorEnum mErr;

        void Update(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                    const chip::app::DataModel::Nullable<chip::NodeId> & nodeId, const chip::Optional<chip::ByteSpan> & pin,
                    chip::app::Clusters::DoorLock::OperationErrorEnum & err)
        {
            mEndpointId = endpointId;
            mFabricIdx  = fabricIdx;
            mNodeId     = nodeId;
            mErr        = err;

            if (pin.HasValue())
            {
                memcpy(mPinBuffer, pin.Value().data(), pin.Value().size());
                mPinLength = static_cast<uint8_t>(pin.Value().size());
            }
            else
            {
                memset(mPinBuffer, 0, kMaxPinLength);
                mPinLength = 0;
            }
        }
    };

    // ---- Lock-domain bring-up / state machine ----
    CHIP_ERROR InitLockDomain(chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state,
                              SilabsDoorLock::LockInitParams::LockParam lockParam, chip::PersistentStorageDelegate * storage);

    bool MigrateLockConfig(const SilabsDoorLock::LockInitParams::LockParam & params);

    bool NextState();

    static void TimerEventHandler(void * timerCbArg);
    static void ActuatorMovementTimerEventHandler(AppEvent * aEvent);
    static void UpdateClusterState(intptr_t context);

    // ---- PIN validation / cluster state push ----
    bool Unlock(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                const chip::app::DataModel::Nullable<chip::NodeId> & nodeId, const chip::Optional<chip::ByteSpan> & pin,
                chip::app::Clusters::DoorLock::OperationErrorEnum & err);
    bool SetLockState(chip::EndpointId endpointId, const chip::app::DataModel::Nullable<chip::FabricIndex> & fabricIdx,
                      const chip::app::DataModel::Nullable<chip::NodeId> & nodeId,
                      chip::app::Clusters::DoorLock::DlLockState lockState, const chip::Optional<chip::ByteSpan> & pin,
                      chip::app::Clusters::DoorLock::OperationErrorEnum & err);

    // ---- State ----
    UnlatchContext mUnlatchContext;
    chip::EndpointId mCurrentEndpointId  = chip::kInvalidEndpointId;
    LockActuatorState mLockActuatorState = LockActuatorState::kLockCompleted;
    osTimerId_t mLockTimer               = nullptr;

    SilabsDoorLock::LockInitParams::LockParam mLockParams;
    chip::PersistentStorageDelegate * mStorage = nullptr;
};
