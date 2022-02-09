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

#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "LEDWidget.h"

#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "shell_extension/launch.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/ESP32/OTAImageProcessorImpl.h>
#include <platform/GenericOTARequestorDriver.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

#if CONFIG_ENABLE_OTA_REQUESTOR
OTARequestor gRequestorCore;
GenericOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
#endif

LEDWidget AppLED;

static const char * TAG = "light-app";

static DeviceCallbacks EchoCallbacks;

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));
} // namespace

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

static void InitServer(intptr_t context)
{
    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    sWiFiNetworkCommissioningInstance.Init();
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

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "chip-esp32-light-example starting");
    ESP_LOGI(TAG, "==================================================");

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
#endif

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    AppLED.Init();

    InitOTARequestor();

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
}
