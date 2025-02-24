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
#include <DFUManager.h>

#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <lib/support/logging/CHIPLogging.h>
#include <static-supported-modes-manager.h>

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);

static bool sIsWiFiStationProvisioned = false;
static bool sIsWiFiStationEnabled     = false;
static bool sIsWiFiStationConnected   = false;
static bool sIsPairedToAccount        = false;
static bool sHaveBLEConnections       = false;

static events::EventQueue sAppEventQueue;

using namespace ::chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Credentials;

AppTask AppTask::sAppTask;
app::Clusters::ModeSelect::StaticSupportedModesManager sStaticSupportedModesManager;

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

    // Init ZCL Data Model and start server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    error                        = Server::GetInstance().Init(initParams);
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

    app::Clusters::ModeSelect::setSupportedModesManager(&sStaticSupportedModesManager);
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

    ChipLogProgress(NotSpecified, "Mbed all-clusters-minimal-app example application run");

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

        // Update the status LED
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
