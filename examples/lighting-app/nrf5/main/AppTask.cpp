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
#include "AppEvent.h"
#include "DataModelHandler.h"
#include "LEDWidget.h"
#include "LightingManager.h"
#include "Server.h"

#include "app_button.h"
#include "app_config.h"
#include "app_timer.h"
#include "boards.h"

#include "nrf_log.h"

#include "FreeRTOS.h"

#include <platform/CHIPDeviceLayer.h>

APP_TIMER_DEF(sFunctionTimer);

namespace {

constexpr int kFactoryResetTriggerTimeout      = 3000;
constexpr int kFactoryResetCancelWindowTimeout = 3000;
constexpr size_t kAppTaskStackSize             = 4096;
constexpr int kAppTaskPriority                 = 2;
constexpr int kAppEventQueueSize               = 10;

SemaphoreHandle_t sCHIPEventLock;

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;
LEDWidget sUnusedLED;
LEDWidget sUnusedLED_1;

bool sIsThreadProvisioned     = false;
bool sIsThreadEnabled         = false;
bool sIsThreadAttached        = false;
bool sIsPairedToAccount       = false;
bool sHaveBLEConnections      = false;
bool sHaveServiceConnectivity = false;

} // namespace

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::StartAppTask()
{
    ret_code_t ret = NRF_SUCCESS;

    sAppEventQueue = xQueueCreate(kAppEventQueueSize, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        NRF_LOG_INFO("Failed to allocate app event queue");
        ret = NRF_ERROR_NULL;
        APP_ERROR_HANDLER(ret);
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", kAppTaskStackSize / sizeof(StackType_t), NULL, kAppTaskPriority, &sAppTaskHandle) != pdPASS)
    {
        ret = NRF_ERROR_NULL;
    }

    return ret;
}

int AppTask::Init()
{
    ret_code_t ret;

    // Initialize LEDs
    sStatusLED.Init(SYSTEM_STATE_LED);

    sUnusedLED.Init(BSP_LED_2);
    sUnusedLED_1.Init(BSP_LED_3);

    // Initialize buttons
    static app_button_cfg_t sButtons[] = {
        { LIGHTING_BUTTON, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, ButtonEventHandler },
        { FUNCTION_BUTTON, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, ButtonEventHandler },
    };

    ret = app_button_init(sButtons, ARRAY_SIZE(sButtons), pdMS_TO_TICKS(FUNCTION_BUTTON_DEBOUNCE_PERIOD_MS));
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_button_init() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_button_enable();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_button_enable() failed");
        APP_ERROR_HANDLER(ret);
    }

    // Initialize Timer for Function Selection
    ret = app_timer_init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_init() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = app_timer_create(&sFunctionTimer, APP_TIMER_MODE_SINGLE_SHOT, TimerEventHandler);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_create() failed");
        APP_ERROR_HANDLER(ret);
    }

    ret = LightingMgr().Init(LIGHTING_GPIO);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("LightingMgr().Init() failed");
        APP_ERROR_HANDLER(ret);
    }

    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sCHIPEventLock = xSemaphoreCreateMutex();
    if (sCHIPEventLock == NULL)
    {
        NRF_LOG_INFO("xSemaphoreCreateMutex() failed");
        APP_ERROR_HANDLER(NRF_ERROR_NULL);
    }

    return ret;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    ret_code_t ret;
    AppEvent event;

    ret = sAppTask.Init();
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("AppTask.Init() failed");
        APP_ERROR_HANDLER(ret);
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            sIsThreadAttached        = ConnectivityMgr().IsThreadAttached();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Consider the system to be "fully connected" if it has service
        // connectivity and it is able to interact with the service on a regular basis.
        bool isFullyConnected = sHaveServiceConnectivity;

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network yet OR no
        // connectivity to the service OR subscriptions are not fully established
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (isFullyConnected)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled && sIsPairedToAccount && (!sIsThreadAttached || !isFullyConnected))
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }

        sStatusLED.Animate();
        sUnusedLED.Animate();
        sUnusedLED_1.Animate();
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    LightingManager::Action_t action;
    ret_code_t ret = NRF_SUCCESS;

    if (aEvent->Type == AppEvent::kEventType_Lighting)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightingEvent.Action);
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LightingMgr().IsTurnedOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }
    }
    else
    {
        ret = NRF_ERROR_NULL;
    }

    if (ret == NRF_SUCCESS)
    {
        initiated = LightingMgr().InitiateAction(action);

        if (!initiated)
        {
            NRF_LOG_INFO("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if (pin_no != LIGHTING_BUTTON && pin_no != FUNCTION_BUTTON)
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == LIGHTING_BUTTON && button_action == APP_BUTTON_PUSH)
    {
        button_event.Handler = LightingActionEventHandler;
    }
    else if (pin_no == FUNCTION_BUTTON)
    {
        button_event.Handler = FunctionHandler;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(void * p_context)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = p_context;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past kFactoryResetTriggerTimeout, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        NRF_LOG_INFO("Factory Reset Triggered. Release button within %ums to cancel.", kFactoryResetTriggerTimeout);

        // Start timer for kFactoryResetCancelWindowTimeout to allow user to cancel, if required.
        sAppTask.StartTimer(kFactoryResetCancelWindowTimeout);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sUnusedLED_1.Set(false);
        sUnusedLED.Set(false);

        sStatusLED.Blink(500);
        sUnusedLED.Blink(500);
        sUnusedLED_1.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        NRF_LOG_INFO("Factory reset is not implemented");
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< kFactoryResetTriggerTimeout)
    // To initiate factory reset: press the FUNCTION_BUTTON for kFactoryResetTriggerTimeout + kFactoryResetCancelWindowTimeout
    // All LEDs start blinking after kFactoryResetTriggerTimeout to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // kFactoryResetCancelWindowTimeout
    if (aEvent->ButtonEvent.Action == APP_BUTTON_PUSH)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(kFactoryResetTriggerTimeout);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
            NRF_LOG_INFO("Software update is not implemented");
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sUnusedLED.Set(false);
            sUnusedLED_1.Set(false);

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            NRF_LOG_INFO("Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    ret_code_t ret;

    ret = app_timer_stop(sFunctionTimer);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_stop() failed");
        APP_ERROR_HANDLER(ret);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    ret_code_t ret;

    ret = app_timer_start(sFunctionTimer, pdMS_TO_TICKS(aTimeoutInMs), this);
    if (ret != NRF_SUCCESS)
    {
        NRF_LOG_INFO("app_timer_start() failed");
        APP_ERROR_HANDLER(ret);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LightingManager::ON_ACTION)
    {
        NRF_LOG_INFO("Turn On Action has been initiated")
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        NRF_LOG_INFO("Turn Off Action has been initiated")
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == LightingManager::ON_ACTION)
    {
        NRF_LOG_INFO("Turn On Action has been completed")
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        NRF_LOG_INFO("Turn Off Action has been completed")
    }
}

void AppTask::PostLightingActionRequest(LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type                 = AppEvent::kEventType_Lighting;
    event.LightingEvent.Action = aAction;
    event.Handler              = LightingActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            NRF_LOG_INFO("Failed to post event to app task event queue");
        }
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        NRF_LOG_INFO("Event received with no handler. Dropping event.");
    }
}
