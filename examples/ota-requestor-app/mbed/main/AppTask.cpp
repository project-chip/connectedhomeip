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
#include <LEDWidget.h>

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

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#else
#include "drivers/InterruptIn.h"
#include "platform/Callback.h"
#endif

static bool sIsWiFiStationProvisioned = false;
static bool sIsWiFiStationEnabled     = false;
static bool sIsWiFiStationConnected   = false;
static bool sIsPairedToAccount        = false;
static bool sHaveBLEConnections       = false;

static events::EventQueue sAppEventQueue;

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

#define FACTORY_RESET_TRIGGER_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_TRIGGER_TIMEOUT)
#define COMMISSIONING_RESET_TRIGGER_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_TRIGGER_TIMEOUT)
#define RESET_CANCEL_WINDOW_TIMEOUT (MBED_CONF_APP_RESET_CANCEL_WINDOW_TIMEOUT)
#define USER_RESPONSE_TIMEOUT (MBED_CONF_APP_USER_RESPONSE_TIMEOUT)

#define FUNCTION_BUTTON1 (MBED_CONF_APP_FUNCTION_BUTTON1)
#define FUNCTION_BUTTON2 (MBED_CONF_APP_FUNCTION_BUTTON2)

#ifdef CAPSENSE_ENABLED
static mbed::CapsenseButton CapFunctionButton1(Capsense::getInstance(), 0);
static mbed::CapsenseButton CapFunctionButton2(Capsense::getInstance(), 1);
#else
static mbed::InterruptIn sFunctionButton1(FUNCTION_BUTTON1);
static mbed::InterruptIn sFunctionButton2(FUNCTION_BUTTON2);
#endif

static mbed::Timeout sFunctionTimer[AppTask::kFunction_Button_last];

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

    // Initialize buttons
#ifdef CAPSENSE_ENABLED
    CapFunctionButton1.fall(mbed::callback(this, &AppTask::FunctionButton1PressEventHandler));
    CapFunctionButton1.rise(mbed::callback(this, &AppTask::FunctionButton1ReleaseEventHandler));
    CapFunctionButton2.fall(mbed::callback(this, &AppTask::FunctionButton2PressEventHandler));
    CapFunctionButton2.rise(mbed::callback(this, &AppTask::FunctionButton2ReleaseEventHandler));
#else
    sFunctionButton1.fall(mbed::callback(this, &AppTask::FunctionButton1PressEventHandler));
    sFunctionButton1.rise(mbed::callback(this, &AppTask::FunctionButton1ReleaseEventHandler));
    sFunctionButton2.fall(mbed::callback(this, &AppTask::FunctionButton2PressEventHandler));
    sFunctionButton2.rise(mbed::callback(this, &AppTask::FunctionButton2ReleaseEventHandler));
#endif

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

    error = GetDFUManager().Init(&mOnUpdateAvailableCallback, &mOnUpdateApplyCallback);
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

    ChipLogProgress(NotSpecified, "Mbed ota-requestor-app example application run");

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

        // If system is connected to Wi-Fi station, keep the LED On constantly.
        //
        // If Wi-Fi is provisioned, but not connected to Wi-Fi station yet
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
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

        sStatusLED.Animate();
    }
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

void AppTask::StartTimer(uint8_t index, uint32_t aTimeoutInMs)
{
    auto chronoTimeoutMs = std::chrono::duration<uint32_t, std::milli>(aTimeoutInMs);
    sFunctionTimer[index].attach(mTimerCallbacks[index], chronoTimeoutMs);
    mFunctionTimerActive[index] = true;
}

void AppTask::CancelTimer(uint8_t index)
{
    sFunctionTimer[index].detach();
    mFunctionTimerActive[index] = false;
}

