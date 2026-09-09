/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppKeys.h"

#ifdef ENABLE_CHIP_SHELL
#include <DeviceShellCommands.h>
#endif

#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <app_config/enabled_devices.h>
#include <delegates/SilabsBatteryPowerSource.h>
#include <device-factory/DeviceFactory.h>
#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/root-node/RootNode.h>

#if CHIP_ENABLE_OPENTHREAD
#include <device/types/root-node/ThreadRootNode.h>
#include <platform/NetworkCommissioning.h>
#endif

#if defined(CHIP_DEVICE_CONFIG_ENABLE_WIFI) && CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <device/types/root-node/WifiRootNode.h>            // nogncheck
#include <platform/silabs/NetworkCommissioningWiFiDriver.h> // nogncheck
#endif

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#define APP_FUNCTION_BUTTON 0

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

namespace {
chip::app::DefaultAttributePersistenceProvider sAttributePersistenceProvider;
chip::app::DefaultSafeAttributePersistenceProvider sSafeAttributePersistenceProvider;
std::unique_ptr<chip::app::CodeDrivenDataModelProvider> sDataModelProvider;
std::unique_ptr<chip::app::DeviceInterface> sRootNode;
std::vector<std::unique_ptr<chip::app::DeviceInterface>> sConstructedDevices;

#if CHIP_ENABLE_OPENTHREAD
chip::DeviceLayer::NetworkCommissioning::GenericThreadDriver sThreadDriver;
#endif

constexpr chip::EndpointId kDeviceEndpointId = 1;
} // namespace

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::StartAppTask()
{
    return BaseApplication::StartAppTask(AppTaskMain);
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    osMessageQueueId_t sAppEventQueue = *(static_cast<osMessageQueueId_t *>(pvParameter));

    CHIP_ERROR err = GetAppTask().Init();
    if (err != CHIP_NO_ERROR)
    {
        SILABS_LOG("AppTask.Init() failed");
        appError(err);
    }

#if !(defined(CHIP_CONFIG_ENABLE_ICD_SERVER) && CHIP_CONFIG_ENABLE_ICD_SERVER)
    // On ICD builds the status LED timer would wake the CPU every 10 ms (see
    // kLightTimerPeriod in BaseApplication.cpp), which defeats low-power mode.
    GetAppTask().StartStatusLEDTimer();
#endif

    SILABS_LOG("App Task started");

    while (true)
    {
        osStatus_t eventReceived = osMessageQueueGet(sAppEventQueue, &event, nullptr, osWaitForever);
        while (eventReceived == osOK)
        {
            GetAppTask().DispatchEvent(&event);
            eventReceived = osMessageQueueGet(sAppEventQueue, &event, nullptr, 0);
        }
    }
}

CHIP_ERROR AppTask::AppInit()
{
    chip::DeviceLayer::Silabs::GetPlatform().SetButtonsCb(&AppTask::ButtonEventHandler);
#ifdef ENABLE_CHIP_SHELL
    chip::Shell::DeviceCommands::GetInstance().Register();
#endif
    return CHIP_NO_ERROR;
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;

    if (button == APP_FUNCTION_BUTTON)
    {
        button_event.Handler = BaseApplication::ButtonHandler;
        GetAppTask().PostEvent(&button_event);
    }
}

