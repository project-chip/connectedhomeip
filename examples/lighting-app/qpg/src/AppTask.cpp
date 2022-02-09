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

#include "qvIO.h"

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "ota.h"

#include <app/server/OnboardingCodesUtil.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

using namespace chip::TLV;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;

#include <platform/CHIPDeviceLayer.h>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (3 * 1024)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

namespace {
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

bool sIsThreadProvisioned     = false;
bool sIsThreadEnabled         = false;
bool sHaveBLEConnections      = false;
bool sHaveServiceConnectivity = false;

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];

StaticQueue_t sAppEventQueueStruct;

StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t appTaskStruct;
} // namespace

AppTask AppTask::sAppTask;

namespace {
constexpr int extDiscTimeoutSecs = 20;
}

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreateStatic(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), sAppEventQueueBuffer, &sAppEventQueueStruct);
    if (sAppEventQueue == nullptr)
    {
        ChipLogError(NotSpecified, "Failed to allocate app event queue");
        return CHIP_ERROR_NO_MEMORY;
    }

    // Start App task.
    sAppTaskHandle = xTaskCreateStatic(AppTaskMain, APP_TASK_NAME, ArraySize(appStack), nullptr, 1, appStack, &appTaskStruct);
    if (sAppTaskHandle == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    err = LightingMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "LightingMgr().Init() failed");
        return err;
    }
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Subscribe with our button callback to the qvCHIP button handler.
    qvIO_SetBtnCallback(ButtonEventHandler);

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(extDiscTimeoutSecs);
#endif

    // Init ZCL Data Model
    chip::Server::GetInstance().Init();

    // Init OTA engine
    InitializeOTARequestor();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    UpdateClusterState();

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    // Enable BLE advertisements
    chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    ChipLogProgress(NotSpecified, "BLE advertising started. Waiting for Pairing.");

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

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
            sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
            sHaveBLEConnections  = (ConnectivityMgr().NumBLEConnections() != 0);
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
            if (sIsThreadProvisioned && sIsThreadEnabled)
            {
                qvIO_LedBlink(SYSTEM_STATE_LED, 950, 50);
            }
            else if (sHaveBLEConnections)
            {
                qvIO_LedBlink(SYSTEM_STATE_LED, 100, 100);
            }
            else
            {
                qvIO_LedBlink(SYSTEM_STATE_LED, 50, 950);
            }
        }
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action;

    if (aEvent->Type == AppEvent::kEventType_Button)
    {
        // Toggle light
        if (LightingMgr().IsTurnedOn())
        {
            action = LightingManager::OFF_ACTION;
        }
        else
        {
            action = LightingManager::ON_ACTION;
        }

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 0, 0);
    }
    if (aEvent->Type == AppEvent::kEventType_Level && aEvent->ButtonEvent.Action != 0)
    {
        // Toggle Dimming of light between 2 fixed levels
        uint8_t val = 0x0;
        val         = LightingMgr().GetLevel() == 0x7f ? 0x1 : 0x7f;
        action      = LightingManager::LEVEL_ACTION;

        sAppTask.mSyncClusterToButtonAction = true;
        LightingMgr().InitiateAction(action, 0, 1, &val);
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, bool btnPressed)
{
    if (btnIdx != APP_ON_OFF_BUTTON && btnIdx != APP_FUNCTION_BUTTON && btnIdx != APP_LEVEL_BUTTON)
    {
        return;
    }

    ChipLogProgress(NotSpecified, "ButtonEventHandler %d, %d", btnIdx, btnPressed);

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnPressed;

    if (btnIdx == APP_ON_OFF_BUTTON && btnPressed == true)
    {
        // Hand off to Light handler - On/Off light
        button_event.Handler = LightingActionEventHandler;
    }
    else if (btnIdx == APP_LEVEL_BUTTON)
    {
        // Hand off to Light handler - Change level of light
        button_event.Type    = AppEvent::kEventType_Level;
        button_event.Handler = LightingActionEventHandler;
    }
    else if (btnIdx == APP_FUNCTION_BUTTON)
    {
        // Hand off to Functionality handler - depends on duration of press
        button_event.Handler = FunctionHandler;
    }
    else
    {
        return;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::TimerEventHandler(chip::System::Layer * aLayer, void * aAppState)
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
        ChipLogProgress(NotSpecified, "[BTN] Factory Reset selected. Release within %us to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT / 1000);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is
        // co-ordinated.
        qvIO_LedSet(SYSTEM_STATE_LED, false);

        qvIO_LedBlink(SYSTEM_STATE_LED, 500, 500);
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
            ChipLogProgress(NotSpecified, "[BTN] Hold to select function:");
            ChipLogProgress(NotSpecified, "[BTN] - Trigger OTA (0-3s)");
            ChipLogProgress(NotSpecified, "[BTN] - Factory Reset (>6s)");

            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

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

            ChipLogProgress(NotSpecified, "[BTN] Triggering OTA Query");

            TriggerOTAQuery();
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "[BTN] Factory Reset has been Canceled");
        }
    }
}

void AppTask::CancelTimer()
{
    chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    CHIP_ERROR err;

    chip::DeviceLayer::SystemLayer().CancelTimer(TimerEventHandler, this);
    err = chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(aTimeoutInMs), TimerEventHandler, this);
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
    else
    {
        ChipLogError(NotSpecified, "Event Queue is NULL should never happen");
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

/**
 * Update cluster status after application level changes
 */
void AppTask::UpdateClusterState(void)
{
    uint8_t newValue;

    ChipLogProgress(NotSpecified, "UpdateClusterState");

    // write the new on/off value
    newValue             = LightingMgr().IsTurnedOn();
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                                 (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %" PRIx8, status);
    }

    newValue = LightingMgr().GetLevel();
    status   = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                   (uint8_t *) &newValue, ZCL_INT8U_ATTRIBUTE_TYPE);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating level %" PRIx8, status);
    }
}
