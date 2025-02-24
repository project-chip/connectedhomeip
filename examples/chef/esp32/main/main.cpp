/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "nvs_flash.h"

#include <lib/shell/Engine.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <ChipShellCollection.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>

#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/server/Dnssd.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#include "Display.h"
#include "QRCodeScreen.h"
#include "ScreenManager.h"

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif /* CONFIG_ENABLE_PW_RPC */

using namespace chip;
using namespace chip::Shell;
using chip::Shell::Engine;
using namespace chip::DeviceLayer;

#if CONFIG_ENABLE_CHIP_SHELL
static void chip_shell_task(void * args)
{

    cmd_misc_init();

    Engine::Root().RunMainLoop();
}
#endif /* CONFIG_ENABLE_CHIP_SHELL */

void DeviceEventCallback(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kInternetConnectivityChange:
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
        {
            ChipLogProgress(Shell, "IPv4 Server ready...");
            chip::app::DnssdServer::Instance().StartServer();
        }
        else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
        {
            ChipLogProgress(Shell, "Lost IPv4 connectivity...");
        }
        if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
        {
            ChipLogProgress(Shell, "IPv6 Server ready...");
            chip::app::DnssdServer::Instance().StartServer();
        }
        else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
        {
            ChipLogProgress(Shell, "Lost IPv6 connectivity...");
        }

        break;

    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ChipLogProgress(Shell, "CHIPoBLE connection established");
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ChipLogProgress(Shell, "CHIPoBLE disconnected");
        break;

    case DeviceEventType::kCommissioningComplete:
        ChipLogProgress(Shell, "Commissioning complete");
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // DNSSD server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. DNSSD still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
        break;
    }

    ChipLogProgress(Shell, "Current free heap: %u\n", static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT)));
}

extern const char TAG[] = "chef-app";

#if CONFIG_HAVE_DISPLAY
void printQRCode()
{
    // Create buffer for QR code that can fit max size and null terminator.
    char qrCodeBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan qrCodeText(qrCodeBuffer);

    GetQRCode(qrCodeText, chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    // Initialize the display device.
    esp_err_t err = InitDisplay();
    if (err != ESP_OK)
    {
        ChipLogError(Shell, "InitDisplay() failed: %s", esp_err_to_name(err));
        return;
    }

    // Initialize the screen manager
    ScreenManager::Init();

    ESP_LOGI(TAG, "Opening QR code screen");
    ESP_LOGI(TAG, "QR CODE Text: '%s'", qrCodeText.data());
    ScreenManager::PushScreen(chip::Platform::New<QRCodeScreen>(qrCodeText.data()));
}
#endif // CONFIG_HAVE_DISPLAY

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));

extern void ApplicationInit();

void InitServer(intptr_t)
{
    // Start IM server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);
    chip::Server::GetInstance().Init(initParams);

    // Device Attestation & Onboarding codes
    chip::Credentials::SetDeviceAttestationCredentialsProvider(chip::Credentials::Examples::GetExampleDACProvider());
    sWiFiNetworkCommissioningInstance.Init();
    chip::DeviceLayer::ConfigurationMgr().LogDeviceConfig();

    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        ChipLogError(Shell, "OpenBasicCommissioningWindow() failed");
    }

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(DeviceEventCallback, reinterpret_cast<intptr_t>(nullptr));

    // Application code should always be initialised after the initialisation of
    // server.
    ApplicationInit();
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    chip::Platform::MemoryInit();
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize the Wi-Fi stack");
        return;
    }
#endif
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::DeviceLayer::PlatformMgr().StartEventLoopTask();

#if CONFIG_ENABLE_CHIP_SHELL
    int ret = Engine::Root().Init();
    VerifyOrDie(ret == 0);
#endif /* CONFIG_ENABLE_CHIP_SHELL */

    // Network connectivity
    // Note to integration: StartWiFiManagement does not exist on ESP32

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer);
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));

#if CONFIG_HAVE_DISPLAY
    printQRCode();
#endif // CONFIG_HAVE_DISPLAY

#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif // CONFIG_ENABLE_PW_RPC

#if CONFIG_ENABLE_CHIP_SHELL
    xTaskCreate(&chip_shell_task, "chip_shell", 8192, NULL, 5, NULL);
#endif /* CONFIG_ENABLE_CHIP_SHELL */

    while (true)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
