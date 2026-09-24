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

#pragma once

#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/server/Dnssd.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <device-factory/DeviceFactory.h>
#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/root-node/RootNodeWith.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceControlServer.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ThreadStackManager.h>
#include <platform/Zephyr/DeviceInstanceInfoProviderImpl.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include "Buttons.h"
#include "devices/ZephyrOnOffLight.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <device/types/root-node/features/ThreadFeature.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <device/types/root-node/features/WifiFeature.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <device/types/root-node/features/OtaFeature.h>
#include <platform/Zephyr/OTAImageProcessorImpl.h>
#endif

namespace chip::app::AllDevices {

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::Run()
{
    ReturnErrorOnFailure(Self().InitPlatform());
    ReturnErrorOnFailure(Self().InitCredentials());
    ReturnErrorOnFailure(Self().InitNetwork());
    ReturnErrorOnFailure(Self().InitPersistence());
    ReturnErrorOnFailure(Self().InitRootNode());
    ReturnErrorOnFailure(Self().RegisterDeviceCreators());
    ReturnErrorOnFailure(Self().RegisterOTACluster());
    ReturnErrorOnFailure(Self().RegisterAppDevices());
    ReturnErrorOnFailure(Self().InitServer());
    ReturnErrorOnFailure(Self().InitBoardControls());
    ReturnErrorOnFailure(Self().PostServerInit());
    ReturnErrorOnFailure(Self().InitOTARequestor());

    return DeviceLayer::PlatformMgr().StartEventLoopTask();
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitPlatform()
{
    ReturnErrorOnFailure(Platform::MemoryInit());
    return DeviceLayer::PlatformMgr().InitChipStack();
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitCredentials()
{
#if CONFIG_CHIP_FACTORY_DATA
    // Reaching this means no derived AppTask overrode InitCredentials. Fail rather than
    // silently falling back to test credentials in a build that asked for factory data.
    ChipLogError(AppServer, "CONFIG_CHIP_FACTORY_DATA is set but no AppTask provides it");
    return CHIP_ERROR_NOT_IMPLEMENTED;
#else
    Credentials::SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());
    DeviceLayer::SetDeviceInstanceInfoProvider(&DeviceLayer::DeviceInstanceInfoProviderMgrImpl());
    return CHIP_NO_ERROR;
#endif
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitNetwork()
{
    ReturnErrorOnFailure(Self().InitThreadNetworking());
    ReturnErrorOnFailure(Self().InitWifiNetworking());
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitThreadNetworking()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ReturnErrorOnFailure(DeviceLayer::ThreadStackMgr().InitThreadStack());
    ReturnErrorOnFailure(
        DeviceLayer::ConnectivityMgr().SetThreadDeviceType(DeviceLayer::ConnectivityManager::kThreadDeviceType_Router));
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitWifiNetworking()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // TODO WiFi
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitPersistence()
{
    ReturnErrorOnFailure(mInitParams.InitializeStaticResourcesBeforeServerInit());

    ReturnErrorOnFailure(mAttributePersistenceProvider.Init(mInitParams.persistentStorageDelegate));
    ReturnErrorOnFailure(mSafeAttributePersistenceProvider.Init(mInitParams.persistentStorageDelegate));
    SetSafeAttributePersistenceProvider(&mSafeAttributePersistenceProvider);

    mInitParams.groupDataProvider = &mGroupDataProvider;
    mGroupDataProvider.SetStorageDelegate(mInitParams.persistentStorageDelegate);
    mGroupDataProvider.SetSessionKeystore(mInitParams.sessionKeystore);
    ReturnErrorOnFailure(mGroupDataProvider.Init());
    Credentials::SetGroupDataProvider(&mGroupDataProvider);

    return CHIP_NO_ERROR;
}

template <class Derived>
TestEventTriggerDelegate & AppTaskBase<Derived>::GetTestEventTriggerDelegate()
{
    return mDefaultTestEventTriggerDelegate;
}

template <class Derived>
Clusters::IdentifyDelegate & AppTaskBase<Derived>::GetIdentifyDelegate()
{
    return mDefaultIdentifyDelegate;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitRootNode()
{
    mDataModelProvider =
        std::make_unique<CodeDrivenDataModelProvider>(*mInitParams.persistentStorageDelegate, mAttributePersistenceProvider);
    VerifyOrReturnError(mDataModelProvider != nullptr, CHIP_ERROR_NO_MEMORY);
    mInitParams.dataModelProvider = mDataModelProvider.get();

    mInitParams.testEventTriggerDelegate = &Self().GetTestEventTriggerDelegate();

    DeviceLayer::DeviceInstanceInfoProvider * deviceInstanceInfoProvider = DeviceLayer::GetDeviceInstanceInfoProvider();
    VerifyOrReturnError(deviceInstanceInfoProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const RootNode::Context rootNodeContext{
        .commissioningWindowManager          = Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager                = DeviceLayer::ConfigurationMgr(),
        .deviceControlServer                 = DeviceLayer::DeviceControlServer::DeviceControlSvr(),
        .fabricTable                         = Server::GetInstance().GetFabricTable(),
        .accessControl                       = Server::GetInstance().GetAccessControl(),
        .persistentStorage                   = *mInitParams.persistentStorageDelegate,
        .failSafeContext                     = Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider          = *deviceInstanceInfoProvider,
        .platformManager                     = DeviceLayer::PlatformMgr(),
        .groupDataProvider                   = mGroupDataProvider,
        .sessionManager                      = Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                         = DnssdServer::Instance(),
        .deviceLoadStatusProvider            = *InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider              = DeviceLayer::GetDiagnosticDataProvider(),
        .testEventTriggerDelegate            = mInitParams.testEventTriggerDelegate,
        .dacProvider                         = *Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement                     = EventManagement::GetInstance(),
        .timerDelegate                       = mTimerDelegate,
        .minGuaranteedSubscriptionsPerFabric = InteractionModelEngine::GetInstance()->GetMinGuaranteedSubscriptionsPerFabric(),
    };

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Thread + WiFi
#error "Thread + Wi-Fi root node is not implemented"

#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    using NetworkFeature = WifiFeature;
    NetworkFeature::Context networkContext{
        .wifiDriver = mWifiDriver,
    };

#elif CHIP_DEVICE_CONFIG_ENABLE_THREAD
    using NetworkFeature = ThreadFeature;
    NetworkFeature::Context networkContext{
        .threadDriver = mThreadDriver,
    };

#else
    // None
#error "No network technology enabled."

#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OtaFeature::Context otaContext{
        .otaCommands          = mOTARequestorCore,
        .attributes           = mOTARequestorAttributes,
    };

    using RootNodeType = RootNodeWith<NetworkFeature, OtaFeature>;
    mRootNode = std::make_unique<RootNodeType>(rootNodeContext, networkContext, otaContext);
#else
    using RootNodeType = RootNodeWith<NetworkFeature>;
    mRootNode = std::make_unique<RootNodeType>(rootNodeContext, networkContext);
    
#endif    

    VerifyOrReturnError(mRootNode != nullptr, CHIP_ERROR_NO_MEMORY);

    ConsecutiveEndpointIdAllocator rootAllocator(kRootEndpointId);
    ReturnErrorOnFailure(mRootNode->Register(rootAllocator, *mDataModelProvider));

    NoHooksDeviceFactory::GetInstance().Init(NoHooksDeviceFactory::Context{
        .groupDataProvider        = mGroupDataProvider,
        .fabricTable              = Server::GetInstance().GetFabricTable(),
        .timerDelegate            = mTimerDelegate,
        .storageDelegate          = *mInitParams.persistentStorageDelegate,
        .diagnosticDataProvider   = DeviceLayer::GetDiagnosticDataProvider(),
        .platformManager          = DeviceLayer::PlatformMgr(),
        .failSafeContext          = Server::GetInstance().GetFailSafeContext(),
        .bindingTable             = Clusters::Binding::Table::GetInstance(),
        .bindingManager           = Clusters::Binding::Manager::GetInstance(),
        .testEventTriggerDelegate = *mInitParams.testEventTriggerDelegate,
        .identifyDelegate         = Self().GetIdentifyDelegate(),
    });

    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::RegisterDeviceCreators()
{
    if constexpr (ALL_DEVICES_ENABLE_ON_OFF_LIGHT)
    {
        NoHooksDeviceFactory::GetInstance().RegisterCreator("on-off-light", [this]() {
            return NoHooksDeviceFactory::MakeDevice<ZephyrOnOffLight>(LoggingOnOffLight::Context{
                .groupDataProvider = mGroupDataProvider,
                .fabricTable       = Server::GetInstance().GetFabricTable(),
                .timerDelegate     = mTimerDelegate,
                .identifyDelegate  = Self().GetIdentifyDelegate(),
            });
        });
    }

    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::RegisterOTACluster()
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    // Must be registered before Server::Init() starts the provider.
    mOTARequestorCluster.Create(kRootEndpointId, mOTARequestorCore, mOTARequestorAttributes,
                                Server::GetInstance().GetFabricTable());
    ReturnErrorOnFailure(mDataModelProvider->AddCluster(mOTARequestorCluster.Registration()));
#endif
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitServer()
{
    return Server::GetInstance().Init(mInitParams);
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::RegisterAppDevices()
{
    VerifyOrReturnError(mDataModelProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ConsecutiveEndpointIdAllocator deviceAllocator(kFirstDeviceEndpointId);

    for (const std::string & deviceType : NoHooksDeviceFactory::GetInstance().SupportedDeviceTypes())
    {
        VerifyOrReturnError(mDeviceCount < ALL_DEVICES_ENABLED_DEVICE_COUNT, CHIP_ERROR_NO_MEMORY);

        auto created = NoHooksDeviceFactory::GetInstance().Create(deviceType);
        VerifyOrReturnError(created.device != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(created.device->Register(deviceAllocator, *mDataModelProvider));
        VerifyOrDo(!created.onDeviceRegistered, created.onDeviceRegistered());
        mDevices[mDeviceCount++] = std::move(created.device);
    }

    VerifyOrReturnError(mDeviceCount > 0, CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitBoardControls()
{
    ReturnErrorOnFailure(Button::Init());
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::PostServerInit()
{
    DeviceLayer::ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));
    return CHIP_NO_ERROR;
}

template <class Derived>
CHIP_ERROR AppTaskBase<Derived>::InitOTARequestor()
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    SetRequestorInstance(&mOTARequestorCore);
    mOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    ReturnErrorOnFailure(mOTARequestorCore.Init(Server::GetInstance(), mOTARequestorStorage, mOTARequestorDriver, mOTADownloader,
                                                mOTARequestorAttributes, mOTARequestorCluster.Cluster()));

    auto & imageProcessor = OTAImageProcessorImpl::GetDefaultInstance();
    mOTARequestorDriver.Init(&mOTARequestorCore, &imageProcessor);
    ReturnErrorOnFailure(imageProcessor.Init(&mOTADownloader));
    mOTADownloader.SetImageProcessorDelegate(&imageProcessor);
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR

    return CHIP_NO_ERROR;
}

} // namespace chip::app::AllDevices
