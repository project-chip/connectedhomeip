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
#include "events/EventQueue.h"

#ifdef CAPSENSE_ENABLED
#include "capsense.h"
#endif

#ifdef CHIP_OTA_REQUESTOR
#include "OTARequestorDriverImpl.h"
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

#define CONFIRM_BUTTON (MBED_CONF_APP_CONFIRM_BUTTON)
#define REJECT_BUTTON (MBED_CONF_APP_REJECT_BUTTON)

#ifdef CAPSENSE_ENABLED
static mbed::CapsenseButton CapConfirmButton(Capsense::getInstance(), 0);
static mbed::CapsenseButton CapRejectButton(Capsense::getInstance(), 1);
#else
static mbed::InterruptIn sConfirmButton(CONFIRM_BUTTON);
static mbed::InterruptIn sRejectButton(REJECT_BUTTON);
#endif

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

#ifdef CAPSENSE_ENABLED
    Capsense::getInstance().init();
    CapConfirmButton.fall(mbed::callback(this, &AppTask::OnConfirmButtonPressEventHandler));
    CapRejectButton.fall(mbed::callback(this, &AppTask::OnRejectButtonPressEventHandler));
#else
    sConfirmButton.fall(mbed::callback(this, &AppTask::OnConfirmButtonPressEventHandler));
    sRejectButton.fall(mbed::callback(this, &AppTask::OnRejectButtonPressEventHandler));
#endif

#ifdef MBED_CONF_APP_BLE_DEVICE_NAME
    error = ConnectivityMgr().SetBLEDeviceName(MBED_CONF_APP_BLE_DEVICE_NAME);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Set BLE device name failed: %s", error.AsString());
        return EXIT_FAILURE;
    }
#endif

    // Start BLE advertising if needed
    if (!CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART)
    {
        ChipLogProgress(NotSpecified, "Enabling BLE advertising.");
        error = ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Set BLE advertising enabled failed: %s", error.AsString());
            return EXIT_FAILURE;
        }
    }

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

    Server * server = &(Server::GetInstance());
    if (server == nullptr)
    {
        ChipLogError(NotSpecified, "Get server instance failed");
        return EXIT_FAILURE;
    }
    requestor->SetServerInstance(server);

    // Initialize an instance of the Requestor Driver
    OTARequestorDriverImpl * requestorDriver = new OTARequestorDriverImpl;
    if (requestorDriver == nullptr)
    {
        ChipLogError(NotSpecified, "Create OTA Requestor driver failed");
        return EXIT_FAILURE;
    }

    // Connect the Requestor and Requestor Driver objects
    requestor->SetOtaRequestorDriver(requestorDriver);

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

    imageProcessor->SetOTADownloader(downloader);
    downloader->SetImageProcessorDelegate(imageProcessor);

    requestor->SetBDXDownloader(downloader);
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

void AppTask::OnConfirmButtonPressEventHandler()
{
    ChipLogProgress(NotSpecified, "Confirm button pressed");
    mButtonEventFlag.set(AppTask::kUserResponseType_confirm);
}

void AppTask::OnRejectButtonPressEventHandler()
{
    ChipLogProgress(NotSpecified, "Reject button pressed");
    mButtonEventFlag.set(AppTask::kUserResponseType_reject);
}

AppTask::AppUserResponse AppTask::GetUserResponse(AppEvent::AppEventTypes event)
{
    switch (event)
    {
    case AppEvent::kEventType_ota_update_available: {
        ChipLogProgress(NotSpecified, "OTA update available");
        ChipLogProgress(NotSpecified, "Press BUTTON 0 to confirm or BUTTON 1 to reject update image downloading");
        break;
    }
    case AppEvent::kEventType_ota_apply_download: {
        ChipLogProgress(NotSpecified, "OTA apply download");
        ChipLogProgress(NotSpecified, "Press BUTTON 0 to confirm or BUTTON 1 to reject update image applying");
        break;
    }
    default:
        ChipLogError(NotSpecified, "OTA event unknown");
        return AppTask::kUserResponseType_none;
    }

    return (AppTask::AppUserResponse) mButtonEventFlag.wait_any(AppTask::kUserResponseType_confirm |
                                                                AppTask::kUserResponseType_reject);
}