void AppTask::TimerButton1EventHandler()
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Timer;
    event.TimerEvent.index = kFunction_Button_1;
    event.Handler          = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::TimerButton2EventHandler()
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Timer;
    event.TimerEvent.index = kFunction_Button_2;
    event.Handler          = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionButton1PressEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.button = kFunction_Button_1;
    button_event.ButtonEvent.action = kFunction_Button_push;
    button_event.Handler            = ButtonHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::FunctionButton1ReleaseEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.button = kFunction_Button_1;
    button_event.ButtonEvent.action = kFunction_Button_release;
    button_event.Handler            = ButtonHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::FunctionButton2PressEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.button = kFunction_Button_2;
    button_event.ButtonEvent.action = kFunction_Button_push;
    button_event.Handler            = ButtonHandler;
    sAppTask.PostEvent(&button_event);
}

void AppTask::FunctionButton2ReleaseEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.button = kFunction_Button_2;
    button_event.ButtonEvent.action = kFunction_Button_release;
    button_event.Handler            = ButtonHandler;
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
    button_event.ButtonEvent.button = id == 0 ? kFunction_Button_1 : kFunction_Button_2;
    button_event.ButtonEvent.action = pushed ? kFunction_Button_push : kFunction_Button_release;
    button_event.Handler            = ButtonHandler;

    sAppTask.PostEvent(&button_event);
}

