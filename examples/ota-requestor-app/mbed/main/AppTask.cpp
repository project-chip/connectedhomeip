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
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

// mbed-os headers
#include "drivers/Timeout.h"
#include "events/EventQueue.h"

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#endif

#ifdef CHIP_OTA_REQUESTOR
#include "GenericOTARequestorDriver.h"
#include <BDXDownloader.h>
#include <OTAImageProcessorImpl.h>
#include <OTARequestor.h>
#endif // CHIP_OTA_REQUESTOR

#ifdef BOOT_ENABLED
#include "blockdevice/SlicingBlockDevice.h"
#include <bootutil/bootutil.h>
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
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT (MBED_CONF_APP_FACTORY_RESET_CANCEL_WINDOW_TIMEOUT)

#define FUNCTION_BUTTON (MBED_CONF_APP_FUNCTION_BUTTON)
#define BLE_BUTTON (MBED_CONF_APP_BLE_BUTTON)
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

#ifdef CAPSENSE_ENABLED
static mbed::CapsenseButton CapFunctionButton(Capsense::getInstance(), 0);
static mbed::CapsenseButton CapBleButton(Capsense::getInstance(), 1);
#else
static mbed::InterruptIn sFunctionButton(FUNCTION_BUTTON);
static mbed::InterruptIn sBleButton(BLE_BUTTON);
#endif

static mbed::Timeout sFunctionTimer;

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
    CapFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    CapFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
    CapBleButton.fall(mbed::callback(this, &AppTask::BleButtonPressEventHandler));
#else
    sFunctionButton.fall(mbed::callback(this, &AppTask::FunctionButtonPressEventHandler));
    sFunctionButton.rise(mbed::callback(this, &AppTask::FunctionButtonReleaseEventHandler));
    sBleButton.fall(mbed::callback(this, &AppTask::BleButtonPressEventHandler));
#endif

    ConnectivityMgrImpl().StartWiFiManagement();

    // Init ZCL Data Model and start server
    error = Server::GetInstance().Init();
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

#ifdef CHIP_OTA_REQUESTOR
    // Initialize the instance of the main Requestor Class
    OTARequestor * requestor = new OTARequestor();
    if (requestor == nullptr)
    {
        ChipLogError(NotSpecified, "Create OTA Requestor core failed");
        return EXIT_FAILURE;
    }
    SetRequestorInstance(requestor);

    // Initialize an instance of the Requestor Driver
    GenericOTARequestorDriver * requestorDriver = new GenericOTARequestorDriver;
    if (requestorDriver == nullptr)
    {
        ChipLogError(NotSpecified, "Create OTA Requestor driver failed");
        return EXIT_FAILURE;
    }

    // Initialize  the Downloader object
    BDXDownloader * downloader = new BDXDownloader();
    if (downloader == nullptr)
    {
        ChipLogError(NotSpecified, "Create OTA Downloader failed");
        return EXIT_FAILURE;
    }

    // Initialize the Image Processor object
    OTAImageProcessorImpl * imageProcessor = new OTAImageProcessorImpl;
    if (imageProcessor == nullptr)
    {
        ChipLogError(NotSpecified, "Create OTA Image Processor failed");
        return EXIT_FAILURE;
    }

    requestor->Init(&(chip::Server::GetInstance()), requestorDriver, downloader);
    imageProcessor->SetOTADownloader(downloader);
    downloader->SetImageProcessorDelegate(imageProcessor);
    requestorDriver->Init(requestor, imageProcessor);
#endif // CHIP_OTA_REQUESTOR

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

void AppTask::BleButtonPressEventHandler()
{
    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Pin    = BLE_BUTTON;
    button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
    button_event.Handler            = BleHandler;
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
    button_event.ButtonEvent.Pin    = id == 0 ? BLE_BUTTON : FUNCTION_BUTTON;
    button_event.ButtonEvent.Action = pushed ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;

    if (id == 0)
    {
        button_event.Handler = BleHandler;
    }
    else
    {
        button_event.Handler = FunctionHandler;
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    auto chronoTimeoutMs = std::chrono::duration<uint32_t, std::milli>(aTimeoutInMs);
    sFunctionTimer.attach(mbed::callback(this, &AppTask::TimerEventHandler), chronoTimeoutMs);
    mFunctionTimerActive = true;
}

void AppTask::CancelTimer()
{
    sFunctionTimer.detach();
    mFunctionTimerActive = false;
}

void AppTask::TimerEventHandler()
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

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

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
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

void AppTask::BleHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_Button)
    {
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
}
