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
#include "esp_spiffs.h"
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

#include <BdxOtaSender.h>
#include <app/clusters/ota-provider/ota-provider.h>
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

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset);
void OnTransferComplete(void * context);
void OnTransferFailed(void * context, BdxSenderErrorTypes status);

namespace {
const char * TAG               = "ota-provider-app";
const uint8_t kMaxImagePathlen = 35;
static DeviceCallbacks EchoCallbacks;
BdxOtaSender bdxServer;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint     = 0;
constexpr uint32_t kMaxBdxBlockSize                 = 1024;
constexpr chip::System::Clock::Timeout kBdxTimeout  = chip::System::Clock::Seconds16(5 * 60); // Specification mandates >= 5 minutes
constexpr chip::System::Clock::Timeout kBdxPollFreq = chip::System::Clock::Milliseconds32(500);
const char * otaFilename                            = CONFIG_OTA_IMAGE_NAME;
FILE * otaImageFile                                 = NULL;
uint32_t otaImageLen                                = 0;
uint32_t otaTransferInProgress                      = false;
static OTAProviderExample otaProvider;

chip::Callback::Callback<OnBdxBlockQuery> onBlockQueryCallback(OnBlockQuery, nullptr);
chip::Callback::Callback<OnBdxTransferComplete> onTransferCompleteCallback(OnTransferComplete, nullptr);
chip::Callback::Callback<OnBdxTransferFailed> onTransferFailedCallback(OnTransferFailed, nullptr);
} // namespace

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset)
{
    if (otaTransferInProgress == false)
    {
        if (otaImageFile == NULL || otaImageLen == 0)
        {
            ESP_LOGE(TAG, "Failed to open the OTA image file");
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
    size_t size_read = fread(blockBuf->Start(), 1, size, otaImageFile);
    if (size_read != size)
    {
        ESP_LOGE(TAG, "Failed to read %d bytes from %s", size, otaFilename);
        size  = 0;
        isEof = false;
        return CHIP_ERROR_READ_FAILED;
    }
    ESP_LOGI(TAG, "Read %d bytes from %s", size, otaFilename);
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
    callbacks.onBlockQuery       = &onBlockQueryCallback;
    callbacks.onTransferComplete = &onTransferCompleteCallback;
    callbacks.onTransferFailed   = &onTransferFailedCallback;
    bdxServer.SetCallbacks(callbacks);

    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path              = "/spiffs",
        .partition_label        = NULL,
        .max_files              = 3,
        .format_if_mount_failed = false,
    };

    err = esp_vfs_spiffs_register(&spiffs_conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(err));
        return;
    }
    size_t total = 0, used = 0;
    err = esp_spiffs_info(NULL, &total, &used);
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    char otaImagePath[kMaxImagePathlen];
    sprintf(otaImagePath, "/spiffs/%s", otaFilename);
    otaImageFile = fopen(otaImagePath, "r");
    if (otaImageFile == NULL)
    {
        ESP_LOGE(TAG, "Failed to open %s", otaFilename);
        return;
    }
    fseek(otaImageFile, 0, SEEK_END);
    otaImageLen = ftell(otaImageFile);
    rewind(otaImageFile);
    ESP_LOGI(TAG, "The OTA image size: %d", otaImageLen);
    if (otaImageLen > 0)
    {
        otaProvider.SetQueryImageBehavior(OTAProviderExample::kRespondWithUpdateAvailable);
        otaProvider.SetOTAFilePath(otaFilename);
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
}