void AppTask::ButtonHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Button)
        return;

    switch (aEvent->ButtonEvent.button)
    {
    case kFunction_Button_1:
        // To trigger a confirm response: press the FUNCTION_BUTTON1 button briefly (< FACTORY_RESET_TRIGGER_TIMEOUT)
        // To initiate factory reset: press the FUNCTION_BUTTON1 for FACTORY_RESET_TRIGGER_TIMEOUT +
        // RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset
        // has been initiated. To cancel factory reset: release the FUNCTION_BUTTON1 once all LEDs start blinking within the
        // RESET_CANCEL_WINDOW_TIMEOUT
        if (aEvent->ButtonEvent.action == kFunction_Button_push)
        {
            if (!sAppTask.mFunctionTimerActive[kFunction_Button_1] &&
                sAppTask.mFunction[kFunction_Button_1] == kFunction_NoneSelected)
            {
                sAppTask.StartTimer(kFunction_Button_1, FACTORY_RESET_TRIGGER_TIMEOUT);

                sAppTask.mFunction[kFunction_Button_1] = kFunction_ConfirmResponse;
            }
        }
        else
        {
            // If the button was released before factory reset got initiated, trigger a confirm response.
            if (sAppTask.mFunctionTimerActive[kFunction_Button_1] &&
                sAppTask.mFunction[kFunction_Button_1] == kFunction_ConfirmResponse)
            {
                sAppTask.CancelTimer(kFunction_Button_1);
                sAppTask.mFunction[kFunction_Button_1] = kFunction_NoneSelected;
                sAppTask.mUserResponseFlag.set(kUser_Response_confirm);
            }
            else if (sAppTask.mFunctionTimerActive[kFunction_Button_1] &&
                     sAppTask.mFunction[kFunction_Button_1] == kFunction_FactoryReset)
            {
                sAppTask.CancelTimer(kFunction_Button_1);

                // Change the function to none selected since factory reset has been canceled.
                sAppTask.mFunction[kFunction_Button_1] = kFunction_NoneSelected;

                ChipLogProgress(NotSpecified, "Factory Reset has been Canceled");
            }
        }
        break;
    case kFunction_Button_2:
        // To trigger a reject response: press the FUNCTION_BUTTON2 button briefly (< COMMISSIONING_RESET_TRIGGER_TIMEOUT)
        // To initiate commissioning reset: press the FUNCTION_BUTTON2 for COMMISSIONING_RESET_TRIGGER_TIMEOUT +
        // RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after COMMISSIONING_RESET_TRIGGER_TIMEOUT to signal commissioning
        // reset has been initiated. To cancel commissioning reset: release the FUNCTION_BUTTON2 once all LEDs start blinking within
        // the RESET_CANCEL_WINDOW_TIMEOUT
        if (aEvent->ButtonEvent.action == kFunction_Button_push)
        {
            if (!sAppTask.mFunctionTimerActive[kFunction_Button_2] &&
                sAppTask.mFunction[kFunction_Button_2] == kFunction_NoneSelected)
            {
                sAppTask.StartTimer(kFunction_Button_2, COMMISSIONING_RESET_TRIGGER_TIMEOUT);

                sAppTask.mFunction[kFunction_Button_2] = kFunction_RejectResponse;
            }
        }
        else
        {
            // If the button was released before factory reset got initiated, trigger a confirm response.
            if (sAppTask.mFunctionTimerActive[kFunction_Button_2] &&
                sAppTask.mFunction[kFunction_Button_2] == kFunction_RejectResponse)
            {
                sAppTask.CancelTimer(kFunction_Button_2);
                sAppTask.mFunction[kFunction_Button_2] = kFunction_NoneSelected;
                sAppTask.mUserResponseFlag.set(kUser_Response_reject);
            }
            else if (sAppTask.mFunctionTimerActive[kFunction_Button_2] &&
                     sAppTask.mFunction[kFunction_Button_2] == kFunction_CommissioningReset)
            {
                sAppTask.CancelTimer(kFunction_Button_2);

                // Change the function to none selected since factory reset has been canceled.
                sAppTask.mFunction[kFunction_Button_2] = kFunction_NoneSelected;

                ChipLogProgress(NotSpecified, "Commissioning Reset has been Canceled");
            }
        }
        break;
    default:
        ChipLogError(NotSpecified, "Button type not supported");
    }
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    switch (aEvent->TimerEvent.index)
    {
    case kFunction_Button_1:
        // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
        if (sAppTask.mFunctionTimerActive[kFunction_Button_1] &&
            sAppTask.mFunction[kFunction_Button_1] == kFunction_ConfirmResponse)
        {
            ChipLogProgress(NotSpecified, "Factory Reset Triggered. Release button within %ums to cancel.",
                            RESET_CANCEL_WINDOW_TIMEOUT);

            // Start timer for RESET_CANCEL_WINDOW_TIMEOUT to allow user to
            // cancel, if required.
            sAppTask.StartTimer(kFunction_Button_1, RESET_CANCEL_WINDOW_TIMEOUT);
            sAppTask.mFunction[kFunction_Button_1] = kFunction_FactoryReset;

            // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
            sStatusLED.Set(false);
            sStatusLED.Blink(500);
        }
        else if (sAppTask.mFunctionTimerActive[kFunction_Button_1] &&
                 sAppTask.mFunction[kFunction_Button_1] == kFunction_FactoryReset)
        {
            // Actually trigger Factory Reset
            ChipLogProgress(NotSpecified, "Factory Reset initiated");
            sAppTask.CancelTimer(kFunction_Button_1);
            sAppTask.mFunction[kFunction_Button_1] = kFunction_NoneSelected;

            chip::Server::GetInstance().ScheduleFactoryReset();
        }
        break;
    case kFunction_Button_2:
        // If we reached here, the button was held past COMMISSIONING_RESET_TRIGGER_TIMEOUT, initiate factory reset
        if (sAppTask.mFunctionTimerActive[kFunction_Button_2] && sAppTask.mFunction[kFunction_Button_2] == kFunction_RejectResponse)
        {
            ChipLogProgress(NotSpecified, "Commissioning Reset Triggered. Release button within %ums to cancel.",
                            RESET_CANCEL_WINDOW_TIMEOUT);

            // Start timer for RESET_CANCEL_WINDOW_TIMEOUT to allow user to
            // cancel, if required.
            sAppTask.StartTimer(kFunction_Button_2, RESET_CANCEL_WINDOW_TIMEOUT);
            sAppTask.mFunction[kFunction_Button_2] = kFunction_CommissioningReset;

            // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
            sStatusLED.Set(false);
            sStatusLED.Blink(500);
        }
        else if (sAppTask.mFunctionTimerActive[kFunction_Button_2] &&
                 sAppTask.mFunction[kFunction_Button_2] == kFunction_CommissioningReset)
        {
            // Actually trigger Commissioning Reset
            ChipLogProgress(NotSpecified, "Commissioning Reset initiated");
            sAppTask.CancelTimer(kFunction_Button_2);
            sAppTask.mFunction[kFunction_Button_2] = kFunction_NoneSelected;

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
        break;
    default:
        ChipLogError(NotSpecified, "Timer event index not supported [%d]", index);
    }
}

