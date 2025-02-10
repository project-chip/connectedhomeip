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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#if defined(ENABLE_CHIP_SHELL)
#include "EventHandlerLibShell.h"
#endif // ENABLE_CHIP_SHELL

#include "LEDWidget.h"

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>

#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <assert.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <lib/support/CodeUtils.h>

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <platform/CHIPDeviceLayer.h>
#define SYSTEM_STATE_LED 0
#define LOCK_STATE_LED 1

#define APP_FUNCTION_BUTTON 0
#define APP_LOCK_SWITCH 1

using chip::app::Clusters::DoorLock::DlLockState;
using chip::app::Clusters::DoorLock::OperationErrorEnum;
using chip::app::Clusters::DoorLock::OperationSourceEnum;

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;
using namespace ::chip::DeviceLayer::Internal;
using namespace EFR32DoorLock::LockInitParams;

namespace {
LEDWidget sLockLED;
TimerHandle_t sUnlatchTimer;

void UpdateClusterStateAfterUnlatch(intptr_t context)
{
    LockMgr().UnlockAfterUnlatch();
}

void UnlatchTimerCallback(TimerHandle_t xTimer)
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterStateAfterUnlatch, reinterpret_cast<intptr_t>(nullptr));
}

void CancelUnlatchTimer(void)
{
    if (xTimerStop(sUnlatchTimer, pdMS_TO_TICKS(0)) == pdFAIL)
    {
        SILABS_LOG("sUnlatchTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void StartUnlatchTimer(uint32_t timeoutMs)
{
    if (xTimerIsTimerActive(sUnlatchTimer))
    {
        SILABS_LOG("app timer already started!");
        CancelUnlatchTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ms if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerStart(sUnlatchTimer, pdMS_TO_TICKS(timeoutMs)) != pdPASS)
    {
        SILABS_LOG("sUnlatchTimer timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

} // namespace

using namespace chip::TLV;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(AppTask::ButtonEventHandler);

#ifdef DISPLAY_ENABLED
    GetLCD().Init((uint8_t *) "Lock-App", true);
#endif

    err = BaseApplication::Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("BaseApplication::Init() failed");
        appError(err);
    }

#if defined(ENABLE_CHIP_SHELL)
    err = RegisterLockEvents();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("RegisterLockEvents() failed");
        appError(err);
    }
#endif // ENABLE_CHIP_SHELL

    // Initial lock state
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

    err = LockMgr().Init(state,
                         ParamBuilder()
                             .SetNumberOfUsers(numberOfUsers)
                             .SetNumberOfCredentialsPerUser(numberOfCredentialsPerUser)
                             .SetNumberOfWeekdaySchedulesPerUser(numberOfWeekdaySchedulesPerUser)
                             .SetNumberOfYeardaySchedulesPerUser(numberOfYeardaySchedulesPerUser)
                             .SetNumberOfHolidaySchedules(numberOfHolidaySchedules)
                             .GetLockParam());

    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("LockMgr().Init() failed");
        appError(err);
    }

    LockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(state.Value() == DlLockState::kUnlocked);

    sUnlatchTimer = xTimerCreate("UnlatchTimer", pdMS_TO_TICKS(UNLATCH_TIME_MS), pdFALSE, (void *) 0, UnlatchTimerCallback);

    // Update the LCD with the Stored value. Show QR Code if not provisioned
#ifdef DISPLAY_ENABLED
    GetLCD().WriteDemoUI(state.Value() != DlLockState::kUnlocked);
#ifdef QR_CODE_ENABLED
#ifdef SL_WIFI
    if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
    if (!ConnectivityMgr().IsThreadProvisioned())
#endif /* !SL_WIFI */
    {
        GetLCD().ShowQRCode(true);
    }
#endif // QR_CODE_ENABLED
#endif

    chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));

    ConfigurationMgr().LogDeviceConfig();

    return err;
}

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    sAppTask.StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");

    // Users and credentials should be checked once from nvm flash on boot
    LockMgr().ReadConfigValues();

    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, NULL, 0);
        }
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    LockManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<LockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LockMgr().NextState() == true)
        {
            action = LockManager::LOCK_ACTION;
        }
        else
        {
            action = LockManager::UNLOCK_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            SILABS_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_LOCK_SWITCH && btnAction == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        button_event.Handler = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::ActionInitiated(LockManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LockManager::UNLOCK_ACTION || aAction == LockManager::LOCK_ACTION)
    {
        bool locked = (aAction == LockManager::LOCK_ACTION);
        SILABS_LOG("%s Action has been initiated", (locked) ? "Lock" : "Unlock");
        sLockLED.Set(!locked);

#ifdef DISPLAY_ENABLED
        sAppTask.GetLCD().WriteDemoUI(locked);
#endif // DISPLAY_ENABLED
    }
    else if (aAction == LockManager::UNLATCH_ACTION)
    {
        SILABS_LOG("Unlatch Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(LockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the lock trait.
    // Turn off the lock LED if in a LOCKED state OR
    // Turn on the lock LED if in an UNLOCKED state.
    if (aAction == LockManager::LOCK_ACTION)
    {
        SILABS_LOG("Lock Action has been completed")
    }
    else if (aAction == LockManager::UNLATCH_ACTION)
    {
        SILABS_LOG("Unlatch Action has been completed")
        StartUnlatchTimer(UNLATCH_TIME_MS);
    }
    else if (aAction == LockManager::UNLOCK_ACTION)
    {
        SILABS_LOG("Unlock Action has been completed")
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::ActionRequest(int32_t aActor, LockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::UpdateClusterState(intptr_t context)
{
    bool unlocked        = LockMgr().NextState();
    DlLockState newState = unlocked ? DlLockState::kUnlocked : DlLockState::kLocked;

    // write the new lock value
    Protocols::InteractionModel::Status status = DoorLockServer::Instance().SetLockState(1, newState)
        ? Protocols::InteractionModel::Status::Success
        : Protocols::InteractionModel::Status::Failure;

    if (status != Protocols::InteractionModel::Status::Success)
    {
        SILABS_LOG("ERR: updating lock state %x", to_underlying(status));
    }
}
