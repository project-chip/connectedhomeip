/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <devices/device-factory/DeviceFactory.h>
#include <devices/root-node/RootNodeDevice.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <memory>
#include <string>

#if CONFIG_ENABLE_CHIP_SHELL
#include <DeviceShellCommands.h>
#include <lib/shell/commands/WiFi.h>
#include <shell_extension/launch.h>
#endif

#ifdef CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING
#include <platform/internal/BLEManager.h>
#endif

#if CONFIG_ENABLE_OTA_REQUESTOR
#include <ota/OTAHelper.h>
#endif

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
#include <platform/ESP32/ESP32FactoryDataProvider.h>
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::Credentials;
using chip::DeviceLayer::Internal::ESP32Config;

static const char TAG[] = "all-devices-app";

// NVS key for storing the device type across reboots
static const ESP32Config::Key kConfigKey_DeviceType{ ESP32Config::kConfigNamespace_ChipConfig, "dev-type" };

static std::string gDeviceType = "contact-sensor";

static const size_t kMaxDeviceTypeLength = 64;

namespace {

// Use the singleton - platform event handlers report to GetInstance()
DeviceLayer::NetworkCommissioning::ESPWiFiDriver & sWiFiDriver = DeviceLayer::NetworkCommissioning::ESPWiFiDriver::GetInstance();

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
DeviceLayer::ESP32FactoryDataProvider sFactoryDataProvider;
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

chip::app::DefaultAttributePersistenceProvider gAttributePersistenceProvider;
Credentials::GroupDataProviderImpl gGropupDataProvider;
chip::app::CodeDrivenDataModelProvider * gDataModelProvider = nullptr;
std::unique_ptr<WifiRootNodeDevice> gRootNodeDevice;
std::unique_ptr<DeviceInterface> gConstructedDevice;

void DeInitBLEIfCommissioned()
{
#ifdef CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING
    static bool bleAlreadyShutdown = false;
    if (chip::Server::GetInstance().GetFabricTable().FabricCount() > 0 && !bleAlreadyShutdown)
    {
        bleAlreadyShutdown = true;
        chip::DeviceLayer::Internal::BLEMgr().Shutdown();
    }
#endif
}

void DeviceEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE is deinitialized");
        break;

    case DeviceLayer::DeviceEventType::kInternetConnectivityChange:
        if (event->InternetConnectivityChange.IPv4 == DeviceLayer::kConnectivity_Established)
        {
            ESP_LOGI(TAG, "IPv4 Server ready...");
            chip::app::DnssdServer::Instance().StartServer();
        }
        else if (event->InternetConnectivityChange.IPv4 == DeviceLayer::kConnectivity_Lost)
        {
            ESP_LOGE(TAG, "Lost IPv4 connectivity...");
        }
        if (event->InternetConnectivityChange.IPv6 == DeviceLayer::kConnectivity_Established)
        {
            ESP_LOGI(TAG, "IPv6 Server ready...");
            chip::app::DnssdServer::Instance().StartServer();
        }
        else if (event->InternetConnectivityChange.IPv6 == DeviceLayer::kConnectivity_Lost)
        {
            ESP_LOGE(TAG, "Lost IPv6 connectivity...");
        }
        break;

    case DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished:
        ESP_LOGI(TAG, "CHIPoBLE connection established");
        break;

    case DeviceLayer::DeviceEventType::kCHIPoBLEConnectionClosed:
        ESP_LOGI(TAG, "CHIPoBLE disconnected");
        break;

    case DeviceLayer::DeviceEventType::kDnssdInitialized:
#if CONFIG_ENABLE_OTA_REQUESTOR
        OTAHelpers::Instance().InitOTARequestor();
#endif
        break;

    case DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        DeInitBLEIfCommissioned();
        break;

    case DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == DeviceLayer::InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == DeviceLayer::InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }
        break;

    default:
        break;
    }

    ESP_LOGI(TAG, "Current free heap: Internal: %u/%u External: %u/%u",
             static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL)),
             static_cast<unsigned int>(heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL)),
             static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM)),
             static_cast<unsigned int>(heap_caps_get_total_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM)));
}

