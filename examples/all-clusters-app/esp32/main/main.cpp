/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "BluetoothWidget.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Globals.h"
#include "LEDWidget.h"
#include "OpenThreadLaunch.h"
#include "QRCodeScreen.h"
#include "ShellCommands.h"
#include "WiFiWidget.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "platform/PlatformManager.h"
#include "shell_extension/launch.h"

#include <cmath>
#include <cstdio>
#include <ctype.h>
#include <string>
#include <vector>

#include <app-common/zap-generated/att-storage.h>
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/on-off-server/on-off-server.h>
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <app/server/AppDelegate.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <binding-handler.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/shell/Engine.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ErrorStr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/ESP32/OTAImageProcessorImpl.h>
#include <platform/GenericOTARequestorDriver.h>
#include <setup_payload/ManualSetupPayloadGenerator.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>

#if CONFIG_HAVE_DISPLAY
#include "DeviceWithDisplay.h"
#endif

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

#if CONFIG_OPENTHREAD_ENABLED
#include <platform/ThreadStackManager.h>
#endif

using namespace ::chip;
using namespace ::chip::Shell;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

#define STATUS_LED_GPIO_NUM ((gpio_num_t) CONFIG_STATUS_LED_GPIO_NUM)

// Used to indicate that an IP address has been added to the QRCode
#define EXAMPLE_VENDOR_TAG_IP 1

const char * TAG = "all-clusters-app";

static DeviceCallbacks EchoCallbacks;

namespace {

#if CONFIG_ENABLE_OTA_REQUESTOR
    OTARequestor gRequestorCore;
    GenericOTARequestorDriver gRequestorUser;
    BDXDownloader gDownloader;
    OTAImageProcessorImpl gImageProcessor;
#endif

    namespace {
        app::Clusters::NetworkCommissioning::Instance
            sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));
    } // namespace

    class AppCallbacks : public AppDelegate
    {
        public:
            void OnRendezvousStarted() override { bluetoothLED.Set(true); }
            void OnRendezvousStopped() override
            {
                bluetoothLED.Set(false);
                pairingWindowLED.Set(false);
            }
            void OnPairingWindowOpened() override { pairingWindowLED.Set(true); }
            void OnPairingWindowClosed() override { pairingWindowLED.Set(false); }
    };

    AppCallbacks sCallbacks;

    constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

} // namespace

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    chip::Server::GetInstance().Init(&sCallbacks);

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    sWiFiNetworkCommissioningInstance.Init();
    InitBindingHandlers();

#if CONFIG_DEVICE_TYPE_M5STACK
    SetupPretendDevices();
#endif
}

static void InitOTARequestor(void)
{
#if CONFIG_ENABLE_OTA_REQUESTOR
    SetRequestorInstance(&gRequestorCore);
    gRequestorCore.Init(&Server::GetInstance(), &gRequestorUser, &gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);
#endif
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "All Clusters Demo!");
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "This is ESP32 chip with %d CPU cores, WiFi%s%s, ", chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    ESP_LOGI(TAG, "silicon revision %d, ", chip_info.revision);

    ESP_LOGI(TAG, "%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }
#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
    OnOffCommands::GetInstance().Register();
    CASECommands::GetInstance().Register();
#endif // CONFIG_ENABLE_CHIP_SHELL

#if CONFIG_OPENTHREAD_ENABLED
    LaunchOpenThread();
    ThreadStackMgr().InitThreadStack();
#endif

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    statusLED1.Init(STATUS_LED_GPIO_NUM);
    // Our second LED doesn't map to any physical LEDs so far, just to virtual
    // "LED"s on devices with screens.
    statusLED2.Init(GPIO_NUM_MAX);
    bluetoothLED.Init();
    wifiLED.Init();
    pairingWindowLED.Init();

    InitOTARequestor();

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
#if CONFIG_HAVE_DISPLAY
    InitDeviceDisplay();
#endif
#if CONFIG_DEVICE_TYPE_M5STACK
    PollButtons();
#endif
}

bool lowPowerClusterSleep()
{
    return true;
}
