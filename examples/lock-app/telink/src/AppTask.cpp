/*
 *
 *    Copyright (c) 2023-2024 Project CHIP Authors
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
#include "ButtonManager.h"
#include "LEDManager.h"
#include <LockManager.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/data-model/Nullable.h>
#include <app/server/Server.h>
#include <credentials/FabricTable.h>

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace ::chip::app::Clusters::DoorLock;
using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace TelinkDoorLock::LockInitParams;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init(void)
{
    SetExampleButtonCallbacks(LockActionEventHandler);
    InitCommonParts();

    LedManager::getInstance().setLed(LedManager::EAppLed_App0, LockMgr().IsLocked());

    chip::app::DataModel::Nullable<chip::app::Clusters::DoorLock::DlLockState> state;
    chip::EndpointId endpointId{ kExampleEndpointId };
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::app::Clusters::DoorLock::Attributes::LockState::Get(endpointId, state);

    uint8_t numberOfCredentialsPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfCredentialsSupportedPerUser(endpointId, numberOfCredentialsPerUser))
    {
        ChipLogError(Zcl,
                     "Unable to get number of credentials supported per user when initializing lock endpoint, defaulting to %d "
                     "[endpointId=%d]",
                     APP_DEFAULT_CREDENTIAL_COUNT, endpointId);
        numberOfCredentialsPerUser = APP_DEFAULT_CREDENTIAL_COUNT;
    }

    uint16_t numberOfUsers = 0;
    if (!DoorLockServer::Instance().GetNumberOfUserSupported(endpointId, numberOfUsers))
    {
        ChipLogError(Zcl,
                     "Unable to get number of supported users when initializing lock endpoint, defaulting to %d [endpointId=%d]",
                     APP_DEFAULT_USERS_COUNT, endpointId);
        numberOfUsers = APP_DEFAULT_USERS_COUNT;
    }

    uint8_t numberOfWeekdaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfWeekDaySchedulesPerUserSupported(endpointId, numberOfWeekdaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported weekday schedules when initializing lock endpoint, defaulting to %d [endpointId=%d]",
            APP_DEFAULT_WEEKDAY_SCHEDULE_PER_USER_COUNT, endpointId);
        numberOfWeekdaySchedulesPerUser = APP_DEFAULT_WEEKDAY_SCHEDULE_PER_USER_COUNT;
    }

    uint8_t numberOfYeardaySchedulesPerUser = 0;
    if (!DoorLockServer::Instance().GetNumberOfYearDaySchedulesPerUserSupported(endpointId, numberOfYeardaySchedulesPerUser))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported yearday schedules when initializing lock endpoint, defaulting to %d [endpointId=%d]",
            APP_DEFAULT_YEARDAY_SCHEDULE_PER_USER_COUNT, endpointId);
        numberOfYeardaySchedulesPerUser = APP_DEFAULT_YEARDAY_SCHEDULE_PER_USER_COUNT;
    }

    uint8_t numberOfHolidaySchedules = 0;
    if (!DoorLockServer::Instance().GetNumberOfHolidaySchedulesSupported(endpointId, numberOfHolidaySchedules))
    {
        ChipLogError(
            Zcl,
            "Unable to get number of supported holiday schedules when initializing lock endpoint, defaulting to %d [endpointId=%d]",
            APP_DEFAULT_HOLYDAY_SCHEDULE_PER_USER_COUNT, endpointId);
        numberOfHolidaySchedules = APP_DEFAULT_HOLYDAY_SCHEDULE_PER_USER_COUNT;
    }

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    CHIP_ERROR err = CHIP_NO_ERROR;

    err = LockMgr().Init(state,
                         ParamBuilder()
                             .SetNumberOfUsers(numberOfUsers)
                             .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                             .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                             .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                             .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                             .GetLockParam(),
                         LockStateChanged);

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("LockMgr().Init() failed");
        return err;
    }

    // Disable auto-relock time feature.
    DoorLockServer::Instance().SetAutoRelockTime(kExampleEndpointId, 0);

    return CHIP_NO_ERROR;
}

/* This is a button handler only */
void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    switch (LockMgr().getLockState())
    {
    case LockManager::kState_NotFulyLocked:
    case LockManager::kState_LockCompleted:
        LockMgr().LockAction(AppEvent::kEventType_DeviceAction, LockManager::UNLOCK_ACTION, LockManager::OperationSource::kButton,
                             kExampleEndpointId);
        break;
    case LockManager::kState_UnlockCompleted:
        LockMgr().LockAction(AppEvent::kEventType_DeviceAction, LockManager::LOCK_ACTION, LockManager::OperationSource::kButton,
                             kExampleEndpointId);
        break;
    default:
        LOG_INF("Lock is in intermediate state, ignoring button");
        break;
    }
}

