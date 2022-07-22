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
#include "BoltLockManager.h"
#include <LEDWidget.h>

#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

// mbed-os headers
#include "drivers/Timeout.h"
#include "events/EventQueue.h"

// ZAP -- ZCL Advanced Platform
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/attribute-storage.h>

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#else
#include "drivers/InterruptIn.h"
#include "platform/Callback.h"
#endif

#define FACTORY_RESET_TRIGGER_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_TRIGGER_TIMEOUT)
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_CANCEL_WINDOW_TIMEOUT)
#define LOCK_BUTTON (MBED_CONF_APP_LOCK_BUTTON)
#define FUNCTION_BUTTON (MBED_CONF_APP_FUNCTION_BUTTON)
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

constexpr uint32_t kPublishServicePeriodUs = 5000000;

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);
static LEDWidget sLockLED(MBED_CONF_APP_LOCK_STATE_LED);

#ifdef CAPSENSE_ENABLED
static mbed::CapsenseButton CapFunctionButton(Capsense::getInstance(), 0);
static mbed::CapsenseButton CapLockButton(Capsense::getInstance(), 1);
#else
static mbed::InterruptIn sLockButton(LOCK_BUTTON);
static mbed::InterruptIn sFunctionButton(FUNCTION_BUTTON);
#endif

static bool sIsWiFiStationProvisioned = false;
static bool sIsWiFiStationEnabled     = false;
static bool sIsWiFiStationConnected   = false;
static bool sIsPairedToAccount        = false;
static bool sHaveBLEConnections       = false;

static mbed::Timeout sFunctionTimer;

static events::EventQueue sAppEventQueue;

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::Init()
{
    CHIP_ERROR error;
    // Register the callback to init the MDNS server when connectivity is available
    PlatformMgr().AddEventHandler(
        [](const ChipDeviceEvent * event, intptr_t arg) {
            // Restart the server whenever an ip address is renewed
            if (event->Type == DeviceEventType::kInternetConnectivityChange)
            {
                if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established ||
                    event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
                {
                    chip::app::DnssdServer::Instance().StartServer();
                }
            }
        },
        0);

    // Initialize LEDs
    sLockLED.Set(!BoltLockMgr().IsUnlocked());

    // Initialize buttons
#ifdef CAPSENSE_ENABLED
    CapFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    CapFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
    CapLockButton.fall(mbed::callback(this, &AppTask::LockButtonPressEventHandler));
#else
    sLockButton.fall(mbed::callback(this, &AppTask::LockButtonPressEventHandler));
    sFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    sFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
#endif

    // Initialize lock manager
    BoltLockMgr().Init();
    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    error = Server::GetInstance().Init(initParams);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Server initialization failed: %s", error.AsString());
        return EXIT_FAILURE;
    }

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    ConfigurationMgr().LogDeviceConfig();
    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    error = GetDFUManager().Init();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "DFU manager initialization failed: %s", error.AsString());
        return EXIT_FAILURE;
    }

    return 0;
}

int AppTask::StartApp()
{
    int ret = Init();
    if (ret)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        return ret;
    }

    ChipLogProgress(NotSpecified, "Mbed lock-app example application run");

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
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
        //
        // If system is connected to Wi-Fi station, keep the LED On constantly.
        //
        // If Wi-Fi is provisioned, but not connected to Wi-Fi station yet
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
            if (sIsWiFiStationConnected)
            {
                sStatusLED.Set(true);
            }
            else if (sIsWiFiStationProvisioned && sIsWiFiStationEnabled && sIsPairedToAccount && !sIsWiFiStationConnected)
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
        sLockLED.Animate();
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    BoltLockManager::Action_t action = BoltLockManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = BoltLockMgr().IsUnlocked() ? BoltLockManager::LOCK_ACTION : BoltLockManager::UNLOCK_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != BoltLockManager::INVALID_ACTION && !BoltLockMgr().InitiateAction(actor, action))
        ChipLogProgress(NotSpecified, "Action is already in progress or active.");
}

void AppTask::LockButtonPressEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = LOCK_BUTTON;
    button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
    button_event.Handler            = LockActionEventHandler;
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

void AppTask::ButtonEventHandler(uint32_t id, bool pushed)
{
    if (id > 1)
    {
        ChipLogError(NotSpecified, "Wrong button ID");
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = id == 0 ? LOCK_BUTTON : FUNCTION_BUTTON;
    button_event.ButtonEvent.Action = pushed ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;

    if (id == 0)
    {
        button_event.Handler = LockActionEventHandler;
    }
    else
    {
        button_event.Handler = FunctionHandler;
    }

    sAppTask.PostEvent(&button_event);
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

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been completed");
        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been completed");
        sLockLED.Set(false);
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
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
    {
        ChipLogProgress(NotSpecified, "Factory Reset Triggered. Release button within %ums to cancel.",
                        FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
        // cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Set lock status LED back to show state of lock.
        sLockLED.Set(!BoltLockMgr().IsUnlocked());

        // Actually trigger Factory Reset
        ChipLogProgress(NotSpecified, "Factory Reset initiated");
        sAppTask.mFunction = kFunction_NoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
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

            sAppTask.mFunction = kFunction_StartBleAdv;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;

            chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();

            if (ConnectivityMgr().IsBLEAdvertisingEnabled())
            {
                ChipLogProgress(NotSpecified, "BLE advertising is already enabled");
                return;
            }

            if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
            {
                ChipLogProgress(NotSpecified, "OpenBasicCommissioningWindow() failed");
            }
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            // Set lock status LED back to show state of lock.
            sLockLED.Set(!BoltLockMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            ChipLogProgress(NotSpecified, "Factory Reset has been Canceled");
        }
    }
}

void AppTask::UpdateClusterState()
{
    uint8_t newValue = !BoltLockMgr().IsUnlocked();

    // write the new on/off value
    EmberAfStatus status =
        emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ZCL update failed: %lx", status);
    }
}