CHIP_ERROR AppTask::InitCodeDrivenDataModel(chip::PersistentStorageDelegate & storage,
                                            chip::Credentials::GroupDataProvider * groupDataProvider)
{
    ReturnErrorOnFailure(sAttributePersistenceProvider.Init(&storage));
    ReturnErrorOnFailure(sSafeAttributePersistenceProvider.Init(&storage));
    chip::app::SetSafeAttributePersistenceProvider(&sSafeAttributePersistenceProvider);

    sDataModelProvider = std::make_unique<chip::app::CodeDrivenDataModelProvider>(storage, sAttributePersistenceProvider);
    VerifyOrReturnError(sDataModelProvider != nullptr, CHIP_ERROR_NO_MEMORY);

    chip::DeviceLayer::DeviceInstanceInfoProvider * deviceInfoProvider = chip::DeviceLayer::GetDeviceInstanceInfoProvider();
    VerifyOrReturnError(deviceInfoProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    static chip::app::DefaultTimerDelegate sTimerDelegate;
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate;

    chip::app::RootNode::Context rootNodeContext = {
        .commissioningWindowManager = chip::Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager       = chip::DeviceLayer::ConfigurationMgr(),
        .deviceControlServer        = chip::DeviceLayer::DeviceControlServer::DeviceControlSvr(),
        .fabricTable                = chip::Server::GetInstance().GetFabricTable(),
        .accessControl              = chip::Server::GetInstance().GetAccessControl(),
        .persistentStorage          = storage,
        .failSafeContext            = chip::Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider = *deviceInfoProvider,
        .platformManager            = chip::DeviceLayer::PlatformMgr(),
        .groupDataProvider          = *groupDataProvider,
        .sessionManager             = chip::Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                = chip::app::DnssdServer::Instance(),
        .deviceLoadStatusProvider   = *chip::app::InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider     = chip::DeviceLayer::GetDiagnosticDataProvider(),
        .testEventTriggerDelegate   = &sTestEventTriggerDelegate,
        .dacProvider                = *chip::Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement            = chip::app::EventManagement::GetInstance(),
        .timerDelegate              = sTimerDelegate,
        .minGuaranteedSubscriptionsPerFabric =
            chip::app::InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric(),
    };

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // When the build is configured as an Intermittently Connected Device, hand the RootNode
    // the SessionKeystore so it can construct the ICDManagement cluster on the root endpoint.
    // The ICDManager itself is already owned/initialized by chip::Server when
    // CHIP_CONFIG_ENABLE_ICD_SERVER=1, so no extra setup is required here.
    rootNodeContext.icdSymmetricKeystore = chip::Server::GetInstance().GetSessionKeystore();
    ChipLogProgress(AppServer, "ICD server enabled: registering ICDManagement cluster on the root endpoint");
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if CHIP_ENABLE_OPENTHREAD
    sRootNode = std::make_unique<chip::app::ThreadRootNode>(rootNodeContext,
                                                            chip::app::ThreadRootNode::ThreadContext{
                                                                .threadDriver = sThreadDriver,
                                                            });
#elif defined(CHIP_DEVICE_CONFIG_ENABLE_WIFI) && CHIP_DEVICE_CONFIG_ENABLE_WIFI
    sRootNode = std::make_unique<chip::app::WifiRootNode>(
        rootNodeContext,
        chip::app::WifiRootNode::WifiContext{
            .wifiDriver = *chip::DeviceLayer::NetworkCommissioning::SlWiFiDriver::GetInstance(),
        });
#else
    sRootNode = std::make_unique<chip::app::RootNode>(rootNodeContext);
#endif

    VerifyOrReturnError(sRootNode != nullptr, CHIP_ERROR_NO_MEMORY);

    chip::app::ConsecutiveEndpointIdAllocator rootAllocator(kRootEndpointId);
    ReturnErrorOnFailure(sRootNode->Register(rootAllocator, *sDataModelProvider));

    chip::app::DeviceFactory::GetInstance().Init(chip::app::DeviceFactory::Context{
        .groupDataProvider        = *groupDataProvider,
        .fabricTable              = chip::Server::GetInstance().GetFabricTable(),
        .timerDelegate            = sTimerDelegate,
        .storageDelegate          = storage,
        .diagnosticDataProvider   = chip::DeviceLayer::GetDiagnosticDataProvider(),
        .platformManager          = chip::DeviceLayer::PlatformMgr(),
        .failSafeContext          = chip::Server::GetInstance().GetFailSafeContext(),
        .bindingTable             = chip::app::Clusters::Binding::Table::GetInstance(),
        .bindingManager           = chip::app::Clusters::Binding::Manager::GetInstance(),
        .testEventTriggerDelegate = sTestEventTriggerDelegate,
    });

    auto & deviceFactory = chip::app::DeviceFactory::GetInstance();

#if ALL_DEVICES_ENABLE_POWER_SOURCE
    // Override the generic DecreasingBatteryPowerSource with a silabs-specific
    // implementation tuned for the platform.
    deviceFactory.RegisterCreator("power-source",
                                  []() { return std::make_unique<chip::app::SilabsBatteryPowerSource>(); });
#endif

    ConsecutiveEndpointIdAllocator allocator(kDeviceEndpointId);

    auto instantiateDevice = [&](const std::string & type) -> CHIP_ERROR {
        if (!deviceFactory.IsValidDevice(type))
        {
            ChipLogError(AppServer, "Invalid device type: %s", type.c_str());
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        auto device = deviceFactory.Create(type);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(device->Register(allocator, *sDataModelProvider));
        ChipLogProgress(AppServer, "Registered device type '%s'", type.c_str());
        sConstructedDevices.push_back(std::move(device));
        return CHIP_NO_ERROR;
    };

    // Build-time device list (see all_devices_default_devices in enabled_devices.gni).
    // When the list is non-empty, it fully drives the device topology and the
    // KVS override / factory default is ignored — matching the "no shell needed"
    // build configuration.
    constexpr std::string_view kBuildTimeDevices{ ALL_DEVICES_DEFAULT_DEVICES };

    // Helper that (when this build is configured as an ICD) instantiates an extra
    // `power-source` endpoint so commissioners can display a battery level and
    // battery voltage. The primary device stays whatever the user selected.
    auto maybeAddPowerSource = [&]() -> CHIP_ERROR {
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        if (!deviceFactory.IsValidDevice("power-source"))
        {
            ChipLogError(AppServer,
                         "ICD build requested a power-source endpoint but the device factory has no 'power-source' entry");
            return CHIP_NO_ERROR;
        }
        return instantiateDevice("power-source");
#else
        return CHIP_NO_ERROR;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    };

    if (!kBuildTimeDevices.empty())
    {
        sConstructedDevices.reserve(ALL_DEVICES_DEFAULT_DEVICES_COUNT);
        std::string_view remaining = kBuildTimeDevices;
        while (!remaining.empty())
        {
            auto comma     = remaining.find(',');
            auto tokenView = remaining.substr(0, comma);
            std::string tok(tokenView);
            ReturnErrorOnFailure(instantiateDevice(tok));
            if (comma == std::string_view::npos)
            {
                break;
            }
            remaining.remove_prefix(comma + 1);
        }
        ReturnErrorOnFailure(maybeAddPowerSource());
        return CHIP_NO_ERROR;
    }

    // No build-time selection: fall back to the KVS-stored device type (set via
    // the `devtype` shell command) or the factory default.
    std::string deviceType = deviceFactory.GetDefaultDevice();

    char storedDeviceType[64] = {};
    uint16_t storedLen        = sizeof(storedDeviceType);
    CHIP_ERROR storedErr      = storage.SyncGetKeyValue(chip::kDeviceTypeKey, storedDeviceType, storedLen);
    if (storedErr == CHIP_NO_ERROR && storedLen > 0)
    {
        deviceType = std::string(storedDeviceType, strnlen(storedDeviceType, storedLen));
    }

    if (!deviceFactory.IsValidDevice(deviceType))
    {
        ChipLogError(AppServer, "Invalid device type: %s, falling back to default", deviceType.c_str());
        deviceType = deviceFactory.GetDefaultDevice();
    }

    ReturnErrorOnFailure(instantiateDevice(deviceType));
    ReturnErrorOnFailure(maybeAddPowerSource());
    return CHIP_NO_ERROR;
}

chip::app::CodeDrivenDataModelProvider * AppTask::GetDataModelProvider()
{
    return sDataModelProvider.get();
}
