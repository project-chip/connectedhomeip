/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "nvs_flash.h"
#include "platform/PlatformManager.h"
#include "shell_extension/launch.h"

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/af.h>
#include <binding-handler.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>

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

// Used to indicate that an IP address has been added to the QRCode
#define EXAMPLE_VENDOR_TAG_IP 1

const char * TAG = "all-clusters-app";

static DeviceCallbacks EchoCallbacks;

namespace {

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));

class AppCallbacks : public AppDelegate
{
public:
    void OnCommissioningSessionStarted() override { bluetoothLED.Set(true); }
    void OnCommissioningSessionStopped() override
    {
        bluetoothLED.Set(false);
        pairingWindowLED.Set(false);
    }
    void OnCommissioningWindowOpened() override { pairingWindowLED.Set(true); }
    void OnCommissioningWindowClosed() override { pairingWindowLED.Set(false); }
};

AppCallbacks sCallbacks;

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

} // namespace

static void InitServer(intptr_t context)
{
    // Init ZCL Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.appDelegate = &sCallbacks;
    chip::Server::GetInstance().Init(initParams);

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

extern "C" void app_main()
{
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

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "chip-esp32-all-cluster-demo starting");
    ESP_LOGI(TAG, "==================================================");

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
    CHIP_ERROR error              = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    ESP_LOGI(TAG, "------------------------Starting App Task---------------------------");
    error = GetAppTask().StartAppTask();
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "GetAppTask().StartAppTask() failed : %s", ErrorStr(error));
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
}

bool lowPowerClusterSleep()
{
    return true;
}
