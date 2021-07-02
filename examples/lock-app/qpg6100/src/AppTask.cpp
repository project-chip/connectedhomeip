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

#include "qvCHIP.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

#include <app/server/OnboardingCodesUtil.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

using namespace chip::TLV;
using namespace chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>
#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/qpg6100/ThreadStackManagerImpl.h>
#define JOINER_START_TRIGGER_TIMEOUT 1500
#endif

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (2048)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

static TaskHandle_t sAppTaskHandle;
static QueueHandle_t sAppEventQueue;

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

AppTask AppTask::sAppTask;

int AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    if (xTaskCreate(AppTaskMain, "APP", APP_TASK_STACK_SIZE / sizeof(StackType_t), NULL, 1, &sAppTaskHandle) != pdPASS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

int AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "Current Firmware Version: %s", CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION_STRING);

    err = BoltLockMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "BoltLockMgr().Init() failed");
        return err;
    }
    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Subscribe with our button callback to the qvCHIP button handler.
    qvCHIP_SetBtnCallback(ButtonEventHandler);

    qvCHIP_LedSet(LOCK_STATE_LED, !BoltLockMgr().IsUnlocked());

    // Init ZCL Data Model
    InitServer();
    UpdateClusterState();

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    int err;
    AppEvent event;

    err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        // appError(err);
    }

    ChipLogProgress(NotSpecified, "App Task started");

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
            sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network
        // yet OR no connectivity to the service OR subscriptions are not fully
        // established THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink
        // the LEDs at an even rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            // Consider the system to be "fully connected" if it has service
            // connectivity
            if (sHaveServiceConnectivity)
            {
                qvCHIP_LedSet(SYSTEM_STATE_LED, true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled)
            {
                qvCHIP_LedBlink(SYSTEM_STATE_LED, 950, 50);
            }
            else if (sHaveBLEConnections)
            {
                qvCHIP_LedBlink(SYSTEM_STATE_LED, 100, 100);
            }
            else
            {
                qvCHIP_LedBlink(SYSTEM_STATE_LED, 50, 950);
            }
        }
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    bool initiated = false;
    BoltLockManager::Action_t action;
    int32_t actor;
    int err = CHIP_NO_ERROR;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
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
        err = CHIP_ERROR_INTERNAL;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            ChipLogProgress(NotSpecified, "Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    if (btnIdx != APP_LOCK_BUTTON && btnIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_LOCK_BUTTON && btnPressed == true)
    {
        button_event.Handler = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (btnIdx == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState, CHIP_ERROR aError)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = aAppState;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
    {
#if CHIP_ENABLE_OPENTHREAD
        ChipLogProgress(NotSpecified, "Release button now to Start Thread Joiner");
        ChipLogProgress(NotSpecified, "Hold to trigger Factory Reset");
        sAppTask.mFunction = kFunction_Joiner;
        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_Joiner)
    {
#endif
        ChipLogProgress(NotSpecified, "Factory Reset Triggered. Release button within %ums to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        qvCHIP_LedSet(SYSTEM_STATE_LED, false);
        qvCHIP_LedSet(LOCK_STATE_LED, false);

        qvCHIP_LedBlink(SYSTEM_STATE_LED, 500, 500);
        qvCHIP_LedBlink(LOCK_STATE_LED, 500, 500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.ButtonIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    // To trigger BLE advertising: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == true)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
#if CHIP_ENABLE_OPENTHREAD
            sAppTask.StartTimer(JOINER_START_TRIGGER_TIMEOUT);
#else
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
#endif

            sAppTask.mFunction = kFunction_StartBleAdv;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger BLE advertising.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

            if (ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                ChipLogProgress(NotSpecified, "BLE advertising already in progress.");
            }
            else
            {
                if (!ConnectivityMgr().IsThreadProvisioned())
                {
                    // Enable BLE advertisements and pairing window
                    if (OpenDefaultPairingWindow(chip::ResetAdmins::kNo) == CHIP_NO_ERROR)
                    {
                        ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");
                    }
                }
                else
                {
                    ChipLogError(NotSpecified, "Network is already provisioned, BLE advertisement not enabled");
                }
            }
        }
#if CHIP_ENABLE_OPENTHREAD
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_Joiner)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

            CHIP_ERROR error = ThreadStackMgr().JoinerStart();
            ChipLogProgress(NotSpecified, "Thread joiner triggered: %s", chip::ErrorStr(error));
        }
#endif
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            // Set lock status LED back to show state of lock.
            qvCHIP_LedSet(LOCK_STATE_LED, !BoltLockMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    SystemLayer.CancelTimer(TimerEventHandler, this);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    CHIP_ERROR err;

    SystemLayer.CancelTimer(TimerEventHandler, this);
    err = SystemLayer.StartTimer(aTimeoutInMs, TimerEventHandler, this);
    SuccessOrExit(err);

    mFunctionTimerActive = true;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "StartTimer failed %s: ", chip::ErrorStr(err));
    }
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }

    qvCHIP_LedBlink(LOCK_STATE_LED, 50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been completed");

        qvCHIP_LedSet(LOCK_STATE_LED, true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been completed");

        qvCHIP_LedSet(LOCK_STATE_LED, false);
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            ChipLogError(NotSpecified, "Failed to post event to app task event queue");
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
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(void)
{
    uint8_t newValue = !BoltLockMgr().IsUnlocked();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                                 (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %" PRIx8, status);
    }
}
