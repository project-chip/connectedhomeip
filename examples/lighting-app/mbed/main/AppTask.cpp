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

#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

// mbed-os headers
#include "drivers/Timeout.h"
#include "events/EventQueue.h"

// ZAP -- ZCL Advanced Platform
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/attribute-storage.h>

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#else
#include "drivers/InterruptIn.h"
#include "platform/Callback.h"
#endif

#define FACTORY_RESET_TRIGGER_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_TRIGGER_TIMEOUT)
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_CANCEL_WINDOW_TIMEOUT)
#define LIGHTING_BUTTON (MBED_CONF_APP_LIGHTING_BUTTON)
#define FUNCTION_BUTTON (MBED_CONF_APP_FUNCTION_BUTTON)
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

#ifdef CAPSENSE_ENABLED
static mbed::CapsenseButton CapFunctionButton(Capsense::getInstance(), 0);
static mbed::CapsenseButton CapLockButton(Capsense::getInstance(), 1);
static mbed::CapsenseSlider CapSlider(Capsense::getInstance());
#else
static mbed::InterruptIn sLightingButton(LIGHTING_BUTTON);
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

    //-------------
    // Initialize button
#ifdef CAPSENSE_ENABLED
    CapFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    CapFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
    CapLockButton.fall(mbed::callback(this, &AppTask::LightingButtonPressEventHandler));
    CapSlider.on_move(mbed::callback(this, &AppTask::SliderEventHandler));
#else
    sLightingButton.fall(mbed::callback(this, &AppTask::LightingButtonPressEventHandler));
    sFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    sFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
#endif

    // Initialize lighting manager
    LightingMgr().Init(MBED_CONF_APP_LIGHTING_STATE_LED);
    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

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

    ChipLogProgress(NotSpecified, "Mbed lighting-app example application run");

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
    }
}

void AppTask::LightingActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action = LightingManager::INVALID_ACTION;
    int32_t actor                    = 0;
    uint8_t value                    = 0;
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
    else if (aEvent->Type == AppEvent::kEventType_Slider)
    {
        action = LightingManager::LEVEL_ACTION;
        actor  = AppEvent::kEventType_Slider;
        value  = aEvent->SliderEvent.Value;
    }

    if (action != LightingManager::INVALID_ACTION && !LightingMgr().InitiateAction(action, actor, 0, &value))
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

void AppTask::ButtonEventHandler(uint32_t id, bool pushed)
{
    if (id > 1)
    {
        ChipLogError(NotSpecified, "Wrong button ID");
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = id == 0 ? LIGHTING_BUTTON : FUNCTION_BUTTON;
    button_event.ButtonEvent.Action = pushed ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;

    if (id == 0)
    {
        button_event.Handler = LightingActionEventHandler;
    }
    else
    {
        button_event.Handler = FunctionHandler;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::SliderEventHandler(int slider_pos)
{
    AppEvent slider_event;
    slider_event.Type              = AppEvent::kEventType_Slider;
    slider_event.SliderEvent.Value = slider_pos;
    slider_event.Handler           = LightingActionEventHandler;
    sAppTask.PostEvent(&slider_event);
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
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_StartBleAdv)
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
    Protocols::InteractionModel::Status status = app::Clusters::OnOff::Attributes::OnOff::Set(1, onoff);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "Updating on/off cluster failed: %x", to_underlying(status));
    }

    uint8_t level = LightingMgr().GetLevel();

    status = app::Clusters::LevelControl::Attributes::CurrentLevel::Set(1, level);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "Updating level cluster failed: %x", to_underlying(status));
    }
}
