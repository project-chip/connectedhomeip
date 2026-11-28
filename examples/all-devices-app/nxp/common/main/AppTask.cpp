/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2026 NXP
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
#include "CHIPDeviceManager.h"
#include "ICDUtil.h"

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
#error "This example does not supply a TermsAndConditionsProvider."
#endif

#include <app/InteractionModelEngine.h>

#include <app/EventManagement.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/reporting/reporting.h>
#include <app/server/Dnssd.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <device-factory/DeviceFactory.h>
#include <device/api/Interface.h>
#if CONFIG_NET_L2_OPENTHREAD
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#endif
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

#include <utility>

#if CONFIG_CHIP_LIB_SHELL
#include "DeviceShellCommands.h"
#endif

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

namespace {

#if CONFIG_NET_L2_OPENTHREAD
DeviceLayer::NetworkCommissioning::GenericThreadDriver gThreadDriver;
#endif

#if CONFIG_CHIP_LIB_SHELL
CHIP_ERROR AddRuntimeDevice(const char * type)
{
    VerifyOrReturnError(type != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return ::AppTask::GetDefaultInstance().RegisterAppDevice(type);
}
#endif // CONFIG_CHIP_LIB_SHELL

} // namespace

void AppTask::PreInitMatterStack()
{
    ChipLogProgress(DeviceLayer, "Welcome to NXP all-devices-app (code-driven) Demo App");
}

void AppTask::PostInitMatterStack()
{
    chip::app::InteractionModelEngine::GetInstance()->RegisterReadHandlerAppCallback(&chip::NXP::App::GetICDUtil());
}

chip::EndpointId AppTask::RuntimeEndpointIdAllocator::Allocate()
{
    VerifyOrReturnValue(mNext < chip::kInvalidEndpointId, chip::kInvalidEndpointId);
    return static_cast<chip::EndpointId>(mNext++);
}

CHIP_ERROR AppTask::PrepareCodeDrivenDataModel(chip::CommonCaseDeviceServerInitParams & params)
{
    VerifyOrReturnError(&params == &mInitParams, CHIP_ERROR_INCORRECT_STATE);
    // Failed preparation may leave dependencies registered; retry requires a reboot.
    VerifyOrReturnError(!mPreparationStarted, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "all-devices: data model preparation cannot be repeated"));
    mPreparationStarted = true;
    VerifyOrReturnError(Credentials::IsDeviceAttestationCredentialsProviderSet(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "all-devices: device attestation provider not registered"));
    VerifyOrReturnError(mDataModelProvider == nullptr && mRootNode == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(InitPersistence());
    ReturnErrorOnFailure(InitRootNode());
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitPersistence()
{
    VerifyOrReturnError(mInitParams.persistentStorageDelegate != nullptr && mInitParams.sessionKeystore != nullptr &&
                            mInitParams.groupDataProvider != nullptr,
                        CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mAttributePersistenceProvider.Init(mInitParams.persistentStorageDelegate));
    mDeviceInfoProvider.SetStorageDelegate(mInitParams.persistentStorageDelegate);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitRootNode()
{
    using namespace chip;
    using namespace chip::app;

    auto * deviceInfo  = DeviceLayer::GetDeviceInstanceInfoProvider();
    auto * dacProvider = Credentials::GetDeviceAttestationCredentialsProvider();
    VerifyOrReturnError(deviceInfo != nullptr && dacProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mDataModelProvider =
        std::make_unique<CodeDrivenDataModelProvider>(*mInitParams.persistentStorageDelegate, mAttributePersistenceProvider);
    VerifyOrReturnError(mDataModelProvider != nullptr, CHIP_ERROR_NO_MEMORY);
    mInitParams.dataModelProvider = mDataModelProvider.get();
    if (mInitParams.testEventTriggerDelegate == nullptr)
    {
        mInitParams.testEventTriggerDelegate = &mDefaultTestEventTriggerDelegate;
    }

    auto & server = Server::GetInstance();
    const RootNode::Context rootContext{
        .commissioningWindowManager          = server.GetCommissioningWindowManager(),
        .configurationManager                = DeviceLayer::ConfigurationMgr(),
        .deviceControlServer                 = DeviceLayer::DeviceControlServer::DeviceControlSvr(),
        .fabricTable                         = server.GetFabricTable(),
        .accessControl                       = server.GetAccessControl(),
        .persistentStorage                   = *mInitParams.persistentStorageDelegate,
        .failSafeContext                     = server.GetFailSafeContext(),
        .deviceInstanceInfoProvider          = *deviceInfo,
        .platformManager                     = DeviceLayer::PlatformMgr(),
        .groupDataProvider                   = *mInitParams.groupDataProvider,
        .sessionManager                      = server.GetSecureSessionManager(),
        .dnssdServer                         = DnssdServer::Instance(),
        .deviceLoadStatusProvider            = *InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider              = DeviceLayer::GetDiagnosticDataProvider(),
        .testEventTriggerDelegate            = mInitParams.testEventTriggerDelegate,
        .dacProvider                         = *dacProvider,
        .eventManagement                     = EventManagement::GetInstance(),
        .timerDelegate                       = mTimerDelegate,
        .minGuaranteedSubscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric(),
    };
#if CONFIG_NET_L2_OPENTHREAD
    ThreadFeature::Context networkContext{ .threadDriver = gThreadDriver };
#elif CONFIG_CHIP_WIFI
    auto * wifiDriver = GetWifiDriverInstance();
    VerifyOrReturnError(wifiDriver != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "all-devices: WiFi driver not available"));
    WifiFeature::Context networkContext{ .wifiDriver = *wifiDriver };
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    auto & otaInitiator = chip::NXP::App::OTARequestorInitiator::Instance();
    OtaFeature::Context otaContext{
        .otaCommands = otaInitiator.gRequestorCore,
        .attributes  = mOTARequestorAttributes,
    };
#endif
    mRootNode = std::make_unique<RootNodeType>(rootContext
#if CONFIG_NET_L2_OPENTHREAD || CONFIG_CHIP_WIFI
                                               ,
                                               networkContext
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
                                               ,
                                               otaContext
#endif
    );
    VerifyOrReturnError(mRootNode != nullptr, CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(mRootNode->Register(kRootEndpointId, *mDataModelProvider));
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    otaInitiator.SetCodeDrivenContext(mOTARequestorAttributes, mRootNode->GetFeature<OtaFeature>().Cluster());
#endif

    NoHooksDeviceFactory::GetInstance().Init(NoHooksDeviceFactory::Context{
        .groupDataProvider        = *mInitParams.groupDataProvider,
        .fabricTable              = server.GetFabricTable(),
        .timerDelegate            = mTimerDelegate,
        .storageDelegate          = *mInitParams.persistentStorageDelegate,
        .diagnosticDataProvider   = DeviceLayer::GetDiagnosticDataProvider(),
        .platformManager          = DeviceLayer::PlatformMgr(),
        .failSafeContext          = server.GetFailSafeContext(),
        .breadcrumbTracker        = mRootNode->GeneralCommissioning(),
        .bindingTable             = Clusters::Binding::Table::GetInstance(),
        .bindingManager           = Clusters::Binding::Manager::GetInstance(),
        .bindingHandler           = mBindingHandler,
        .testEventTriggerDelegate = *mInitParams.testEventTriggerDelegate,
        .identifyDelegate         = mDefaultIdentifyDelegate,
        .deviceInfoProvider       = mDeviceInfoProvider,
    });
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::PostServerInit()
{
    VerifyOrReturnError(mPreparationStarted && mDataModelProvider != nullptr && mRootNode != nullptr, CHIP_ERROR_INCORRECT_STATE);
    auto & server = Server::GetInstance();
    ReturnErrorOnFailure(mBindingHandler.Init(
        Clusters::Binding::Manager::GetInstance(),
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() }, server.GetExchangeManager()));
    mRuntimeReady = true;
    ChipLogProgress(AppServer, "devtype: ready; use 'devtype list' or 'devtype set <type>'");
    return CHIP_NO_ERROR;
}

void AppTask::PostInitMatterServerInstance()
{
    VerifyOrDie(PostServerInit() == CHIP_NO_ERROR);
}

CHIP_ERROR AppTask::RegisterAppDevice(const std::string & type)
{
    VerifyOrReturnError(mRuntimeReady && mDataModelProvider != nullptr && mRootNode != nullptr, CHIP_ERROR_INCORRECT_STATE);
    auto & factory = NoHooksDeviceFactory::GetInstance();
    VerifyOrReturnError(factory.IsValidDevice(type), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(AppServer, "devtype: unsupported type '%s'; use devtype list", type.c_str()));
    VerifyOrReturnError(
        mDeviceCount < mDevices.size(), CHIP_ERROR_NO_MEMORY,
        ChipLogError(AppServer, "devtype: runtime instance limit reached (%u)", static_cast<unsigned>(mDevices.size())));
    VerifyOrReturnError(mAllocator.NextEndpointId() < kInvalidEndpointId, CHIP_ERROR_NO_MEMORY,
                        ChipLogError(AppServer, "devtype: endpoint IDs exhausted"));

    auto entry = factory.Create(type);
    VerifyOrReturnError(entry.device != nullptr, CHIP_ERROR_NO_MEMORY);
    const uint32_t firstEndpoint = mAllocator.NextEndpointId();

    // Cover failures after a derived Register() has completed its base registration.
    DeviceRegistrationTransaction transaction(*entry.device, *mDataModelProvider);
    ReturnErrorOnFailure(entry.Register(mAllocator, *mDataModelProvider));
    VerifyOrReturnError(mAllocator.NextEndpointId() > firstEndpoint, CHIP_ERROR_INCORRECT_STATE);
    transaction.Commit();

    mDevices[mDeviceCount++] = std::move(entry);
    MatterReportingAttributeChangeCallback(kRootEndpointId, Descriptor::Id, Descriptor::Attributes::PartsList::Id);
    ChipLogProgress(AppServer, "devtype: added '%s', endpoint range %u-%u (instances: %u/%u)", type.c_str(),
                    static_cast<unsigned>(firstEndpoint), static_cast<unsigned>(mAllocator.NextEndpointId() - 1),
                    static_cast<unsigned>(mDeviceCount), static_cast<unsigned>(mDevices.size()));
    return CHIP_NO_ERROR;
}

void AppTask::AppMatter_RegisterCustomCliCommands(void)
{
#if CONFIG_CHIP_LIB_SHELL
    chip::Shell::DeviceCommands::GetInstance().Register(AddRuntimeDevice);
#endif
}

// This returns an instance of this class.
AppTask & AppTask::GetDefaultInstance()
{
    static AppTask sAppTask;
    return sAppTask;
}

chip::NXP::App::AppTaskBase & chip::NXP::App::GetAppTask()
{
    return ::AppTask::GetDefaultInstance();
}
