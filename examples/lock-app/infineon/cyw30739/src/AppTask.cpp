/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "AppTask.h"
#include "ButtonHandler.h"
#include <LEDWidget.h>
#include <LockManager.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <cycfg.h>
#include <matter_config.h>
#include <platform/CHIPDeviceLayer.h>
#include <wiced_led_manager.h>

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;

using namespace CYW30739DoorLock::LockInitParams;
using namespace chip::app;

wiced_bool_t syncClusterToButtonAction = false;

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
};

static wiced_led_config_t chip_lighting_led_config[] = {
    {
        .led    = PLATFORM_LED_RED,
        .bright = 50,
    },
#if defined(PLATFORM_LED_YELLOW_ENABLED)
    {
        .led    = PLATFORM_LED_YELLOW,
        .bright = 50,
    },
#endif
};

static void ActionInitiated(LockManager::Action_t aAction, int32_t aActor);
static void ActionCompleted(LockManager::Action_t aAction);
static void UpdateClusterState(intptr_t context);

void AppTaskMain(intptr_t args)
{
    wiced_result_t result;
    result = app_button_init();
    if (result != WICED_SUCCESS)
    {
        ChipLogError(Zcl, "ERROR app_button_init %d\n", result);
    }
    LEDWid().Init(chip_lighting_led_config, MATTER_ARRAY_SIZE(chip_lighting_led_config));

    CYW30739MatterConfig::InitApp();

    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ 1 };
    chip::DeviceLayer::PlatformMgr().LockChipStack();
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

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    LockMgr().Init(state,
                   ParamBuilder()
                       .SetNumberOfUsers(numberOfUsers)
                       .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                       .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                       .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                       .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                       .GetLockParam());

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);
}

static void ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LockManager::LOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Lock Action has been initiated");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        syncClusterToButtonAction = true;
    }

    // Action initiated, update the light led
    if (aAction == LockManager::LOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Lock Action has been initiated");
#if defined(PLATFORM_LED_YELLOW_ENABLED)
        wiced_led_manager_disable_led(PLATFORM_LED_YELLOW);
#endif
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been initiated");
#if defined(PLATFORM_LED_YELLOW_ENABLED)
        wiced_led_manager_enable_led(PLATFORM_LED_YELLOW);
#endif
    }
}

static void UpdateClusterState(intptr_t context)
{
    bool unlocked        = LockMgr().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    OperationSourceEnum source = OperationSourceEnum::kUnspecified;

    // write the new lock value
    Protocols::InteractionModel::Status status = DoorLockServer::Instance().SetLockState(1, newState, source)
        ? Protocols::InteractionModel::Status::Success
        : Protocols::InteractionModel::Status::Failure;

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(Zcl, "ERR: updating lock state %x", to_underlying(status));
    }
}

static void ActionCompleted(LockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == LockManager::LOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Lock Action has been completed");
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been completed");
    }

    if (syncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        syncClusterToButtonAction = false;
    }
}
