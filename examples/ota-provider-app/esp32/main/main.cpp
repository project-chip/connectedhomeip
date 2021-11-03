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
#include "esp_heap_caps_init.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <app-common/zap-generated/callback.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/ErrorStr.h>

#include <app/clusters/ota-provider/ota-provider.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/OTAProviderExample.h>

using chip::BitFlags;
using chip::app::Clusters::OTAProviderDelegate;
using chip::bdx::TransferControlFlags;
using chip::Callback::Callback;
using chip::Messaging::ExchangeManager;
using namespace ::chip;
using namespace ::chip::System;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

const char * TAG = "ota-provider-app";

static DeviceCallbacks EchoCallbacks;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;

constexpr uint32_t kMaxBdxBlockSize                 = 1024;
constexpr chip::System::Clock::Timeout kBdxTimeout  = chip::System::Clock::Seconds16(5 * 60); // Specification mandates >= 5 minutes
constexpr chip::System::Clock::Timeout kBdxPollFreq = chip::System::Clock::Milliseconds32(500);
const char * gOtaFilename                           = "hello-world.bin";
BdxOtaSender bdxServer;

const esp_partition_t * otaPartition = nullptr;
uint32_t otaImageLen                 = 0;
uint32_t otaTransferInProgress       = false;

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset);
void OnTransferComplete(void * context);
void OnTransferFailed(void * context, BdxSenderErrorTypes status);

chip::Callback::Callback<OnBdxBlockQuery> mOnBlockQuery(OnBlockQuery, nullptr);
chip::Callback::Callback<OnBdxTransferComplete> mOnTransferComplete(OnTransferComplete, nullptr);
chip::Callback::Callback<OnBdxTransferFailed> mOnTransferFailed(OnTransferFailed, nullptr);

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset)
{
    if (otaTransferInProgress == false)
    {
        if (otaPartition == nullptr || otaImageLen == 0)
        {
            ESP_LOGE(TAG, "OTA partition not found");
            return CHIP_ERROR_OPEN_FAILED;
        }
        otaTransferInProgress = true;
    }

    uint16_t blockBufAvailableLength = blockBuf->AvailableDataLength();
    uint16_t transferBlockSize       = bdxServer.GetTransferBlockSize();

    size = (blockBufAvailableLength < transferBlockSize) ? blockBufAvailableLength : transferBlockSize;

    if (offset + size >= otaImageLen)
    {
        size  = otaImageLen - offset;
        isEof = true;
    }
    else
    {
        isEof = false;
    }

    esp_err_t err = esp_partition_read(otaPartition, offset + sizeof(otaImageLen), blockBuf->Start(), size);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Failed to read %d bytes from offset %d", size, offset + sizeof(otaImageLen));
        size  = 0;
        isEof = false;
        return CHIP_ERROR_READ_FAILED;
    }

    ESP_LOGI(TAG, "Read %d bytes from offset %d", size, offset + sizeof(otaImageLen));
    return CHIP_NO_ERROR;
}

void OnTransferComplete(void * context)
{
    ESP_LOGI(TAG, "OTA Image Transfer Complete");
    otaTransferInProgress = false;
}

void OnTransferFailed(void * context, BdxSenderErrorTypes status)
{
    ESP_LOGI(TAG, "OTA Image Transfer Failed, status:%x", status);
    otaTransferInProgress = false;
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "OTA Provider!");

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

    OTAProviderExample otaProvider;

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Register handler to handle bdx messages
    error = chip::Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(chip::Protocols::BDX::Id,
                                                                                                          &bdxServer);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "RegisterUnsolicitedMessageHandler failed: %s", chip::ErrorStr(error));
        return;
    }

    BdxOtaSenderCallbacks callbacks;
    callbacks.onBlockQuery       = &mOnBlockQuery;
    callbacks.onTransferComplete = &mOnTransferComplete;
    callbacks.onTransferFailed   = &mOnTransferFailed;
    bdxServer.SetCallbacks(callbacks);

    // If OTA image is available in flash storage then set to update available
    otaPartition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS, "ota_data");
    if (otaPartition != nullptr)
    {
        ESP_LOGI(TAG, "Partition found %s address:0x%x size:0x%x", otaPartition->label, otaPartition->address, otaPartition->size);

        // TODO: Use the OTA image header specified in the specification
        //       Right now we are using just image length instead of full header
        esp_partition_read(otaPartition, 0, &otaImageLen, sizeof(otaImageLen));
        if (otaImageLen > otaPartition->size)
        {
            otaImageLen = 0;
        }
        ESP_LOGI(TAG, "OTA image length %d bytes", otaImageLen);
    }
    else
    {
        ESP_LOGE(TAG, "OTA partition not found");
    }

    if (otaImageLen > 0)
    {
        otaProvider.SetQueryImageBehavior(OTAProviderExample::kRespondWithUpdateAvailable);
        otaProvider.SetOTAFilePath(gOtaFilename);
    }

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &otaProvider);

    BitFlags<TransferControlFlags> bdxFlags;
    bdxFlags.Set(TransferControlFlags::kReceiverDrive);
    error = bdxServer.PrepareForTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kSender, bdxFlags,
                                         kMaxBdxBlockSize, kBdxTimeout, kBdxPollFreq);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(BDX, "Failed to init BDX server: %s", chip::ErrorStr(error));
        return;
    }

    // Run the UI Loop
    while (true)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
