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
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include <app/server/Server.h>
#include <common/CHIPDeviceManager.h>
#include <common/Esp32AppServer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ESP32Utils.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <OTAProviderCommands.h>
#include <app/clusters/ota-provider/ota-provider.h>
#include <ota-provider-common/BdxOtaSender.h>
#include <ota-provider-common/OTAProviderExample.h>
#include <shell_extension/launch.h>

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

using chip::Callback::Callback;
using namespace chip;
using namespace chip::Shell;
using namespace chip::System;
using namespace chip::DeviceManager;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;
using namespace ::chip::Credentials;

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset);
void OnTransferComplete(void * context);
void OnTransferFailed(void * context, BdxSenderErrorTypes status);

namespace {
extern const char TAG[]        = "ota-provider-app";
const uint8_t kMaxImagePathlen = 35;
static AppDeviceCallbacks EchoCallbacks;

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

static void InitServer(intptr_t context)
{
    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    Esp32AppServer::Init(); // Init ZCL Data Model and CHIP App Server AND Initialize device attestation config

    BdxOtaSender * bdxOtaSender = otaProvider.GetBdxOtaSender();
    VerifyOrReturn(bdxOtaSender != nullptr, ESP_LOGE(TAG, "bdxOtaSender is nullptr"));

    // Register handler to handle bdx messages
    CHIP_ERROR error = chip::Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(
        chip::Protocols::BDX::Id, bdxOtaSender);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "RegisterUnsolicitedMessageHandler failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    BdxOtaSenderCallbacks callbacks;
    callbacks.onBlockQuery       = &onBlockQueryCallback;
    callbacks.onTransferComplete = &onTransferCompleteCallback;
    callbacks.onTransferFailed   = &onTransferFailedCallback;
    bdxOtaSender->SetCallbacks(callbacks);

    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path              = "/fs",
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
    ESP_LOGI(TAG, "Partition size: total: %u, used: %u", total, used);
    char otaImagePath[kMaxImagePathlen];
    memset(otaImagePath, 0, sizeof(otaImagePath));
    snprintf(otaImagePath, sizeof(otaImagePath), "/fs/%s", otaFilename);

    otaImageFile = fopen(otaImagePath, "r");
    if (otaImageFile == NULL)
    {
        ESP_LOGE(TAG, "Failed to open %s", otaFilename);
        return;
    }

    fseek(otaImageFile, 0, SEEK_END);
    otaImageLen = ftell(otaImageFile);
    rewind(otaImageFile);
    ESP_LOGI(TAG, "The OTA image size: %" PRIu32, otaImageLen);
    if (otaImageLen > 0)
    {
        otaProvider.SetQueryImageStatus(OTAQueryStatus::kUpdateAvailable);
        otaProvider.SetOTAFilePath(otaImagePath);
        otaProvider.SetApplyUpdateAction(OTAApplyUpdateAction::kProceed);
    }
    fclose(otaImageFile);
    otaImageFile = NULL;

    chip::app::Clusters::OTAProvider::SetDelegate(kOtaProviderEndpoint, &otaProvider);

    // Launch a chip shell and register OTA Provider Commands
    chip::LaunchShell();
    OTAProviderCommands & otaProviderCommands = OTAProviderCommands::GetInstance();
    otaProviderCommands.SetExampleOTAProvider(&otaProvider);
    otaProviderCommands.Register();
}

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
DeviceLayer::ESP32FactoryDataProvider sFactoryDataProvider;
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
} // namespace

CHIP_ERROR OnBlockQuery(void * context, chip::System::PacketBufferHandle & blockBuf, size_t & size, bool & isEof, uint32_t offset)
{
    BdxOtaSender * bdxOtaSender = otaProvider.GetBdxOtaSender();
    VerifyOrReturnError(bdxOtaSender != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (otaTransferInProgress == false)
    {
        const char * fileDesignator = bdxOtaSender->GetFileDesignator();
        if (fileDesignator == nullptr || fileDesignator[0] == 0)
        {
            ESP_LOGE(TAG, "File designator is null");
            return CHIP_ERROR_INCORRECT_STATE;
        }

        char otaImagePath[kMaxImagePathlen];
        memset(otaImagePath, 0, sizeof(otaImagePath));
        snprintf(otaImagePath, sizeof(otaImagePath), "%s", fileDesignator);
        ESP_LOGI(TAG, "File designator: %s", otaImagePath);

        otaImageFile = fopen(otaImagePath, "r");
        if (otaImageFile == NULL)

        {
            ESP_LOGE(TAG, "Failed to open the OTA image file");
            return CHIP_ERROR_OPEN_FAILED;
        }
        otaTransferInProgress = true;
    }

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
        ESP_LOGE(TAG, "Failed to read %u bytes from %s", size, otaFilename);
        size  = 0;
        isEof = false;
        return CHIP_ERROR_READ_FAILED;
    }
    ESP_LOGI(TAG, "Read %u bytes from %s", size, otaFilename);
    return CHIP_NO_ERROR;
}

void OnTransferComplete(void * context)
{
    ESP_LOGI(TAG, "OTA Image Transfer Complete");
    otaTransferInProgress = false;
    if (otaImageFile)
    {
        fclose(otaImageFile);
        otaImageFile = NULL;
    }
}

void OnTransferFailed(void * context, BdxSenderErrorTypes status)
{
    ESP_LOGI(TAG, "OTA Image Transfer Failed, status:%x", status);
    otaTransferInProgress = false;
    if (otaImageFile)
    {
        fclose(otaImageFile);
        otaImageFile = NULL;
    }
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
    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_create_default() failed: %s", esp_err_to_name(err));
        return;
    }
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi stack");
        return;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    CHIP_ERROR error = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %" CHIP_ERROR_FORMAT, error.Format());
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
}
