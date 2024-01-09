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
#include "platform/CHIPDeviceLayer.h"
#include <LEDWidget.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/identify-server/identify-server.h>
#include <matter_config.h>
#include <stdio.h>
#include <wiced_led_manager.h>

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;

using namespace ::chip;
using namespace ::chip::app;
using namespace CYW30739DoorLock::LockInitParams;

wiced_bool_t syncClusterToButtonAction = false;

static Identify gIdentify = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kNone,
};

static wiced_led_config_t chip_lighting_led_config[] = {
    {
        .led    = PLATFORM_LED_1,
        .bright = 50,
    },
    {
        .led    = PLATFORM_LED_2,
        .bright = 50,
    },
};

static void ActionInitiated(LockManager::Action_t aAction, int32_t aActor);
static void ActionCompleted(LockManager::Action_t aAction);
static void UpdateClusterState(intptr_t context);

void InitAppTask()
{
    // Initial lock state
    LEDWidget().GetInstance().Init(chip_lighting_led_config, ArraySize(chip_lighting_led_config));
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
        wiced_led_manager_disable_led(PLATFORM_LED_2);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        ChipLogDetail(Zcl, "Unlock Action has been initiated");
        wiced_led_manager_enable_led(PLATFORM_LED_2);
    }
}

static void UpdateClusterState(intptr_t context)
{
    bool unlocked        = LockMgr().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    OperationSourceEnum source = OperationSourceEnum::kUnspecified;

    // write the new lock value
    EmberAfStatus status =
        DoorLockServer::Instance().SetLockState(1, newState, source) ? EMBER_ZCL_STATUS_SUCCESS : EMBER_ZCL_STATUS_FAILURE;

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(Zcl, "ERR: updating lock state %x", status);
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
