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

#include "Buttons.h"

#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/server/Dnssd.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <device-factory/DeviceFactory.h>
#include <device/api/allocator/ConsecutiveEndpointIdAllocator.h>
#include <device/types/root-node/ThreadRootNode.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceControlServer.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/ThreadStackManager.h>
#include <platform/Zephyr/DeviceInstanceInfoProviderImpl.h>
#include <setup_payload/OnboardingCodesUtil.h>

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <platform/Zephyr/OTAImageProcessorImpl.h>
#endif

using namespace chip::Credentials;
using namespace chip::DeviceLayer;

namespace chip::app::AllDevices {
namespace {

constexpr EndpointId kRootEndpointId   = 0;
constexpr EndpointId kFirstDeviceEndpointId = 1;

} // namespace

CHIP_ERROR AppTask::Run()
{
    ReturnErrorOnFailure(InitPlatform());
    ReturnErrorOnFailure(InitCredentials());
    ReturnErrorOnFailure(InitNetwork());
    ReturnErrorOnFailure(InitPersistence());
    ReturnErrorOnFailure(InitRootNode());
    ReturnErrorOnFailure(RegisterOTACluster());
    ReturnErrorOnFailure(RegisterAppDevices());
    ReturnErrorOnFailure(InitServer());
    ReturnErrorOnFailure(InitBoardControls());
    ReturnErrorOnFailure(PostServerInit());
    ReturnErrorOnFailure(InitOTARequestor());

    return PlatformMgr().StartEventLoopTask();
}

CHIP_ERROR AppTask::InitPlatform()
{
    ReturnErrorOnFailure(Platform::MemoryInit());
    return PlatformMgr().InitChipStack();
}

CHIP_ERROR AppTask::InitCredentials()
{
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitNetwork()
{
    ReturnErrorOnFailure(InitThreadNetworking());
    ReturnErrorOnFailure(InitWifiNetworking());
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitThreadNetworking()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ReturnErrorOnFailure(ThreadStackMgr().InitThreadStack());
    ReturnErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router));
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitWifiNetworking()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // TODO WiFi
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitPersistence()
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

TestEventTriggerDelegate & AppTask::GetTestEventTriggerDelegate()
{
    return mDefaultTestEventTriggerDelegate;
}

CHIP_ERROR AppTask::InitRootNode()
{
    mDataModelProvider =
        std::make_unique<CodeDrivenDataModelProvider>(*mInitParams.persistentStorageDelegate, mAttributePersistenceProvider);
    VerifyOrReturnError(mDataModelProvider != nullptr, CHIP_ERROR_NO_MEMORY);
    mInitParams.dataModelProvider = mDataModelProvider.get();

    mInitParams.testEventTriggerDelegate = &GetTestEventTriggerDelegate();

    DeviceInstanceInfoProvider * deviceInstanceInfoProvider = GetDeviceInstanceInfoProvider();
    VerifyOrReturnError(deviceInstanceInfoProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const RootNode::Context context{
        .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(),
        .configurationManager       = ConfigurationMgr(),
        .deviceControlServer        = DeviceControlServer::DeviceControlSvr(),
        .fabricTable                = Server::GetInstance().GetFabricTable(),
        .accessControl              = Server::GetInstance().GetAccessControl(),
        .persistentStorage          = *mInitParams.persistentStorageDelegate,
        .failSafeContext            = Server::GetInstance().GetFailSafeContext(),
        .deviceInstanceInfoProvider = *deviceInstanceInfoProvider,
        .platformManager            = PlatformMgr(),
        .groupDataProvider          = mGroupDataProvider,
        .sessionManager             = Server::GetInstance().GetSecureSessionManager(),
        .dnssdServer                = DnssdServer::Instance(),
        .deviceLoadStatusProvider   = *InteractionModelEngine::GetInstance(),
        .diagnosticDataProvider     = GetDiagnosticDataProvider(),
        .testEventTriggerDelegate   = mInitParams.testEventTriggerDelegate,
        .dacProvider                = *Credentials::GetDeviceAttestationCredentialsProvider(),
        .eventManagement            = EventManagement::GetInstance(),
        .timerDelegate              = mTimerDelegate,
    };

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // Thread + WiFi
#error "Thread + Wi-Fi root node is not implemented"

#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // WiFi
#error "Wi-Fi root node is not implemented"

#elif CHIP_DEVICE_CONFIG_ENABLE_THREAD
    // Thread
    mRootNode = std::make_unique<ThreadRootNode>(context, ThreadRootNode::ThreadContext{ .threadDriver = mThreadDriver });

#else
    // None
#error "No network technology enabled."

#endif

    VerifyOrReturnError(mRootNode != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ConsecutiveEndpointIdAllocator rootAllocator(kRootEndpointId);
    ReturnErrorOnFailure(mRootNode->Register(rootAllocator, *mDataModelProvider));

    DeviceFactory::GetInstance().Init(DeviceFactory::Context{
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
    });

    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::RegisterOTACluster()
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    // Must be registered before Server::Init() starts the provider.
    mOTARequestorCluster.Create(kRootEndpointId, mOTARequestorCore, mOTARequestorAttributes,
                                Server::GetInstance().GetFabricTable());
    ReturnErrorOnFailure(mDataModelProvider->AddCluster(mOTARequestorCluster.Registration()));
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitServer()
{
    return Server::GetInstance().Init(mInitParams);
}

CHIP_ERROR AppTask::RegisterAppDevices()
{
    VerifyOrReturnError(mDataModelProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ConsecutiveEndpointIdAllocator deviceAllocator(kFirstDeviceEndpointId);

    for (const std::string & deviceType : DeviceFactory::GetInstance().SupportedDeviceTypes())
    {
        VerifyOrReturnError(mDeviceCount < ALL_DEVICES_ENABLED_DEVICE_COUNT, CHIP_ERROR_NO_MEMORY);

        std::unique_ptr<DeviceInterface> device = DeviceFactory::GetInstance().Create(deviceType);
        VerifyOrReturnError(device != nullptr, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(device->Register(deviceAllocator, *mDataModelProvider));
        mDevices[mDeviceCount++] = std::move(device);
    }

    VerifyOrReturnError(mDeviceCount > 0, CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitBoardControls()
{
    ReturnErrorOnFailure(Button::Init());
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::PostServerInit()
{
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(RendezvousInformationFlags(RendezvousInformationFlag::kBLE));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::InitOTARequestor()
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    SetRequestorInstance(&mOTARequestorCore);
    mOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    ReturnErrorOnFailure(mOTARequestorCore.Init(Server::GetInstance(), mOTARequestorStorage, mOTARequestorDriver,
                                                mOTADownloader, mOTARequestorAttributes, mOTARequestorCluster.Cluster()));

    auto & imageProcessor = OTAImageProcessorImpl::GetDefaultInstance();
    mOTARequestorDriver.Init(&mOTARequestorCore, &imageProcessor);
    ReturnErrorOnFailure(imageProcessor.Init(&mOTADownloader));
    mOTADownloader.SetImageProcessorDelegate(&imageProcessor);
#endif // CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR

    return CHIP_NO_ERROR;
}

} // namespace chip::app::AllDevices
