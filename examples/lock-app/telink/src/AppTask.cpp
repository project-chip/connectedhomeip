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

#include "AppTask.h"
#include "ButtonManager.h"
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

namespace {
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
LEDWidget sLockLED;
#endif
} // namespace

AppTask AppTask::sAppTask;
static const struct gpio_dt_spec sLockJammedInputDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_5), gpios);
static const struct gpio_dt_spec sLockStatusInputDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_6), gpios);
Button sLockJammedAction;
Button sLockStatusChangedAction;

CHIP_ERROR AppTask::Init(void)
{
#if APP_USE_EXAMPLE_START_BUTTON
    SetExampleButtonCallbacks(LockActionEventHandler);
#endif
    sLockJammedAction.Configure(&sLockJammedInputDt, LockJammedEventHandler);
    sLockStatusChangedAction.Configure(&sLockStatusInputDt, LockStateEventHandler);
    ButtonManagerInst().AddButton(sLockJammedAction);
    ButtonManagerInst().AddButton(sLockStatusChangedAction);
    InitCommonParts();

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    sLockLED.Init(GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios));
    sLockLED.Set(LockMgr().IsLocked());
#endif

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
        LockMgr().LockAction(AppEvent::kEventType_Lock, LockManager::UNLOCK_ACTION, LockManager::OperationSource::kButton,
                             kExampleEndpointId);
        break;
    case LockManager::kState_UnlockCompleted:
        LockMgr().LockAction(AppEvent::kEventType_Lock, LockManager::LOCK_ACTION, LockManager::OperationSource::kButton,
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
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(50, 50);
#endif
        break;
    case LockManager::State_t::kState_LockCompleted:
        LOG_INF("Callback: Lock action completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Set(true);
#endif
        break;
    case LockManager::State_t::kState_UnlockInitiated:
        LOG_INF("Callback: Unlock action initiated");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(50, 50);
#endif
        break;
    case LockManager::State_t::kState_UnlockCompleted:
        LOG_INF("Callback: Unlock action completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Set(false);
#endif
        break;
    case LockManager::State_t::kState_UnlatchInitiated:
        LOG_INF("Callback: Unbolt action initiated");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(75, 25);
#endif
        break;
    case LockManager::State_t::kState_UnlatchCompleted:
        LOG_INF("Callback: Unbolt action completed");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(25, 75);
#endif
        break;
    case LockManager::State_t::kState_NotFulyLocked:
        LOG_INF("Callback: Lock not fully locked. Unexpected state");
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        sLockLED.Blink(10, 90);
#endif
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