void AppTask::LockStateChanged(LockManager::State_t state)
{
    switch (state)
    {
    case LockManager::State_t::kState_LockInitiated:
        LOG_INF("Callback: Lock action initiated");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, 50, 50);
        break;
    case LockManager::State_t::kState_LockCompleted:
        LOG_INF("Callback: Lock action completed");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, true);
        break;
    case LockManager::State_t::kState_UnlockInitiated:
        LOG_INF("Callback: Unlock action initiated");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, 50, 50);
        break;
    case LockManager::State_t::kState_UnlockCompleted:
        LOG_INF("Callback: Unlock action completed");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, false);
        break;
    case LockManager::State_t::kState_UnlatchInitiated:
        LOG_INF("Callback: Unbolt action initiated");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, 75, 25);
        break;
    case LockManager::State_t::kState_UnlatchCompleted:
        LOG_INF("Callback: Unbolt action completed");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, 25, 75);
        break;
    case LockManager::State_t::kState_NotFulyLocked:
        LOG_INF("Callback: Lock not fully locked. Unexpected state");
        LedManager::getInstance().setLed(LedManager::EAppLed_App0, 10, 90);
        break;
    }
}

void AppTask::LockJammedEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = LockJammedActionHandler;
    GetAppTask().PostEvent(&event);
}

void AppTask::LockJammedActionHandler(AppEvent * aEvent)
{
    LOG_INF("Sending a lock jammed event");

    /* Generating Door Lock Jammed event */
    DoorLockServer::Instance().SendLockAlarmEvent(kExampleEndpointId, AlarmCodeEnum::kLockJammed);
}

void AppTask::LockStateEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = LockStateActionHandler;
    GetAppTask().PostEvent(&event);
}

void AppTask::LockStateActionHandler(AppEvent * aEvent)
{
    LOG_INF("Sending a lock state event");

    // This code was written for testing purpose only
    // For real door status the level detection may be used instead of pulse
    static DoorStateEnum mDoorState = DoorStateEnum::kDoorOpen;
    if (mDoorState == DoorStateEnum::kDoorOpen)
    {
        mDoorState = DoorStateEnum::kDoorClosed;
    }
    else
    {
        mDoorState = DoorStateEnum::kDoorOpen;
    }

    /* Generating Door Lock Status event */
    DoorLockServer::Instance().SetDoorState(kExampleEndpointId, mDoorState);
}

void AppTask::LinkButtons(ButtonManager & buttonManager)
{
    buttonManager.addCallback(FactoryResetButtonEventHandler, 0, true);
    buttonManager.addCallback(ExampleActionButtonEventHandler, 1, true);
    buttonManager.addCallback(LockJammedEventHandler, 2, true);
    buttonManager.addCallback(LockStateEventHandler, 3, true);
}

void AppTask::LinkLeds(LedManager & ledManager)
{
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    ledManager.linkLed(LedManager::EAppLed_Status, 0);
    ledManager.linkLed(LedManager::EAppLed_App0, 1);
#else
    ledManager.linkLed(LedManager::EAppLed_App0, 0);
#endif // CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
}
