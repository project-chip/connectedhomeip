/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "Button.h"
#include "LEDWidget.h"
#include "esp_log.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-enums.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lock/AppConfig.h>
#include <lock/AppEvent.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_EVENT_QUEUE_SIZE 10
#define APP_TASK_STACK_SIZE (3000)
#define APP_TASK_PRIORITY 2
#define STATUS_LED_GPIO_NUM GPIO_NUM_2 // Use LED1 (blue LED) as status LED on DevKitC

static const char * const TAG = "lock-app";

namespace {
TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

LEDWidget sStatusLED;
LEDWidget sLockLED;

Button resetButton;
Button lockButton;

BaseType_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sHaveBLEConnections = false;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
} // namespace

using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate app event queue");
        return APP_ERROR_EVENT_QUEUE_FAILED;
    }

    // Start App task.
    sAppTaskHandle = xTaskCreate(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), NULL, 1, NULL);
    return sAppTaskHandle ? CHIP_NO_ERROR : APP_ERROR_CREATE_TASK_FAILED;
}

CHIP_ERROR AppTask::Init()
{
    // Create FreeRTOS sw timer for Function Selection
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );

    chip::DeviceLayer::StackLock lock;
    CHIP_ERROR err = BoltLockMgr().InitLockState();

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sStatusLED.Init(SYSTEM_STATE_LED);
    sLockLED.Init(LOCK_STATE_LED);

    resetButton.Init(APP_FUNCTION_BUTTON, APP_BUTTON_DEBOUNCE_PERIOD_MS);
    lockButton.Init(APP_LOCK_BUTTON, APP_BUTTON_DEBOUNCE_PERIOD_MS);

    sLockLED.Set(!BoltLockMgr().IsUnlocked());

    chip::DeviceLayer::SystemLayer().ScheduleWork(UpdateClusterState, nullptr);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    Clock::Timestamp lastChangeTime = Clock::kZero;

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGI(TAG, "AppTask.Init() failed due to %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    ESP_LOGI(TAG, "App Task started");

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If no connectivity to the service OR subscriptions are not fully
        // established THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink
        // the LEDs at an even rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
        }

        sStatusLED.Animate();
        sLockLED.Animate();

        Clock::Timestamp now            = SystemClock().GetMonotonicTimestamp();
        Clock::Timestamp nextChangeTime = lastChangeTime + Clock::Seconds16(5);

        if (nextChangeTime < now)
        {
            lastChangeTime = now;
        }
        if (lockButton.Poll())
        {
            if (lockButton.IsPressed())
            {
                GetAppTask().ButtonEventHandler(APP_LOCK_BUTTON, APP_BUTTON_PRESSED);
            }
        }
        if (resetButton.Poll())
        {
            if (resetButton.IsPressed())
            {
                GetAppTask().ButtonEventHandler(APP_FUNCTION_BUTTON, APP_BUTTON_PRESSED);
            }
        }
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    BoltLockManager::Action_t action;
    int32_t actor;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aEvent->mType == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->mLockEvent.mAction);
        actor  = aEvent->mLockEvent.mActor;
    }
    else if (aEvent->mType == AppEvent::kEventType_Button)
    {
        if (BoltLockMgr().IsUnlocked())
        {
            action = BoltLockManager::LOCK_ACTION;
        }
        else
        {
            action = BoltLockManager::UNLOCK_ACTION;
        }
        actor = AppEvent::kEventType_Button;
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            ESP_LOGI(TAG, "Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    if (btnIdx != APP_LOCK_BUTTON && btnIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    AppEvent button_event             = {};
    button_event.mType                = AppEvent::kEventType_Button;
    button_event.mButtonEvent.mPinNo  = btnIdx;
    button_event.mButtonEvent.mAction = btnAction;

    if (btnIdx == APP_LOCK_BUTTON && btnAction == APP_BUTTON_PRESSED)
    {
        button_event.mHandler = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (btnIdx == APP_FUNCTION_BUTTON)
    {
        button_event.mHandler = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.mType                = AppEvent::kEventType_Timer;
    event.mTimerEvent.mContext = (void *) xTimer;
    event.mHandler             = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->mType != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
    {
        ESP_LOGI(TAG, "Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->mButtonEvent.mPinNo != APP_FUNCTION_BUTTON)
    {
        return;
    }

    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->mButtonEvent.mAction == APP_BUTTON_PRESSED)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
            sAppTask.mFunction = kFunction_StartBleAdv;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, start BLE advertissement in fast mode
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

            if (!ConnectivityMgr().IsThreadProvisioned())
            {
                // Enable BLE advertisements
                ConnectivityMgr().SetBLEAdvertisingEnabled(true);
                ConnectivityMgr().SetBLEAdvertisingMode(ConnectivityMgr().kFastAdvertising);
            }
            else
            {
                ESP_LOGI(TAG, "Network is already provisioned, Ble advertissement not enabled");
            }
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            // Set lock status LED back to show state of lock.
            sLockLED.Set(!BoltLockMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            ESP_LOGI(TAG, "Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        ESP_LOGI(TAG, "app timer stop() failed");
        return;
    }

    mFunctionTimerActive = false;
}
void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        ESP_LOGI(TAG, "app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        ESP_LOGI(TAG, "app timer start() failed");
        return;
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ESP_LOGI(TAG, "Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ESP_LOGI(TAG, "Unlock Action has been initiated");
    }
    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ESP_LOGI(TAG, "Lock Action has been completed");

        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ESP_LOGI(TAG, "Unlock Action has been completed");

        sLockLED.Set(false);
    }
    if (sAppTask.mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::SystemLayer().ScheduleWork(UpdateClusterState, nullptr);
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction)
{
    AppEvent event;
    event.mType              = AppEvent::kEventType_Lock;
    event.mLockEvent.mActor  = aActor;
    event.mLockEvent.mAction = aAction;
    event.mHandler           = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            ESP_LOGI(TAG, "Failed to post event to app task event queue");
        }
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->mHandler)
    {
        aEvent->mHandler(aEvent);
    }
    else
    {
        ESP_LOGI(TAG, "Event received with no handler. Dropping event.");
    }
}

/* if unlocked then it locked it first*/
void AppTask::UpdateClusterState(chip::System::Layer *, void * context)
{
    uint8_t newValue = !BoltLockMgr().IsUnlocked();

    // write the new on/off value
    EmberAfStatus status = chip::app::Clusters::OnOff::Attributes::OnOff::Set(1, newValue);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ESP_LOGI(TAG, "ERR: updating on/off %x", status);
    }
}
