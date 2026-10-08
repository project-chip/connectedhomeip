/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include "LoggingCamera.h"

#include <memory>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DefaultTimerDelegate.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/root-node/RootNode.h>
#include <device/types/root-node/RootNodeWith.h>

#if CHIP_ENABLE_OPENTHREAD
#include <device/types/root-node/features/ThreadFeature.h>               // nogncheck
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h> // nogncheck
#elif defined(CHIP_DEVICE_CONFIG_ENABLE_WIFI) && CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <device/types/root-node/features/WifiFeature.h>    // nogncheck
#include <platform/silabs/NetworkCommissioningWiFiDriver.h> // nogncheck
#else
#error "The Silabs camera-app requires either Thread or Wi-Fi"
#endif

#if defined(SILABS_OTA_ENABLED) && SILABS_OTA_ENABLED
#include <app/clusters/ota-requestor/CodegenIntegration.h>  // nogncheck
#include <app/clusters/ota-requestor/DefaultOTARequestor.h> // nogncheck
#include <device/types/root-node/features/OtaFeature.h>     // nogncheck

// gRequestorCore is defined in examples/platform/silabs/OTAConfig.cpp and drives the
// OTA state machine that the OTARequestorCluster (composed by OtaFeature) forwards to.
extern chip::DefaultOTARequestor gRequestorCore;
#endif

#define APP_FUNCTION_BUTTON 0

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;

namespace {
DefaultAttributePersistenceProvider sAttributePersistenceProvider;
DefaultSafeAttributePersistenceProvider sSafeAttributePersistenceProvider;
std::unique_ptr<CodeDrivenDataModelProvider> sDataModelProvider;
std::unique_ptr<RootNode> sRootNode;
LoggingCamera sCamera;

#if CHIP_ENABLE_OPENTHREAD
DeviceLayer::NetworkCommissioning::GenericThreadDriver sThreadDriver;
using NetworkFeature = ThreadFeature;
#else
using NetworkFeature = WifiFeature;
#endif

constexpr EndpointId kCameraEndpointId = 1;
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

    // The camera clusters are started by Server::Init(), which has completed by the time the app task runs.
    PlatformMgr().LockChipStack();
    CHIP_ERROR err = sCamera.InitClusters();
    PlatformMgr().UnlockChipStack();

    return err;
}

void AppTask::ButtonEventHandler(uint8_t button, uint8_t btnAction)
{
    VerifyOrReturn(button == APP_FUNCTION_BUTTON);

    AppEvent button_event           = {};
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.Action = btnAction;
    button_event.Handler            = BaseApplication::ButtonHandler;
    GetAppTask().PostEvent(&button_event);
}

CHIP_ERROR AppTask::InitCodeDrivenDataModel(chip::PersistentStorageDelegate & storage,
                                            chip::Credentials::GroupDataProvider * groupDataProvider,
                                            chip::Crypto::SessionKeystore * /* sessionKeyStore */)
{
    VerifyOrReturnError(groupDataProvider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(sAttributePersistenceProvider.Init(&storage));
    ReturnErrorOnFailure(sSafeAttributePersistenceProvider.Init(&storage));
    SetSafeAttributePersistenceProvider(&sSafeAttributePersistenceProvider);

    sDataModelProvider = std::make_unique<CodeDrivenDataModelProvider>(storage, sAttributePersistenceProvider);
    VerifyOrReturnError(sDataModelProvider != nullptr, CHIP_ERROR_NO_MEMORY);

    DeviceInstanceInfoProvider * deviceInfoProvider = GetDeviceInstanceInfoProvider();
    VerifyOrReturnError(deviceInfoProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    static DefaultTimerDelegate sTimerDelegate;
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate;

    RootNode::Context rootNodeContext = {
        .commissioningWindowManager          = Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager                = ConfigurationMgr(),
        .deviceControlServer                 = DeviceControlServer::DeviceControlSvr(),
        .fabricTable                         = Server::GetInstance().GetFabricTable(),
        .accessControl                       = Server::GetInstance().GetAccessControl(),
        .persistentStorage                   = storage,
        .failSafeContext                     = Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider          = *deviceInfoProvider,
        .platformManager                     = PlatformMgr(),
        .groupDataProvider                   = *groupDataProvider,
        .sessionManager                      = Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                         = DnssdServer::Instance(),
        .deviceLoadStatusProvider            = *InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider              = GetDiagnosticDataProvider(),
        .testEventTriggerDelegate            = &sTestEventTriggerDelegate,
        .dacProvider                         = *Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement                     = EventManagement::GetInstance(),
        .timerDelegate                       = sTimerDelegate,
        .minGuaranteedSubscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric(),
    };

#if CHIP_ENABLE_OPENTHREAD
    NetworkFeature::Context networkContext{ .threadDriver = sThreadDriver };
#else
    NetworkFeature::Context networkContext{ .wifiDriver = *DeviceLayer::NetworkCommissioning::SlWiFiDriver::GetInstance() };
#endif

#if defined(SILABS_OTA_ENABLED) && SILABS_OTA_ENABLED
    OtaFeature::Context otaContext{
        .otaCommands = gRequestorCore,
        .attributes  = chip::GetOTARequestorAttributes(),
    };
    sRootNode = std::make_unique<RootNodeWith<NetworkFeature, OtaFeature>>(rootNodeContext, networkContext, otaContext);
#else
    sRootNode = std::make_unique<RootNodeWith<NetworkFeature>>(rootNodeContext, networkContext);
#endif // SILABS_OTA_ENABLED
    VerifyOrReturnError(sRootNode != nullptr, CHIP_ERROR_NO_MEMORY);

    ConsecutiveEndpointIdAllocator rootAllocator(kRootEndpointId);
    ReturnErrorOnFailure(sRootNode->Register(rootAllocator, *sDataModelProvider));

    ReturnErrorOnFailure(sCamera.Register(kCameraEndpointId, *sDataModelProvider));
    ChipLogProgress(AppServer, "Registered camera device on endpoint %u", kCameraEndpointId);

    return CHIP_NO_ERROR;
}

chip::app::CodeDrivenDataModelProvider * AppTask::GetDataModelProvider()
{
    return sDataModelProvider.get();
}
