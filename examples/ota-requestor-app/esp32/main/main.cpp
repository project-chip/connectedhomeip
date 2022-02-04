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
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <app/server/Server.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/ErrorStr.h>

#include "OTAImageProcessorImpl.h"
#include "platform/GenericOTARequestorDriver.h"
#include "platform/OTARequestorInterface.h"
#include <platform/ESP32/OTARequestorNVSHelper.h>

using namespace ::chip;
using namespace ::chip::System;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

namespace {
const char * TAG = "ota-requester-app";
static DeviceCallbacks EchoCallbacks;

OTARequestor gRequestorCore;
GenericOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));

static void InitServer(intptr_t context)
{
    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    sWiFiNetworkCommissioningInstance.Init();
}

} // namespace

// Test mode operation
void NotifyUpdateAppliedHandler(Layer * systemLayer, void * appState)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(appState);
    uint32_t softwareVersion;
    NodeId nodeId;
    FabricIndex fabIndex;
    EndpointId endpointId;
    esp_err_t err = OTARequestorNVSHelper::ReadProviderLocation(nodeId, fabIndex, endpointId);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read provider location: %s", esp_err_to_name(err));
        return;
    }
    // TODO : switch this over once persistent storage in the OTA Requestor core
    requestorCore->TestModeSetProviderParameters(nodeId, fabIndex, endpointId);
    DeviceLayer::ConfigurationMgr().GetSoftwareVersion(softwareVersion);
    requestorCore->NotifyUpdateApplied(softwareVersion);
    err = OTARequestorNVSHelper::ClearAllData();

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to clear NVS data: %s", esp_err_to_name(err));
        return;
    }
}

extern "C" void app_main()
{
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

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    SetRequestorInstance(&gRequestorCore);
    gRequestorCore.Init(&(Server::GetInstance()), &gRequestorUser, &gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    uint32_t prevSoftwareVersion, currSoftwareVersion;

    err = OTARequestorNVSHelper::ReadSoftwareVersion(prevSoftwareVersion);
    DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currSoftwareVersion);
    if (err == ESP_OK && currSoftwareVersion > prevSoftwareVersion)
    {
        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(5 * 1000), NotifyUpdateAppliedHandler,
                                                    &gRequestorCore);
    }
}
