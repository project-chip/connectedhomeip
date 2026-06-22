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

#include "AllDevicesServer.h"
#include "AppTaskCommon.h"
#include "FabricTableDelegate.h"

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <devices/device-factory/DeviceFactory.h>
#include <devices/interface/DeviceInterface.h>
#include <devices/root-node/RootNodeDevice.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <setup_payload/OnboardingCodesUtil.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <devices/root-node/WifiRootNodeDevice.h>
#include <platform/telink/wifi/TelinkWiFiDriver.h>
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <devices/root-node/ThreadRootNodeDevice.h>
#include <platform/NetworkCommissioning.h>
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace chip {
namespace app {
namespace all_devices {
namespace {

constexpr const char * kDeviceTypeStorageKey = "all-devices/dev-type";
constexpr size_t kMaxDeviceTypeLength        = 64;

constexpr EndpointId kRootEndpointId   = 0;
constexpr EndpointId kDeviceEndpointId = 1;

CommonCaseDeviceServerInitParams * gInitParams = nullptr;

std::string gDeviceType;
bool gServerStarted = false;

DefaultAttributePersistenceProvider gAttributePersistenceProvider;
DefaultSafeAttributePersistenceProvider gSafeAttributePersistenceProvider;
Credentials::GroupDataProviderImpl gGroupDataProvider;
DefaultTimerDelegate gTimerDelegate;

std::unique_ptr<CodeDrivenDataModelProvider> gDataModelProvider;
std::unique_ptr<DeviceInterface> gRootNodeDevice;
std::unique_ptr<DeviceInterface> gConstructedDevice;

#if CHIP_ENABLE_OPENTHREAD
DeviceLayer::NetworkCommissioning::GenericThreadDriver gThreadDriver;
#endif

RootNodeDevice::Context MakeRootNodeContext(CommonCaseDeviceServerInitParams & initParams,
                                            DeviceInstanceInfoProvider & deviceInfoProvider)
{
    return RootNodeDevice::Context{
        .commissioningWindowManager       = Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager             = ConfigurationMgr(),
        .deviceControlServer              = DeviceControlServer::DeviceControlSvr(),
        .fabricTable                      = Server::GetInstance().GetFabricTable(),
        .accessControl                    = Server::GetInstance().GetAccessControl(),
        .persistentStorage                = Server::GetInstance().GetPersistentStorage(),
        .failSafeContext                  = Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider       = deviceInfoProvider,
        .platformManager                  = PlatformMgr(),
        .groupDataProvider                = gGroupDataProvider,
        .sessionManager                   = Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                      = DnssdServer::Instance(),
        .deviceLoadStatusProvider         = *InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider           = GetDiagnosticDataProvider(),
        .testEventTriggerDelegate         = initParams.testEventTriggerDelegate,
        .dacProvider                      = *Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement                  = EventManagement::GetInstance(),
        .safeAttributePersistenceProvider = gSafeAttributePersistenceProvider,
        .timerDelegate                    = gTimerDelegate,
    };
}

CHIP_ERROR CreateAndRegisterRootNode(CommonCaseDeviceServerInitParams & initParams)
{
    DeviceInstanceInfoProvider * deviceInfoProvider = GetDeviceInstanceInfoProvider();
    VerifyOrReturnError(deviceInfoProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    gRootNodeDevice = std::make_unique<WifiRootNodeDevice>(MakeRootNodeContext(initParams, *deviceInfoProvider),
                                                           WifiRootNodeDevice::WifiContext{
                                                               .wifiDriver = NetworkCommissioning::TelinkWiFiDriver::Instance(),
                                                           });
#elif CHIP_ENABLE_OPENTHREAD
    gRootNodeDevice = std::make_unique<ThreadRootNodeDevice>(MakeRootNodeContext(initParams, *deviceInfoProvider),
                                                             ThreadRootNodeDevice::ThreadContext{
                                                                 .threadDriver = gThreadDriver,
                                                             });
#else
    gRootNodeDevice = std::make_unique<RootNodeDevice>(MakeRootNodeContext(initParams, *deviceInfoProvider));
#endif

    VerifyOrReturnError(gRootNodeDevice != nullptr, CHIP_ERROR_NO_MEMORY);

    return gRootNodeDevice->Register(kRootEndpointId, *gDataModelProvider, kInvalidEndpointId);
}

CHIP_ERROR PopulateAllDevicesDataModelProvider(CommonCaseDeviceServerInitParams & initParams)
{
    ReturnErrorOnFailure(gAttributePersistenceProvider.Init(initParams.persistentStorageDelegate));
    ReturnErrorOnFailure(gSafeAttributePersistenceProvider.Init(initParams.persistentStorageDelegate));

    SetSafeAttributePersistenceProvider(&gSafeAttributePersistenceProvider);

    initParams.groupDataProvider = &gGroupDataProvider;
    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    gGroupDataProvider.SetSessionKeystore(initParams.sessionKeystore);

    ReturnErrorOnFailure(gGroupDataProvider.Init());
    Credentials::SetGroupDataProvider(&gGroupDataProvider);

    gDataModelProvider =
        std::make_unique<CodeDrivenDataModelProvider>(*initParams.persistentStorageDelegate, gAttributePersistenceProvider);
    VerifyOrReturnError(gDataModelProvider != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(CreateAndRegisterRootNode(initParams));

    DeviceFactory::GetInstance().Init(DeviceFactory::Context{
        .groupDataProvider = gGroupDataProvider,
        .fabricTable       = Server::GetInstance().GetFabricTable(),
        .timerDelegate     = gTimerDelegate,
    });

    VerifyOrReturnError(!gDeviceType.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    auto & deviceFactory = DeviceFactory::GetInstance();

    if (!deviceFactory.IsValidDevice(gDeviceType))
    {
        ChipLogError(AppServer, "Invalid device type: %s", gDeviceType.c_str());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    gConstructedDevice = deviceFactory.Create(gDeviceType);
    VerifyOrReturnError(gConstructedDevice != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(gConstructedDevice->Register(kDeviceEndpointId, *gDataModelProvider, kInvalidEndpointId));

    initParams.dataModelProvider = gDataModelProvider.get();

    return CHIP_NO_ERROR;
}

CHIP_ERROR DoStartAllDevicesServer(const std::string & deviceType, bool persistDeviceType)
{
    VerifyOrReturnError(!gServerStarted, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(gInitParams != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!deviceType.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    gDeviceType = deviceType;

    ReturnErrorOnFailure(PopulateAllDevicesDataModelProvider(*gInitParams));
    ReturnErrorOnFailure(Server::GetInstance().Init(*gInitParams));

    if (persistDeviceType)
    {
        ReturnErrorOnFailure(gInitParams->persistentStorageDelegate->SyncSetKeyValue(kDeviceTypeStorageKey, gDeviceType.data(),
                                                                                     static_cast<uint16_t>(gDeviceType.size())));
    }

    gServerStarted = true;

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));

    AppFabricTableDelegate::Init();

    ChipLogProgress(AppServer, "All-devices server started with device type '%s'", gDeviceType.c_str());

    return CHIP_NO_ERROR;
}

} // namespace

bool IsServerStarted()
{
    return gServerStarted;
}

CHIP_ERROR InitAllDevicesServer(CommonCaseDeviceServerInitParams & initParams)
{
    gInitParams = &initParams;

    char storedDeviceType[kMaxDeviceTypeLength] = {};
    uint16_t storedLen                          = sizeof(storedDeviceType);

    CHIP_ERROR storedErr =
        initParams.persistentStorageDelegate->SyncGetKeyValue(kDeviceTypeStorageKey, storedDeviceType, storedLen);

    if (storedErr == CHIP_NO_ERROR && storedLen > 0)
    {
        std::string deviceType(storedDeviceType, storedLen);

        ChipLogProgress(AppServer, "Found stored device type '%s', starting all-devices server", deviceType.c_str());

        return DoStartAllDevicesServer(deviceType, false /* persistDeviceType */);
    }

    ChipLogProgress(AppServer, "No stored device type found; use 'devtype set <device-type>' to select one");

    return CHIP_NO_ERROR;
}

CHIP_ERROR StartAllDevicesServer(const std::string & deviceType)
{
    return DoStartAllDevicesServer(deviceType, true /* persistDeviceType */);
}

} // namespace all_devices
} // namespace app
} // namespace chip
