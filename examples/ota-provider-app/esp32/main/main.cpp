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
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>

#include <OTAProviderCommands.h>
#include <app/clusters/ota-provider/ota-provider.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/OTAProviderExample.h>
#include <shell_extension/launch.h>

using chip::Callback::Callback;
using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::Credentials;
using namespace chip::DeviceManager;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset);
void OnTransferComplete(void * context);
void OnTransferFailed(void * context, BdxSenderErrorTypes status);

namespace {
const char * TAG               = "ota-provider-app";
const uint8_t kMaxImagePathlen = 35;
static DeviceCallbacks EchoCallbacks;

// TODO: this should probably be done dynamically
constexpr chip::EndpointId kOtaProviderEndpoint = 0;
const char * otaFilename                        = CONFIG_OTA_IMAGE_NAME;
FILE * otaImageFile                             = NULL;
uint32_t otaImageLen                            = 0;
uint32_t otaTransferInProgress                  = false;
static OTAProviderExample otaProvider;

chip::Callback::Callback<OnBdxBlockQuery> onBlockQueryCallback(OnBlockQuery, nullptr);
chip::Callback::Callback<OnBdxTransferComplete> onTransferCompleteCallback(OnTransferComplete, nullptr);
chip::Callback::Callback<OnBdxTransferFailed> onTransferFailedCallback(OnTransferFailed, nullptr);

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::ESPWiFiDriver::GetInstance()));

static void InitServer(intptr_t context)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    sWiFiNetworkCommissioningInstance.Init();

    BdxOtaSender * bdxOtaSender = otaProvider.GetBdxOtaSender();
    VerifyOrReturn(bdxOtaSender != nullptr, ESP_LOGE(TAG, "bdxOtaSender is nullptr"));

    // Register handler to handle bdx messages
    CHIP_ERROR error = chip::Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(
        chip::Protocols::BDX::Id, bdxOtaSender);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "RegisterUnsolicitedMessageHandler failed: %s", chip::ErrorStr(error));
        return;
    }

    BdxOtaSenderCallbacks callbacks;
    callbacks.onBlockQuery       = &onBlockQueryCallback;
    callbacks.onTransferComplete = &onTransferCompleteCallback;
    callbacks.onTransferFailed   = &onTransferFailedCallback;
    bdxOtaSender->SetCallbacks(callbacks);

    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path              = "/spiffs",
        .partition_label        = NULL,
        .max_files              = 3,
        .format_if_mount_failed = false,
    };

    esp_err_t err = esp_vfs_spiffs_register(&spiffs_conf);
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
        otaProvider.SetQueryImageStatus(OTAQueryStatus::kUpdateAvailable);
        otaProvider.SetOTAFilePath(otaImagePath);
    }

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &otaProvider);

    // Launch a chip shell and register OTA Provider Commands
    chip::LaunchShell();
    OTAProviderCommands & otaProviderCommands = OTAProviderCommands::GetInstance();
    otaProviderCommands.SetExampleOTAProvider(&otaProvider);
    otaProviderCommands.Register();
}

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

    BdxOtaSender * bdxOtaSender = otaProvider.GetBdxOtaSender();
    VerifyOrReturnError(bdxOtaSender != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint16_t blockBufAvailableLength = blockBuf->AvailableDataLength();
    uint16_t transferBlockSize       = bdxOtaSender->GetTransferBlockSize();

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
}
