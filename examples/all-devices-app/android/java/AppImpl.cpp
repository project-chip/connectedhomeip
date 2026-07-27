/*
 *   Copyright (c) 2026 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "AppImpl.h"
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/DeviceLoadStatusProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <device-factory/DeviceFactory.h>
#include <devices/endpoint-id-allocator/DynamicEndpointIdAllocator.h>
#include <devices/root-node/RootNodeDevice.h>
#include <json/json.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {
chip::app::DefaultAttributePersistenceProvider gAttributePersistenceProvider;
chip::app::DefaultSafeAttributePersistenceProvider gSafeAttributePersistenceProvider;
Credentials::GroupDataProviderImpl gGroupDataProvider;
DefaultTimerDelegate gTimerDelegate;

std::unique_ptr<RootNodeDevice> gRootNodeDevice;
std::vector<std::unique_ptr<DeviceInterface>> gConstructedDevices;
chip::app::CodeDrivenDataModelProvider * gDataModelProvider = nullptr;
} // namespace

namespace chip {
namespace app {
// Empty overrides for Android platform to bypass linux-specific chime/sound/ranging drivers
void RegisterDeviceFactoryOverrides(TimerDelegate & timerDelegate, PersistentStorageDelegate * storageDelegate) {}
} // namespace app
} // namespace chip

CHIP_ERROR AllDevicesAppInit(int discriminator)
{
    // Android platform already initialized memory, stack, and persistence in AndroidChipPlatform
    return CHIP_NO_ERROR;
}

CHIP_ERROR AllDevicesAppStart(const std::string & configurationJson)
{
    chip::DeviceLayer::StackLock lock;

    static chip::CommonCaseDeviceServerInitParams initParams;
    ReturnErrorOnFailure(initParams.InitializeStaticResourcesBeforeServerInit());

    // Initialize the device factory
    DeviceFactory::GetInstance().Init(DeviceFactory::Context{
        .groupDataProvider = gGroupDataProvider,
        .fabricTable       = Server::GetInstance().GetFabricTable(),
        .timerDelegate     = gTimerDelegate,
        .storageDelegate   = *initParams.persistentStorageDelegate,
    });

    RegisterDeviceFactoryOverrides(gTimerDelegate, initParams.persistentStorageDelegate);

    // Group data provider setup
    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    gGroupDataProvider.SetSessionKeystore(initParams.sessionKeystore);
    ReturnErrorOnFailure(gGroupDataProvider.Init());
    Credentials::SetGroupDataProvider(&gGroupDataProvider);

    // Initialize the attribute persistence providers
    ReturnErrorOnFailure(gAttributePersistenceProvider.Init(initParams.persistentStorageDelegate));
    ReturnErrorOnFailure(gSafeAttributePersistenceProvider.Init(initParams.persistentStorageDelegate));
    SetSafeAttributePersistenceProvider(&gSafeAttributePersistenceProvider);

    // Set attestation credentials provider
    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    // Create the CodeDrivenDataModelProvider
    static chip::app::CodeDrivenDataModelProvider dataModelProvider =
        chip::app::CodeDrivenDataModelProvider(*initParams.persistentStorageDelegate, gAttributePersistenceProvider);
    gDataModelProvider = &dataModelProvider;

    // Create and register Root Node
    DeviceLayer::DeviceInstanceInfoProvider * provider = DeviceLayer::GetDeviceInstanceInfoProvider();
    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    gRootNodeDevice = std::make_unique<RootNodeDevice>(RootNodeDevice::Context{
        .commissioningWindowManager       = Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager             = DeviceLayer::ConfigurationMgr(),
        .deviceControlServer              = DeviceLayer::DeviceControlServer::DeviceControlSvr(),
        .fabricTable                      = Server::GetInstance().GetFabricTable(),
        .accessControl                    = Server::GetInstance().GetAccessControl(),
        .persistentStorage                = Server::GetInstance().GetPersistentStorage(),
        .failSafeContext                  = Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider       = *provider,
        .platformManager                  = DeviceLayer::PlatformMgr(),
        .groupDataProvider                = gGroupDataProvider,
        .sessionManager                   = Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                      = DnssdServer::Instance(),
        .deviceLoadStatusProvider         = *InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider           = DeviceLayer::GetDiagnosticDataProvider(),
        .testEventTriggerDelegate         = initParams.testEventTriggerDelegate,
        .dacProvider                      = *Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement                  = EventManagement::GetInstance(),
        .safeAttributePersistenceProvider = gSafeAttributePersistenceProvider,
        .timerDelegate                    = gTimerDelegate,
    });

    ReturnErrorOnFailure(gRootNodeDevice->Register(kRootEndpointId, dataModelProvider, kInvalidEndpointId));

    // Parse the data model configuration from JSON
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(configurationJson, root))
    {
        ChipLogError(AppServer, "Failed to parse configuration JSON: %s", reader.getFormattedErrorMessages().c_str());
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    DynamicEndpointIdAllocator endpointIdAllocator;

    if (root.isArray())
    {
        for (unsigned int i = 0; i < root.size(); i++)
        {
            Json::Value item       = root[i];
            std::string deviceType = item.get("deviceType", "").asString();
            int endpointId         = item.get("endpointId", 0).asInt();
            int parentId           = item.get("parentId", 0).asInt();
            std::string nodeLabel  = item.get("nodeLabel", "").asString();
            bool bridged           = item.get("bridged", false).asBool();

            if (deviceType.empty() || endpointId <= 0)
            {
                ChipLogError(AppServer, "Invalid device entry at index %u", i);
                continue;
            }

            if (bridged && deviceType != "bridged-node")
            {
                // Create intermediate bridged-node parent
                auto bridgedNode = DeviceFactory::GetInstance().Create("bridged-node", nodeLabel.empty() ? deviceType : nodeLabel);
                if (bridgedNode == nullptr)
                {
                    ChipLogError(AppServer, "Failed to create bridged-node for device: %s", deviceType.c_str());
                    continue;
                }

                ChipLogProgress(AppServer, "Registering intermediate bridged-node on endpoint %u (parent: %d)", endpointId,
                                parentId);
                endpointIdAllocator.ForceNext(static_cast<EndpointId>(endpointId));
                ReturnErrorOnFailure(bridgedNode->Register(endpointIdAllocator, dataModelProvider,
                                                           EndpointComposition::WithParent(static_cast<EndpointId>(parentId))));
                gConstructedDevices.push_back(std::move(bridgedNode));

                // Create leaf device parented to the bridged-node endpoint
                auto leafDevice = DeviceFactory::GetInstance().Create(deviceType, "");
                if (leafDevice == nullptr)
                {
                    ChipLogError(AppServer, "Failed to create leaf device of type: %s", deviceType.c_str());
                    continue;
                }

                ChipLogProgress(AppServer, "Registering bridged leaf device %s on endpoint %u (parent: %u)", deviceType.c_str(),
                                endpointId + 1, endpointId);
                endpointIdAllocator.ForceNext(static_cast<EndpointId>(endpointId + 1));
                ReturnErrorOnFailure(leafDevice->Register(endpointIdAllocator, dataModelProvider,
                                                          EndpointComposition::WithParent(static_cast<EndpointId>(endpointId))));
                gConstructedDevices.push_back(std::move(leafDevice));
            }
            else
            {
                auto device = DeviceFactory::GetInstance().Create(deviceType, nodeLabel);
                if (device == nullptr)
                {
                    ChipLogError(AppServer, "Failed to create device of type: %s", deviceType.c_str());
                    continue;
                }

                ChipLogProgress(AppServer, "Registering device %s on endpoint %u (parent: %d)", deviceType.c_str(), endpointId,
                                parentId);
                endpointIdAllocator.ForceNext(static_cast<EndpointId>(endpointId));
                ReturnErrorOnFailure(device->Register(endpointIdAllocator, dataModelProvider,
                                                      EndpointComposition::WithParent(static_cast<EndpointId>(parentId))));
                gConstructedDevices.push_back(std::move(device));
            }
        }
    }

    initParams.dataModelProvider             = gDataModelProvider;
    initParams.groupDataProvider             = &gGroupDataProvider;
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    // Start Server
    ReturnErrorOnFailure(Server::GetInstance().Init(initParams));

    // Print setup payload and codes
    ConfigurationMgr().LogDeviceConfig();

    chip::PayloadContents payload;
    payload.version = 0;
    payload.rendezvousInformation.SetValue(RendezvousInformationFlag::kOnNetwork);

    if (GetCommissionableDataProvider()->GetSetupPasscode(payload.setUpPINCode) != CHIP_NO_ERROR)
    {
        payload.setUpPINCode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
    }

    uint16_t discriminator = 0;
    SuccessOrDie(GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));
    payload.discriminator.SetLongValue(discriminator);

    SuccessOrDie(chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(payload.vendorID));
    SuccessOrDie(chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(payload.productID));
    PrintOnboardingCodes(payload);

    return CHIP_NO_ERROR;
}

void AllDevicesAppShutdown()
{
    chip::DeviceLayer::StackLock lock;
    for (auto & device : gConstructedDevices)
    {
        if (gDataModelProvider)
        {
            device->Unregister(*gDataModelProvider);
        }
    }
    gConstructedDevices.clear();

    if (gRootNodeDevice && gDataModelProvider)
    {
        gRootNodeDevice->Unregister(*gDataModelProvider);
    }
    gRootNodeDevice.reset();

    Server::GetInstance().Shutdown();
}

namespace chip {
namespace app {
// Dummy implementation to resolve reference in legacy AndroidAppServerWrapper
DataModel::Provider * CodegenDataModelProviderInstance(PersistentStorageDelegate * delegate)
{
    return nullptr;
}
} // namespace app
} // namespace chip
