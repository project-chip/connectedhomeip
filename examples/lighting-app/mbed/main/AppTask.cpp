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

#include "AppTask.h"
#include "LEDWidget.h"
#include "LightingManager.h"
#include "OnboardingCodesUtil.h"

// FIXME: Undefine the `sleep()` function included by the CHIPDeviceLayer.h
// from unistd.h to avoid a conflicting declaration with the `sleep()` provided
// by Mbed-OS in mbed_power_mgmt.h.
#define sleep unistd_sleep
#include "Server.h"
#include <platform/CHIPDeviceLayer.h>
#undef sleep

#include <support/logging/CHIPLogging.h>

// ZAP -- ZCL Advanced Platform
#include "attribute-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"

// mbed-os headers
#include "drivers/InterruptIn.h"
#include "drivers/Timeout.h"
#include "events/EventQueue.h"
#include "platform/Callback.h"

#define FACTORY_RESET_TRIGGER_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_TRIGGER_TIMEOUT)
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_CANCEL_WINDOW_TIMEOUT)
#define LIGHTING_BUTTON (MBED_CONF_APP_LIGHTING_BUTTON)
#define FUNCTION_BUTTON (MBED_CONF_APP_FUNCTION_BUTTON)
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

constexpr uint32_t kPublishServicePeriodUs = 5000000;

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

static mbed::InterruptIn sLightingButton(LIGHTING_BUTTON);
static mbed::InterruptIn sFunctionButton(FUNCTION_BUTTON);

static bool sIsWiFiStationProvisioned = false;
static bool sIsWiFiStationEnabled     = false;
static bool sIsWiFiStationConnected   = false;
static bool sIsPairedToAccount        = false;
static bool sHaveBLEConnections       = false;
static bool sHaveServiceConnectivity  = false;

static mbed::Timeout sFunctionTimer;

// TODO: change EventQueue default event size
static events::EventQueue sAppEventQueue;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::Init()
{
    //-------------
    // Initialize button
    sLightingButton.fall(mbed::callback(this, &AppTask::LightingButtonPressEventHandler));
    sFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    sFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
    //----------------
    // Initialize lighting manager
    LightingMgr().Init(MBED_CONF_APP_LIGHTING_STATE_LED);
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Start BLE advertising if needed
    if (!CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART)
    {
        ChipLogProgress(NotSpecified, "Enabling BLE advertising.");
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);
    }
#ifdef MBED_CONF_APP_DEVICE_NAME
    ConnectivityMgr().SetBLEDeviceName(MBED_CONF_APP_DEVICE_NAME);
#endif

    chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();

    // Init ZCL Data Model and start server
    InitServer();
    ConfigurationMgr().LogDeviceConfig();
    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    return 0;
}

int AppTask::StartApp()
{
    int ret                            = Init();
    uint64_t mLastPublishServiceTimeUS = 0;

    if (ret)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        return ret;
    }

    while (true)
    {
        sAppEventQueue.dispatch(100);

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).

        if (PlatformMgr().TryLockChipStack())
        {
            sIsWiFiStationProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sIsWiFiStationEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiStationConnected   = ConnectivityMgr().IsWiFiStationConnected();
            sHaveBLEConnections       = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity  = ConnectivityMgr().HaveServiceConnectivity();
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
            else if (sIsWiFiStationProvisioned && sIsWiFiStationEnabled && sIsPairedToAccount &&
                     (!sIsWiFiStationConnected || !isFullyConnected))
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

        uint64_t nowUS            = chip::System::Platform::Layer::GetClock_Monotonic();
        uint64_t nextChangeTimeUS = mLastPublishServiceTimeUS + kPublishServicePeriodUs;

        if (nowUS > nextChangeTimeUS)
        {
            // TODO:
            // PublishService();
            mLastPublishServiceTimeUS = nowUS;
        }
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action = LightingManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lighting)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightingEvent.Action);
        actor  = aEvent->LightingEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = LightingMgr().IsTurnedOn() ? LightingManager::OFF_ACTION : LightingManager::ON_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != LightingManager::INVALID_ACTION && !LightingMgr().InitiateAction(action, actor, 0, NULL))
        ChipLogProgress(NotSpecified, "Action is already in progress or active.");
}

void AppTask::LightingButtonPressEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = LIGHTING_BUTTON;
    button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
    button_event.Handler            = LightingActionEventHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::FunctionButtonPressEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = FUNCTION_BUTTON;
    button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
    button_event.Handler            = FunctionHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::FunctionButtonReleaseEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = FUNCTION_BUTTON;
    button_event.ButtonEvent.Action = BUTTON_RELEASE_EVENT;
    button_event.Handler            = FunctionHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Turn On Action has been initiated");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Turn Off Action has been initiated");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        ChipLogProgress(NotSpecified, "Level Action has been initiated");
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction, int32_t aActor)
{
    if (aAction == LightingManager::ON_ACTION)
    {
        ChipLogProgress(NotSpecified, "Turn On Action has been completed");
    }
    else if (aAction == LightingManager::OFF_ACTION)
    {
        ChipLogProgress(NotSpecified, "Turn Off Action has been completed");
    }
    else if (aAction == LightingManager::LEVEL_ACTION)
    {
        ChipLogProgress(NotSpecified, "Level Action has been completed");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::CancelTimer()
{
    sFunctionTimer.detach();
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    auto chronoTimeoutMs = std::chrono::duration<uint32_t, std::milli>(aTimeoutInMs);
    sFunctionTimer.attach(mbed::callback(this, &AppTask::TimerEventHandler), chronoTimeoutMs);
    mFunctionTimerActive = true;
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    auto handle = sAppEventQueue.call([event = *aEvent, this] { DispatchEvent(&event); });
    if (!handle)
    {
        ChipLogError(NotSpecified, "Failed to post event to app task event queue: Not enough memory");
    }
}

void AppTask::DispatchEvent(const AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(const_cast<AppEvent *>(aEvent));
    }
    else
    {
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

void AppTask::TimerEventHandler()
{
    AppEvent event;
    event.Type = AppEvent::kEventType_Timer;
    // event.TimerEvent.Context = nullptr;
    event.Handler = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

// static
void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        ChipLogProgress(NotSpecified, "Factory Reset Triggered. Release button within %ums to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;

        sStatusLED.Set(false);
        sStatusLED.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        ChipLogProgress(NotSpecified, "Factory Reset initiated");
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.Pin != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< FACTORY_RESET_TRIGGER_TIMEOUT)
    // To initiate factory reset: press the FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    // All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == BUTTON_PUSH_EVENT)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

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
            ChipLogError(NotSpecified, "Software Update not supported.");
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "Factory Reset has been Canceled");
        }
    }
}

void AppTask::UpdateClusterState()
{
    uint8_t onoff = LightingMgr().IsTurnedOn();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &onoff,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "Updating on/off cluster failed: %x", status);
    }

    uint8_t level = LightingMgr().GetLevel();

    status = emberAfWriteAttribute(1, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &level,
                                   ZCL_DATA8_ATTRIBUTE_TYPE);

    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "Updating level cluster failed: %x", status);
    }
}