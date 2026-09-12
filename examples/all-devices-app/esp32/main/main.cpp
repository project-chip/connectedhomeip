/*
 *
 *    Copyright (c) 2025-2026 Project CHIP Authors
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

#include <ESP32DimmableLight.h>
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <device-factory/DeviceFactory.h>
#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/api/allocator/DynamicEndpointIdAllocator.h>
#include <device/capabilities/identify/LoggingIdentifyDelegate.h>
#include <device/types/root-node/WifiRootNode.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <oob-accessors/OOBAccessorRegistry.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/NetworkCommissioningDriver.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <app_config/enabled_devices.h>

#include <cctype>
#include <memory>
#include <string>
#include <vector>

#if CONFIG_HAVE_DISPLAY
#include "DeviceDisplay.h"
#include "Display.h"
#include "ScreenManager.h"
#endif // CONFIG_HAVE_DISPLAY

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
static std::string gDeviceType;
static const size_t kMaxDeviceTypeLength = 64;

#include "DeviceFactoryPlatformOverride.h"

namespace {

std::string KebabCaseToTitleCase(const std::string & input)
{
    std::string output  = input;
    bool capitalizeNext = true;
    for (char & c : output)
    {
        if (c == '-')
        {
            c              = ' ';
            capitalizeNext = true;
        }
        else if (capitalizeNext)
        {
            c              = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            capitalizeNext = false;
        }
    }
    return output;
}

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
chip::app::DefaultSafeAttributePersistenceProvider gSafeAttributePersistenceProvider;
Credentials::GroupDataProviderImpl gGroupDataProvider;
LoggingIdentifyDelegate gIdentifyDelegate;
chip::app::CodeDrivenDataModelProvider * gDataModelProvider = nullptr;
std::unique_ptr<DeviceInterface> gRootNode;
std::vector<std::unique_ptr<DeviceInterface>> gConstructedDevices;
std::vector<std::unique_ptr<OOBAccessor>> gConstructedAccessors;
DefaultTimerDelegate gTimerDelegate;

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

chip::app::DataModel::Provider * PopulateCodeDrivenDataModelProvider(PersistentStorageDelegate * delegate,
                                                                     TestEventTriggerDelegate * testEventTriggerDelegate)
{
    // Initialize the attribute persistence provider with the storage delegate
    CHIP_ERROR err = gAttributePersistenceProvider.Init(delegate);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to init attribute persistence provider: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    // Initialize the safe attribute persistence provider with the storage delegate
    err = gSafeAttributePersistenceProvider.Init(delegate);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to init safe attribute persistence provider: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }
    SetSafeAttributePersistenceProvider(&gSafeAttributePersistenceProvider);

    static chip::app::CodeDrivenDataModelProvider dataModelProvider =
        chip::app::CodeDrivenDataModelProvider(*delegate, gAttributePersistenceProvider);

    gDataModelProvider = &dataModelProvider;

    DeviceLayer::DeviceInstanceInfoProvider * provider = DeviceLayer::GetDeviceInstanceInfoProvider();
    if (provider == nullptr)
    {
        ESP_LOGE(TAG, "Failed to get DeviceInstanceInfoProvider.");
        return nullptr;
    }

    gRootNode = std::make_unique<WifiRootNode>(
        RootNode::Context {
            .commissioningWindowManager     = Server::GetInstance().GetCommissioningWindowManager(),   //
                .configurationManager       = DeviceLayer::ConfigurationMgr(),                         //
                .deviceControlServer        = DeviceLayer::DeviceControlServer::DeviceControlSvr(),    //
                .fabricTable                = Server::GetInstance().GetFabricTable(),                  //
                .accessControl              = Server::GetInstance().GetAccessControl(),                //
                .persistentStorage          = Server::GetInstance().GetPersistentStorage(),            //
                .failSafeContext            = Server::GetInstance().GetFailSafeContext(),              //
                .deviceInstanceInfoProvider = *provider,                                               //
                .platformManager            = DeviceLayer::PlatformMgr(),                              //
                .groupDataProvider          = gGroupDataProvider,                                      //
                .sessionManager             = Server::GetInstance().GetSecureSessionManager(),         //
                .dnssdServer                = DnssdServer::Instance(),                                 //
                .deviceLoadStatusProvider   = *InteractionModelEngine::GetInstance(),                  //
                .diagnosticDataProvider     = DeviceLayer::GetDiagnosticDataProvider(),                //
                .testEventTriggerDelegate   = testEventTriggerDelegate,                                //
                .dacProvider                = *Credentials::GetDeviceAttestationCredentialsProvider(), //
                .eventManagement            = EventManagement::GetInstance(),                          //
                .timerDelegate              = gTimerDelegate,                                          //
                .minGuaranteedSubscriptionsPerFabric =
                    InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric(), //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        },
        WifiRootNode::WifiContext{
            .wifiDriver = sWiFiDriver,
        });

    ConsecutiveEndpointIdAllocator rootAllocator(kRootEndpointId);
    err = gRootNode->Register(rootAllocator, dataModelProvider);
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to register root node device: %" CHIP_ERROR_FORMAT, err.Format());
        return nullptr;
    }

    auto & deviceFactory = NoHooksDeviceFactory::GetInstance();
    gConstructedDevices.clear();

    DynamicEndpointIdAllocator endpointIdAllocator;
    constexpr size_t kMinFreeInternalHeap = 24 * 1024;

    if (gDeviceType == "*" || gDeviceType == "aggregator")
    {
        if (deviceFactory.IsValidDevice("aggregator") && deviceFactory.IsValidDevice("bridged-node"))
        {
            auto aggregator = deviceFactory.Create("aggregator");
            if (aggregator.device == nullptr)
            {
                ESP_LOGE(TAG, "Failed to create aggregator");
                return nullptr;
            }
            EndpointId aggregatorEp = endpointIdAllocator.Allocate();
            endpointIdAllocator.ForceNext(aggregatorEp);
            ESP_LOGI(TAG, "Registering aggregator on endpoint %u", aggregatorEp);
            err = aggregator.device->Register(endpointIdAllocator, dataModelProvider,
                                              EndpointComposition::WithParent(kInvalidEndpointId));
            if (err != CHIP_NO_ERROR)
            {
                ESP_LOGE(TAG, "Failed to register aggregator on endpoint %u: %" CHIP_ERROR_FORMAT, aggregatorEp, err.Format());
                return nullptr;
            }
            if (aggregator.onDeviceRegistered)
            {
                aggregator.onDeviceRegistered();
            }
            gConstructedDevices.push_back(std::move(aggregator.device));

            unsigned int createdCount = 0;
            unsigned int skippedCount = 0;

            for (const auto & deviceType : deviceFactory.SupportedDeviceTypes())
            {
                if (deviceType == "aggregator" || deviceType == "bridged-node")
                {
                    continue;
                }

                size_t freeInternalHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
                if (freeInternalHeap < kMinFreeInternalHeap)
                {
                    ESP_LOGW(TAG, "Not creating '%s': low internal heap (%u bytes free, threshold %u bytes)", deviceType.c_str(),
                             static_cast<unsigned int>(freeInternalHeap), static_cast<unsigned int>(kMinFreeInternalHeap));
                    skippedCount++;
                    continue;
                }

                auto bridgedNode = deviceFactory.Create("bridged-node", KebabCaseToTitleCase(deviceType));
                if (bridgedNode.device == nullptr)
                {
                    ESP_LOGE(TAG, "Failed to create bridged-node for '%s'", deviceType.c_str());
                    skippedCount++;
                    continue;
                }
                EndpointId bnEp = endpointIdAllocator.Allocate();
                endpointIdAllocator.ForceNext(bnEp);
                ESP_LOGI(TAG, "Registering bridged-node for '%s' on endpoint %u (parent %u)", deviceType.c_str(), bnEp,
                         aggregatorEp);
                err = bridgedNode.device->Register(endpointIdAllocator, dataModelProvider,
                                                   EndpointComposition::WithParent(aggregatorEp));
                if (err != CHIP_NO_ERROR)
                {
                    ESP_LOGE(TAG, "Failed to register bridged-node for '%s' on endpoint %u: %" CHIP_ERROR_FORMAT,
                             deviceType.c_str(), bnEp, err.Format());
                    skippedCount++;
                    continue;
                }

                auto device = deviceFactory.Create(deviceType);
                if (device.device == nullptr)
                {
                    ESP_LOGE(TAG, "Failed to create device '%s'", deviceType.c_str());
                    bridgedNode.device->Unregister(dataModelProvider);
                    skippedCount++;
                    continue;
                }
                ESP_LOGI(TAG, "Registering device '%s' with parent %u", deviceType.c_str(), bnEp);
                err = device.device->Register(endpointIdAllocator, dataModelProvider, EndpointComposition::WithParent(bnEp));
                if (err != CHIP_NO_ERROR)
                {
                    ESP_LOGE(TAG, "Failed to register device '%s' with parent %u: %" CHIP_ERROR_FORMAT, deviceType.c_str(), bnEp,
                             err.Format());
                    bridgedNode.device->Unregister(dataModelProvider);
                    skippedCount++;
                    continue;
                }

                if (bridgedNode.onDeviceRegistered)
                {
                    bridgedNode.onDeviceRegistered();
                }
                if (device.onDeviceRegistered)
                {
                    device.onDeviceRegistered();
                }

                gConstructedDevices.push_back(std::move(bridgedNode.device));
                gConstructedDevices.push_back(std::move(device.device));
                createdCount++;
            }
            ESP_LOGI(TAG,
                     "Aggregator initialization complete: %u bridged devices created, %u devices skipped (remaining internal heap: "
                     "%u bytes)",
                     createdCount, skippedCount,
                     static_cast<unsigned int>(heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL)));
        }
        else
        {
            auto defaultDevName = deviceFactory.GetDefaultDevice();
            gDeviceType         = defaultDevName;
            auto defaultDevice  = deviceFactory.Create(defaultDevName);
            if (defaultDevice.device == nullptr)
            {
                ESP_LOGE(TAG, "Failed to create default device %s", defaultDevName.c_str());
                return nullptr;
            }
            endpointIdAllocator.ForceNext(CONFIG_ALL_DEVICES_ENDPOINT);
            ESP_LOGI(TAG, "Registering default device '%s' on endpoint %u", defaultDevName.c_str(),
                     static_cast<unsigned int>(CONFIG_ALL_DEVICES_ENDPOINT));
            err = defaultDevice.device->Register(endpointIdAllocator, dataModelProvider);
            if (err != CHIP_NO_ERROR)
            {
                ESP_LOGE(TAG, "Failed to register default device '%s': %" CHIP_ERROR_FORMAT, defaultDevName.c_str(), err.Format());
                return nullptr;
            }
            if (defaultDevice.onDeviceRegistered)
            {
                defaultDevice.onDeviceRegistered();
            }
            gConstructedDevices.push_back(std::move(defaultDevice.device));
        }
    }
    else
    {
        if (gDeviceType.empty() || !deviceFactory.IsValidDevice(gDeviceType))
        {
            gDeviceType = deviceFactory.GetDefaultDevice();
        }
        auto device = deviceFactory.Create(gDeviceType);
        if (device.device == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create device of type: %s", gDeviceType.c_str());
            return nullptr;
        }

        endpointIdAllocator.ForceNext(CONFIG_ALL_DEVICES_ENDPOINT);
        ESP_LOGI(TAG, "Registering device '%s' on endpoint %u", gDeviceType.c_str(),
                 static_cast<unsigned int>(CONFIG_ALL_DEVICES_ENDPOINT));
        err = device.device->Register(endpointIdAllocator, dataModelProvider);
        if (err != CHIP_NO_ERROR)
        {
            ESP_LOGE(TAG, "Failed to register device '%s': %" CHIP_ERROR_FORMAT, gDeviceType.c_str(), err.Format());
            return nullptr;
        }
        if (device.onDeviceRegistered)
        {
            device.onDeviceRegistered();
        }

        gConstructedDevices.push_back(std::move(device.device));
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

    // Initialize the test event trigger delegate
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate;
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    NoHooksDeviceFactory::GetInstance().Init(NoHooksDeviceFactory::Context{
        .groupDataProvider        = gGroupDataProvider,                     //
        .fabricTable              = Server::GetInstance().GetFabricTable(), //
        .timerDelegate            = gTimerDelegate,                         //
        .storageDelegate          = *initParams.persistentStorageDelegate,  //
        .diagnosticDataProvider   = DeviceLayer::GetDiagnosticDataProvider(),
        .platformManager          = DeviceLayer::PlatformMgr(),
        .failSafeContext          = Server::GetInstance().GetFailSafeContext(),
        .bindingTable             = Clusters::Binding::Table::GetInstance(),
        .bindingManager           = Clusters::Binding::Manager::GetInstance(),
        .testEventTriggerDelegate = *initParams.testEventTriggerDelegate,
        .identifyDelegate         = gIdentifyDelegate,
    });

#if ALL_DEVICES_ENABLE_DIMMABLE_LIGHT
    // Override dimmable-light with ESP32 hardware implementation that drives a real LED
    NoHooksDeviceFactory::GetInstance().RegisterCreator("dimmable-light", [&]() {
        return NoHooksDeviceFactory::MakeDevice<ESP32DimmableLight>(ESP32DimmableLight::Context{
            .groupDataProvider = gGroupDataProvider,
            .fabricTable       = Server::GetInstance().GetFabricTable(),
            .timerDelegate     = gTimerDelegate,
            .identifyDelegate  = gIdentifyDelegate,
        });
    });
#endif

    RegisterDeviceFactoryOverrides(gTimerDelegate, initParams.persistentStorageDelegate);

    initParams.groupDataProvider = &gGroupDataProvider;
    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    gGroupDataProvider.SetSessionKeystore(initParams.sessionKeystore);
    SuccessOrDie(gGroupDataProvider.Init());
    Credentials::SetGroupDataProvider(&gGroupDataProvider);

    initParams.dataModelProvider =
        PopulateCodeDrivenDataModelProvider(initParams.persistentStorageDelegate, initParams.testEventTriggerDelegate);
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
    SuccessOrDie(PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr)));
}

const std::string & GetActiveDeviceType()
{
    return gDeviceType;
}

CHIP_ERROR SetDeviceTypeAndRestart(const std::string & deviceType)
{
    ESP_LOGI(TAG, "Saving device type '%s' to NVS and restarting...", deviceType.c_str());
    CHIP_ERROR err = ESP32Config::WriteConfigValueStr(kConfigKey_DeviceType, deviceType.c_str());
    if (err != CHIP_NO_ERROR)
    {
        ESP_LOGE(TAG, "Failed to save device type to NVS: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

#if CONFIG_HAVE_DISPLAY
    TFT_fillScreen(TFT_BLACK);
    TFT_setFont(DEJAVU24_FONT, nullptr);
    tft_fg = ScreenNormalColor;
    TFT_print("Restarting...", 40, DisplayHeight / 2 - 20);
#endif

    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
    return CHIP_NO_ERROR;
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
    SuccessOrDie(PlatformMgr().AddEventHandler(DeviceEventHandler, 0));

#if CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER
    SetCommissionableDataProvider(&sFactoryDataProvider);
#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_ENABLE_ESP32_FACTORY_DATA_PROVIDER

    // Print onboarding codes (QR code URL and manual code)
    PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));

#if CONFIG_HAVE_DISPLAY
    InitDeviceDisplay();
#endif // CONFIG_HAVE_DISPLAY

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

#if CONFIG_ENABLE_CHIP_SHELL
    chip::LaunchShell();
    chip::Shell::DeviceCommands::GetInstance().Register();
#endif // CONFIG_ENABLE_CHIP_SHELL

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
        ESP_LOGI(TAG, "==================================================");
        ESP_LOGI(TAG, "No stored device type found, defaulting to all bridged devices (*)");
        ESP_LOGI(TAG, "Auto-initializing...");
        ESP_LOGI(TAG, "==================================================");
        InitServerWithDeviceType("*");
    }
}