bool AppTask::OnUpdateAvailableHandler(void * context, uint32_t softwareVersion, chip::CharSpan softwareVersionString)
{
    AppTask * appTask = reinterpret_cast<AppTask *>(context);
    ChipLogProgress(NotSpecified, "\tNew update available: \t %.*s [%d]", static_cast<int>(softwareVersionString.size()),
                    softwareVersionString.data(), softwareVersion);

    ChipLogProgress(NotSpecified, "\tDo you want to download new update?");
    ChipLogProgress(NotSpecified, "\tRespond by pressing the button");
    ChipLogProgress(NotSpecified, "\t%10s%10s", "BUTTON0", "BUTTON1");
    ChipLogProgress(NotSpecified, "\t%10s%10s", "YES", "NO");
    if (USER_RESPONSE_TIMEOUT > 0)
    {
        ChipLogProgress(NotSpecified, "\tWaiting response timeout %d", std::chrono::seconds(USER_RESPONSE_TIMEOUT).count());
    }

    appTask->mUserResponseFlag.clear();
    uint32_t timeout  = USER_RESPONSE_TIMEOUT > 0 ? USER_RESPONSE_TIMEOUT : osWaitForever;
    uint32_t response = appTask->mUserResponseFlag.wait_any(kUser_Response_confirm | kUser_Response_reject, timeout);
    if (response == osFlagsErrorTimeout)
    {
        ChipLogProgress(NotSpecified, "\tWaiting for user response timeout...");
    }
    ChipLogProgress(NotSpecified, "\tDownload new update %s", response == kUser_Response_confirm ? "CONFIRM" : "REJECT");
    return response == kUser_Response_confirm;
}

bool AppTask::OnUpdateApplyHandler(void * context)
{
    AppTask * appTask = reinterpret_cast<AppTask *>(context);

    ChipLogProgress(NotSpecified, "\tNew update downloaded");
    ChipLogProgress(NotSpecified, "\tDo you want to apply new update?");
    ChipLogProgress(NotSpecified, "\tRespond by pressing the button");
    ChipLogProgress(NotSpecified, "\t%10s%10s", "YES", "NO");
    ChipLogProgress(NotSpecified, "\t%10s%10s", "BUTTON0", "BUTTON1");
    if (USER_RESPONSE_TIMEOUT > 0)
    {
        ChipLogProgress(NotSpecified, "\tWaiting response timeout %d", std::chrono::seconds(USER_RESPONSE_TIMEOUT).count());
    }

    appTask->mUserResponseFlag.clear();
    uint32_t timeout  = USER_RESPONSE_TIMEOUT > 0 ? USER_RESPONSE_TIMEOUT : osWaitForever;
    uint32_t response = appTask->mUserResponseFlag.wait_any(kUser_Response_confirm | kUser_Response_reject, timeout);
    if (response == osFlagsErrorTimeout)
    {
        ChipLogProgress(NotSpecified, "\tWaiting for user response timeout...");
    }
    ChipLogProgress(NotSpecified, "\tApply new update %s", response == kUser_Response_confirm ? "CONFIRM" : "REJECT");
    return response == kUser_Response_confirm;
}
