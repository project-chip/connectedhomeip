/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "QRCodeUtil.h"

#include "Server.h"
#include "attribute-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"

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
static bool sIsThreadAttached        = false;
static bool sIsPairedToAccount       = false;
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

    ChipLogProgress(NotSpecified, "Current Firmware Version: %s", CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION);

    err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "LightingMgr().Init() failed");
        return err;
    }
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Subscribe with our button callback to the qvCHIP button handler.
    qvCHIP_SetBtnCallback(ButtonEventHandler);

    // Init ZCL Data Model
    InitServer();
    UpdateClusterState();

    PrintQRCode(chip::RendezvousInformationFlags::kBLE);

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    int err;
    AppEvent event;
    uint64_t mLastChangeTimeUS = 0;

    err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        // appError(err);
    }

    ChipLogProgress(NotSpecified, "App Task started");
    // TODO: PR # 2939 - OT support
    // SetDeviceName("QPG6100LightingDemo._chip._udp.local.");

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
            sIsThreadAttached        = ConnectivityMgr().IsThreadAttached();
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
            else if (sIsThreadProvisioned && sIsThreadEnabled && sIsPairedToAccount &&
                     (!sIsThreadAttached || !sHaveServiceConnectivity))
            {
                qvCHIP_LedBlink(SYSTEM_STATE_LED, 950, 50);
            }
            else if (sHaveBLEConnections)
            {
                qvCHIP_LedBlink(SYSTEM_STATE_LED, 100, 100);
            }
            else
            {
                qvCHIP_LedBlink(SYSTEM_STATE_LED, 50, 1950);
            }
        }

        uint64_t nowUS            = chip::System::Layer::GetClock_Monotonic();
        uint64_t nextChangeTimeUS = mLastChangeTimeUS + 5 * 1000 * 1000UL;

        if (nowUS > nextChangeTimeUS)
        {
            // TODO: PR # 2939 - OT support
            /*
            PublishService();
            */
            mLastChangeTimeUS = nowUS;
        }
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action;
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (LightingMgr().IsTurnedOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }
        LightingMgr().InitiateAction(action, 0, 0, 0);
    }
    if (aEvent->Type == AppEvent::kEventType_Level && aEvent->ButtonEvent.Action != 0)
    {
        uint8_t val = 0x0;
        val         = LightingMgr().GetLevel() == 0x7f ? 0x1 : 0x7f;
        action      = LightingManager::LEVEL_ACTION;
        LightingMgr().InitiateAction(action, 0, 1, &val);
    }
    return;
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);
    if (btnIdx != APP_ON_OFF_BUTTON && btnIdx != APP_FUNCTION_BUTTON)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_ON_OFF_BUTTON && btnPressed == true)
    {
        button_event.Handler = LightingActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (btnIdx == APP_FUNCTION_BUTTON)
    {
        // TODO hijacked the function button to change level
        button_event.Type    = AppEvent::kEventType_Level;
        button_event.Handler = LightingActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState, chip::System::Error aError)
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
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
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

        qvCHIP_LedBlink(SYSTEM_STATE_LED, 500, 500);
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

    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
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

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a
        // software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();

            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogError(NotSpecified, "Software Update currently not supported.");
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

void AppTask::ActionInitiated(LightingManager::Action_t aAction)
{
    // Placeholder for light action
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light goes on");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light goes off ");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction)
{
    // Placeholder for light action completed
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Light Off Action has been completed");
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }
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
    uint8_t newValue = !LightingMgr().IsTurnedOn();
    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                                 (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %x", status);
    }

    ChipLogProgress(NotSpecified, "UpdateClusterState");
    newValue = LightingMgr().GetLevel();
    // TODO understand well enough to implement the level cluster ZCL_CURRENT_LEVEL_ATTRIBUTE_ID
    status = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                   (uint8_t *) &newValue, ZCL_DATA8_ATTRIBUTE_TYPE);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating level %x", status);
    }
}
