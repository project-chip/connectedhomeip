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

#include "DeviceCallbacks.h"
#include "app/util/af-enums.h"
#include "app/util/af.h"
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <common/CHIPDeviceManager.h>
#include <common/Esp32AppServer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/ErrorStr.h>
#include <ota/OTAHelper.h>
#include <shell_extension/launch.h>

#include "OTAImageProcessorImpl.h"

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

using namespace ::chip;
using namespace ::chip::System;
using namespace ::chip::DeviceManager;
using namespace chip::Shell;
using namespace ::chip::Credentials;

namespace {
const char * TAG = "ota-requester-app";
static AppDeviceCallbacks EchoCallbacks;

constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

static void InitServer(intptr_t context)
{
    Esp32AppServer::Init(); // Init ZCL Data Model and CHIP App Server AND Initialize device attestation config

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
DeviceLayer::ESP32FactoryDataProvider sFactoryDataProvider;
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

void OTAEventsHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    if (event->Type == DeviceLayer::DeviceEventType::kOtaStateChanged)
    {
        switch (event->OtaStateChanged.newState)
        {
        case DeviceLayer::kOtaDownloadInProgress:
            ChipLogProgress(DeviceLayer, "OTA image download in progress");
            break;
        case DeviceLayer::kOtaDownloadComplete:
            ChipLogProgress(DeviceLayer, "OTA image download complete");
            break;
        case DeviceLayer::kOtaDownloadFailed:
            ChipLogProgress(DeviceLayer, "OTA image download failed");
            break;
        case DeviceLayer::kOtaDownloadAborted:
            ChipLogProgress(DeviceLayer, "OTA image download aborted");
            break;
        case DeviceLayer::kOtaApplyInProgress:
            ChipLogProgress(DeviceLayer, "OTA image apply in progress");
            break;
        case DeviceLayer::kOtaApplyComplete:
            ChipLogProgress(DeviceLayer, "OTA image apply complete");
            break;
        case DeviceLayer::kOtaApplyFailed:
            ChipLogProgress(DeviceLayer, "OTA image apply failed");
            break;
        default:
            break;
        }
    }
}

} // namespace

extern "C" void app_main()
{
#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

    ESP_LOGI(TAG, "OTA Requester!");

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

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
    OTARequestorCommands::GetInstance().Register();
#endif // CONFIG_ENABLE_CHIP_SHELL

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(OTAEventsHandler, reinterpret_cast<intptr_t>(nullptr));
}