chip::app::DataModel::Provider * PopulateCodeDrivenDataModelProvider(PersistentStorageDelegate * delegate)
{
    // Initialize the attribute persistence provider with the storage delegate
    CHIP_ERROR err = gAttributePersistenceProvider.Init(delegate);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to init attribute persistence provider: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    static chip::app::CodeDrivenDataModelProvider dataModelProvider =
        chip::app::CodeDrivenDataModelProvider(*delegate, gAttributePersistenceProvider);

    gDataModelProvider = &dataModelProvider;

    gRootNodeDevice = std::make_unique<WifiRootNodeDevice>(
        {
            .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(),
            .configurationManager       = DeviceLayer::ConfigurationMgr(),
            .deviceControlServer        = DeviceLayer::DeviceControlServer::DeviceControlSvr(),
            .fabricTable = Server::GetInstance().GetFabricTable(), .failsafeContext = Server::GetInstance().GetFailSafeContext(),
            .platformManager = DeviceLayer::PlatformMgr(), .gGropupDataProvider = gGropupDataProvider,
            .sessionManager = Server::GetInstance().GetSecureSessionManager(), .dnssdServer = DnssdServer::Instance(),

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        },
        {
            .wifiDriver = sWiFiDriver,
        });
    err = gRootNodeDevice->Register(kRootEndpointId, dataModelProvider, kInvalidEndpointId);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to register root node device: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    // Default to contact-sensor device type
    const char * deviceType = gDeviceType.c_str();
    gConstructedDevice      = DeviceFactory::GetInstance().Create(deviceType);
    if (gConstructedDevice == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create device of type: %s", deviceType);
        return nullptr;
    }

    err = gConstructedDevice->Register(CONFIG_ALL_DEVICES_ENDPOINT, dataModelProvider, kInvalidEndpointId);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to register device: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    return &dataModelProvider;
}

void InitServer(intptr_t context)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    CHIP_ERROR err = initParams.InitializeStaticResourcesBeforeServerInit();
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "InitializeStaticResourcesBeforeServerInit() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    initParams.dataModelProvider             = PopulateCodeDrivenDataModelProvider(initParams.persistentStorageDelegate);
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    if (initParams.dataModelProvider == nullptr)
    {
        ESP_LOGE(TAG, "Failed to populate data model provider");
        return;
    }

    // Save device type to NVS only after successful device creation
    err = ESP32Config::WriteConfigValueStr(kConfigKey_DeviceType, gDeviceType.c_str());
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGW(TAG, "Failed to save device type to NVS: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        ESP_LOGI(TAG, "Device type '%s' saved to NVS", gDeviceType.c_str());
    }

    err = Server::GetInstance().Init(initParams);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Server init failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI && CONFIG_ENABLE_CHIP_SHELL
    chip::Shell::SetWiFiDriver(&sWiFiDriver);
#endif
}

} // namespace

void InitServerWithDeviceType(std::string deviceType)
{
    // Set the device type (store the actual string, not a pointer to temporary)
    gDeviceType = std::move(deviceType);

    // Init the server
    PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));
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

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "chip-esp32-all-devices-app starting");
    ESP_LOGI(TAG, "==================================================");

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to initialize WiFi stack");
        return;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Platform::MemoryInit() failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    error = PlatformMgr().InitChipStack();
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr().InitChipStack() failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Register event handler to restart DNS-SD on connectivity change
    PlatformMgr().AddEventHandler(DeviceEventHandler, 0);

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

    error = PlatformMgr().StartEventLoopTask();
    if (error != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "PlatformMgr().StartEventLoopTask() failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Check if device type is stored in NVS from a previous boot
    char storedDeviceType[kMaxDeviceTypeLength] = { 0 };
    size_t storedLen                            = 0;
    CHIP_ERROR nvsErr =
        ESP32Config::ReadConfigValueStr(kConfigKey_DeviceType, storedDeviceType, sizeof(storedDeviceType), storedLen);

    if (nvsErr == CHIP_NO_ERROR && storedLen > 0)
    {
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "Found stored device type: %s", storedDeviceType);
        ESP_LOGI(TAG, "Auto-initializing...");
        ESP_LOGI(TAG, "==================================================");
        InitServerWithDeviceType(std::string(storedDeviceType));
    }
    else
    {
#if CONFIG_ENABLE_CHIP_SHELL
        chip::LaunchShell();
        chip::Shell::DeviceCommands::GetInstance().Register();
#endif // CONFIG_ENABLE_CHIP_SHELL
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "No stored device type found.");
        ESP_LOGI(TAG, "Use command: devtype set <device-type>");
        ESP_LOGI(TAG, "==================================================");
    }
}
