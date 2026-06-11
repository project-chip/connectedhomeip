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

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>

#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <device-factory/DeviceFactory.h>
#include <devices/root-node/RootNodeDevice.h>

#if CHIP_ENABLE_OPENTHREAD
#include <devices/root-node/ThreadRootNodeDevice.h>
#include <platform/NetworkCommissioning.h>
#endif

#if defined(CHIP_DEVICE_CONFIG_ENABLE_WIFI) && CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <devices/root-node/WifiRootNodeDevice.h>           // nogncheck
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
std::unique_ptr<chip::app::DeviceInterface> sRootNodeDevice;
std::unique_ptr<chip::app::DeviceInterface> sConstructedDevice;

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

    GetAppTask().StartStatusLEDTimer();

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

    chip::app::RootNodeDevice::Context rootNodeContext = {
        .commissioningWindowManager       = chip::Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager             = chip::DeviceLayer::ConfigurationMgr(),
        .deviceControlServer              = chip::DeviceLayer::DeviceControlServer::DeviceControlSvr(),
        .fabricTable                      = chip::Server::GetInstance().GetFabricTable(),
        .accessControl                    = chip::Server::GetInstance().GetAccessControl(),
        .persistentStorage                = storage,
        .failSafeContext                  = chip::Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider       = *deviceInfoProvider,
        .platformManager                  = chip::DeviceLayer::PlatformMgr(),
        .groupDataProvider                = *groupDataProvider,
        .sessionManager                   = chip::Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                      = chip::app::DnssdServer::Instance(),
        .deviceLoadStatusProvider         = *chip::app::InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider           = chip::DeviceLayer::GetDiagnosticDataProvider(),
        .testEventTriggerDelegate         = nullptr,
        .dacProvider                      = *chip::Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement                  = chip::app::EventManagement::GetInstance(),
        .safeAttributePersistenceProvider = sSafeAttributePersistenceProvider,
        .timerDelegate                    = sTimerDelegate,
    };

#if CHIP_ENABLE_OPENTHREAD
    sRootNodeDevice = std::make_unique<chip::app::ThreadRootNodeDevice>(rootNodeContext,
                                                                        chip::app::ThreadRootNodeDevice::ThreadContext{
                                                                            .threadDriver = sThreadDriver,
                                                                        });
#elif defined(CHIP_DEVICE_CONFIG_ENABLE_WIFI) && CHIP_DEVICE_CONFIG_ENABLE_WIFI
    sRootNodeDevice = std::make_unique<chip::app::WifiRootNodeDevice>(
        rootNodeContext,
        chip::app::WifiRootNodeDevice::WifiContext{
            .wifiDriver = *chip::DeviceLayer::NetworkCommissioning::SlWiFiDriver::GetInstance(),
        });
#else
    sRootNodeDevice = std::make_unique<chip::app::RootNodeDevice>(rootNodeContext);
#endif

    VerifyOrReturnError(sRootNodeDevice != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(sRootNodeDevice->Register(kRootEndpointId, *sDataModelProvider, chip::kInvalidEndpointId));

    chip::app::DeviceFactory::GetInstance().Init(chip::app::DeviceFactory::Context{
        .groupDataProvider = *groupDataProvider,
        .fabricTable       = chip::Server::GetInstance().GetFabricTable(),
        .timerDelegate     = sTimerDelegate,
        .storageDelegate   = storage,
    });

    std::string deviceType = chip::app::DeviceFactory::GetInstance().GetDefaultDevice();

    char storedDeviceType[64] = {};
    uint16_t storedLen        = sizeof(storedDeviceType);
    CHIP_ERROR storedErr      = storage.SyncGetKeyValue(chip::kDeviceTypeKey, storedDeviceType, storedLen);
    if (storedErr == CHIP_NO_ERROR && storedLen > 0)
    {
        deviceType = std::string(storedDeviceType, strnlen(storedDeviceType, storedLen));
    }

    auto & deviceFactory = chip::app::DeviceFactory::GetInstance();
    if (!deviceFactory.IsValidDevice(deviceType))
    {
        ChipLogError(AppServer, "Invalid device type: %s, falling back to default", deviceType.c_str());
        deviceType = deviceFactory.GetDefaultDevice();
    }

    sConstructedDevice = deviceFactory.Create(deviceType);
    VerifyOrReturnError(sConstructedDevice != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(sConstructedDevice->Register(kDeviceEndpointId, *sDataModelProvider, chip::kInvalidEndpointId));

    return CHIP_NO_ERROR;
}

chip::app::CodeDrivenDataModelProvider * AppTask::GetDataModelProvider()
{
    return sDataModelProvider.get();
}
