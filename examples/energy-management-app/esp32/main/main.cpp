/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
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
#include <EnergyManagementAppCommonMain.h>
#include <common/CHIPDeviceManager.h>
#include <common/Esp32AppServer.h>
#include <common/Esp32ThreadInit.h>
#if CONFIG_ENABLE_SNTP_TIME_SYNC
#include <time/TimeSync.h>
#endif
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "spi_flash_mmap.h"
#else
#include "esp_spi_flash.h"
#endif
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "shell_extension/launch.h"
#include "shell_extension/openthread_cli_register.h"
#include <EnergyManagementAppCmdLineOptions.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/endpoint-config-api.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/ESP32/ESP32Utils.h>

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_PW_RPC
#include "Rpc.h"
#endif

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

#if CONFIG_SEC_CERT_DAC_PROVIDER
#include <platform/ESP32/ESP32SecureCertDACProvider.h>
#endif

#if CONFIG_ENABLE_ESP_INSIGHTS_TRACE
#include <esp_insights.h>
#include <tracing/esp32_trace/esp32_tracing.h>
#include <tracing/registry.h>
#endif

using namespace ::chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement;

#if CONFIG_ENABLE_ESP_INSIGHTS_TRACE
extern const char insights_auth_key_start[] asm("_binary_insights_auth_key_txt_start");
extern const char insights_auth_key_end[] asm("_binary_insights_auth_key_txt_end");
#endif

static const char * TAG = "energy-management-app";

static AppDeviceCallbacks EchoCallbacks;
static DeviceCallbacksDelegate sAppDeviceCallbacksDelegate;

namespace {

constexpr chip::EndpointId kEvseEndpoint        = 1;
constexpr chip::EndpointId kWaterHeaterEndpoint = 2;

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
DeviceLayer::ESP32FactoryDataProvider sFactoryDataProvider;
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

#if CONFIG_SEC_CERT_DAC_PROVIDER
DeviceLayer::ESP32SecureCertDACProvider gSecureCertDACProvider;
#endif // CONFIG_SEC_CERT_DAC_PROVIDER

chip::Credentials::DeviceAttestationCredentialsProvider * get_dac_provider(void)
{
#if CONFIG_SEC_CERT_DAC_PROVIDER
    return &gSecureCertDACProvider;
#elif CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    return &sFactoryDataProvider;
#else // EXAMPLE_DAC_PROVIDER
    return chip::Credentials::Examples::GetExampleDACProvider();
#endif
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

// Keep track of the parsed featureMap option
#if defined(CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING) && defined(CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING)
#error Cannot define CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING and CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING
#endif

#ifdef CONFIG_DEM_SUPPORT_POWER_FORECAST_REPORTING
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kPowerForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);
#elif CONFIG_DEM_SUPPORT_STATE_FORECAST_REPORTING
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment, Feature::kStateForecastReporting,
                                          Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                          Feature::kConstraintBasedAdjustment);
#else
static chip::BitMask<Feature> sFeatureMap(Feature::kPowerAdjustment);
#endif

chip::BitMask<Feature> GetFeatureMapFromCmdLine()
{
    return sFeatureMap;
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

// Check we are not trying to build in both app types simultaneously
#if defined(CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE) && defined(CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE)
#error Cannot define CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE and CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
#endif

EndpointId GetEnergyDeviceEndpointId()
{
#if defined(CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE)
    return kWaterHeaterEndpoint;
#else
    return kEvseEndpoint;
#endif
}

void ApplicationInit()
{
    ESP_LOGD(TAG, "Energy Management App: ApplicationInit()");
#if CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE

    EvseApplicationInit();
    // Disable Water Heater Endpoint
    emberAfEndpointEnableDisable(kWaterHeaterEndpoint, false);
#endif // CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE

#if CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    WaterHeaterApplicationInit();
    // Disable EVSE Endpoint
    emberAfEndpointEnableDisable(kEvseEndpoint, false);
#endif // CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
}

void ApplicationShutdown()
{
    ESP_LOGD(TAG, "Energy Management App: ApplicationShutdown()");

#if CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE
    EvseApplicationShutdown();
#endif // CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE

#if CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
    WaterHeaterApplicationShutdown();
#endif // CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE
}

static void InitServer(intptr_t context)
{
    // Print QR Code URL
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

    DeviceCallbacksDelegate::Instance().SetAppDelegate(&sAppDeviceCallbacksDelegate);
    Esp32AppServer::Init(); // Init ZCL Data Model and CHIP App Server AND
                            // Initialize device attestation config
#if CONFIG_ENABLE_ESP_INSIGHTS_TRACE
    esp_insights_config_t config = {
        .log_type = ESP_DIAG_LOG_TYPE_ERROR | ESP_DIAG_LOG_TYPE_WARNING | ESP_DIAG_LOG_TYPE_EVENT,
        .auth_key = insights_auth_key_start,
    };

    esp_err_t ret = esp_insights_init(&config);

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ESP Insights, err:0x%x", ret);
    }

    static Tracing::Insights::ESP32Backend backend;
    Tracing::Register(backend);
#endif

    // Application code should always be initialised after the initialisation of
    // server.
    ApplicationInit();

#if CONFIG_ENABLE_SNTP_TIME_SYNC
    const char kNtpServerUrl[]             = "pool.ntp.org";
    const uint16_t kSyncNtpTimeIntervalDay = 1;
    chip::Esp32TimeSync::Init(kNtpServerUrl, kSyncNtpTimeIntervalDay);
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
    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_event_loop_create_default() failed: %s", esp_err_to_name(err));
        return;
    }
#if CONFIG_ENABLE_PW_RPC
    chip::rpc::Init();
#endif

    ESP_LOGI(TAG, "==================================================");
#if defined(CONFIG_ENABLE_EXAMPLE_EVSE_DEVICE)
    ESP_LOGI(TAG, "chip-esp32-energy-management-example evse starting. featureMap 0x%08lx",
             DeviceEnergyManagement::sFeatureMap.Raw());
#elif defined(CONFIG_ENABLE_EXAMPLE_WATER_HEATER_DEVICE)
    ESP_LOGI(TAG, "chip-esp32-energy-management-example water-heater starting. featureMap 0x%08lx",
             DeviceEnergyManagement::sFeatureMap.Raw());
#else
    ESP_LOGI(TAG, "chip-esp32-energy-management-example starting. featureMap 0x%08lx", DeviceEnergyManagement::sFeatureMap.Raw());
#endif
    ESP_LOGI(TAG, "==================================================");

#if CONFIG_ENABLE_CHIP_SHELL
#if CONFIG_OPENTHREAD_CLI
    chip::RegisterOpenThreadCliCommands();
#endif
    chip::LaunchShell();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize WiFi stack");
        return;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();
    CHIP_ERROR error              = deviceMgr.Init(&EchoCallbacks);
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "device.Init() failed: %s", ErrorStr(error));
        return;
    }

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
#endif

    SetDeviceAttestationCredentialsProvider(get_dac_provider());

    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
}
