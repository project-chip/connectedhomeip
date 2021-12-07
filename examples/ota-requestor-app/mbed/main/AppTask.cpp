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

#ifdef CHIP_OTA_REQUESTOR
#include "MbedOTARequestorDriver.h"
#include <MbedOTADownloader.h>
#include <MbedOTAImageProcessor.h>
#include <MbedOTARequestor.h>
#endif // CHIP_OTA_REQUESTOR

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

#ifdef CHIP_OTA_REQUESTOR
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    // Initialize the instance of the main Requestor Class
    MbedOTARequestor * requestorCore = new MbedOTARequestor(OnAnnounceProviderCallback, OnProviderResponseCallback);
    SetRequestorInstance(requestorCore);

    // Initialize an instance of the Requestor Driver
    MbedOTARequestorDriver * requestorUser = new MbedOTARequestorDriver;

    // Connect the Requestor and Requestor Driver objects
    requestorCore->SetOtaRequestorDriver(requestorUser);

    // Initialize  the Downloader object
    MbedOTADownloader * downloaderCore = new MbedOTADownloader(OnDownloadCompletedCallback);
    SetDownloaderInstance(downloaderCore);

    // Initialize the Image Processor object
    MbedOTAImageProcessor * downloaderUser = new MbedOTAImageProcessor;

    // Connect the Downloader and Image Processor objects
    downloaderCore->SetImageProcessorDelegate(downloaderUser);
    // Initialize and interconnect the Requestor and Image Processor objects -- END
#endif // CHIP_OTA_REQUESTOR

    ConnectivityMgrImpl().StartWiFiManagement();

    // Init ZCL Data Model and start server
    error = Server::GetInstance().Init();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Server initalization failed: %s", error.AsString());
        return EXIT_FAILURE;
    }

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    ConfigurationMgr().LogDeviceConfig();
    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

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

#ifdef CHIP_OTA_REQUESTOR
void AppTask::OnOtaEventHandler(AppEvent * aEvent)
{
    switch (aEvent->Type)
    {
    case AppEvent::kEventType_ota_provider_announce: {
        ChipLogProgress(NotSpecified, "OTA provider announce event");
        MbedOTARequestor * requestor = static_cast<MbedOTARequestor *>(GetRequestorInstance());
        requestor->ConnectProvider();
        break;
    }

    case AppEvent::kEventType_ota_provider_response: {
        ChipLogProgress(NotSpecified, "OTA provider response event");
        MbedOTADownloader * downloader = static_cast<MbedOTADownloader *>(GetDownloaderInstance());
        downloader->SetDownloadImageInfo(aEvent->OTAProviderResponseEvent.imageDatails->updateFileName);
        downloader->BeginPrepareDownload();
        break;
    }
    case AppEvent::kEventType_ota_download_completed:
        ChipLogProgress(NotSpecified, "OTA download completed event");
        ChipLogProgress(NotSpecified, "Download %.*s image size %ukB",
                        static_cast<int>(aEvent->OTADownloadCompletedEvent.imageInfo->imageName.size()),
                        aEvent->OTADownloadCompletedEvent.imageInfo->imageName.data(),
                        (static_cast<unsigned>(aEvent->OTADownloadCompletedEvent.imageInfo->imageSize) / 1024u));
        break;
    default:
        ChipLogError(NotSpecified, "OTA event unknown");
    }
}

void AppTask::OnAnnounceProviderCallback()
{
    AppEvent ota_announce_provider_event;
    ota_announce_provider_event.Type    = AppEvent::kEventType_ota_provider_announce;
    ota_announce_provider_event.Handler = OnOtaEventHandler;
    sAppTask.PostEvent(&ota_announce_provider_event);
}

void AppTask::OnProviderResponseCallback(MbedOTARequestor::OTAUpdateDetails * updateDetails)
{
    AppEvent ota_provider_response_event;
    ota_provider_response_event.Type                                  = AppEvent::kEventType_ota_provider_response;
    ota_provider_response_event.Handler                               = OnOtaEventHandler;
    ota_provider_response_event.OTAProviderResponseEvent.imageDatails = updateDetails;
    sAppTask.PostEvent(&ota_provider_response_event);
}

void AppTask::OnDownloadCompletedCallback(MbedOTADownloader::ImageInfo * imageInfo)
{
    AppEvent ota_download_completed_event;
    ota_download_completed_event.Type                                = AppEvent::kEventType_ota_download_completed;
    ota_download_completed_event.Handler                             = OnOtaEventHandler;
    ota_download_completed_event.OTADownloadCompletedEvent.imageInfo = imageInfo;
    sAppTask.PostEvent(&ota_download_completed_event);
}
#endif