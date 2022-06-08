/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/server/Server.h>

#include <access/examples/ExampleAccessControlDelegate.h>

#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Dnssd.h>
#include <app/server/EchoHandler.h>
#include <app/util/DataModelHandler.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BLEEndPoint.h>
#endif
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/PersistedCounter.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/SetupPayload.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>
using namespace chip::DeviceLayer;

using chip::kMinValidFabricIndex;
using chip::RendezvousInformationFlag;
using chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr;
using chip::Inet::IPAddressType;
#if CONFIG_NETWORK_LAYER_BLE
using chip::Transport::BleListenParameters;
#endif
using chip::Transport::PeerAddress;
using chip::Transport::UdpListenParameters;

namespace {

void StopEventLoop(intptr_t arg)
{
    CHIP_ERROR err = chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Stopping event loop: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

class DeviceTypeResolver : public chip::Access::AccessControl::DeviceTypeResolver
{
public:
    bool IsDeviceTypeOnEndpoint(chip::DeviceTypeId deviceType, chip::EndpointId endpoint) override
    {
        return chip::app::IsDeviceTypeOnEndpoint(deviceType, endpoint);
    }
} sDeviceTypeResolver;

} // namespace

namespace chip {

Server Server::sServer;

#if CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
#define CHIP_NUM_EVENT_LOGGING_BUFFERS 3
static uint8_t sInfoEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE];
static uint8_t sDebugEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE];
static uint8_t sCritEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE];
static ::chip::PersistedCounter<chip::EventNumber> sGlobalEventIdCounter;
static ::chip::app::CircularEventBuffer sLoggingBuffer[CHIP_NUM_EVENT_LOGGING_BUFFERS];
#endif // CHIP_CONFIG_ENABLE_SERVER_IM_EVENT

CHIP_ERROR Server::Init(const ServerInitParams & initParams)
{
    ChipLogProgress(AppServer, "Server initializing...");

    CASESessionManagerConfig caseSessionManagerConfig;
    DeviceLayer::DeviceInfoProvider * deviceInfoprovider = nullptr;

    mOperationalServicePort        = initParams.operationalServicePort;
    mUserDirectedCommissioningPort = initParams.userDirectedCommissioningPort;
    mInterfaceId                   = initParams.interfaceId;

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(initParams.persistentStorageDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.accessDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.aclStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.groupDataProvider != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    // TODO(16969): Remove chip::Platform::MemoryInit() call from Server class, it belongs to outer code
    chip::Platform::MemoryInit();

    SuccessOrExit(err = mCommissioningWindowManager.Init(this));
    mCommissioningWindowManager.SetAppDelegate(initParams.appDelegate);

    // Initialize PersistentStorageDelegate-based storage
    mDeviceStorage            = initParams.persistentStorageDelegate;
    mSessionResumptionStorage = initParams.sessionResumptionStorage;

    mCertificateValidityPolicy = initParams.certificateValidityPolicy;

    // Set up attribute persistence before we try to bring up the data model
    // handler.
    SuccessOrExit(mAttributePersister.Init(mDeviceStorage));
    SetAttributePersistenceProvider(&mAttributePersister);

    err = mFabrics.Init(mDeviceStorage);
    SuccessOrExit(err);

    SuccessOrExit(err = mAccessControl.Init(initParams.accessDelegate, sDeviceTypeResolver));
    Access::SetAccessControl(mAccessControl);

    mAclStorage = initParams.aclStorage;
    SuccessOrExit(err = mAclStorage->Init(*mDeviceStorage, mFabrics.begin(), mFabrics.end()));

    app::DnssdServer::Instance().SetFabricTable(&mFabrics);
    app::DnssdServer::Instance().SetCommissioningModeProvider(&mCommissioningWindowManager);

    mGroupsProvider = initParams.groupDataProvider;
    SetGroupDataProvider(mGroupsProvider);

    mTestEventTriggerDelegate = initParams.testEventTriggerDelegate;

    deviceInfoprovider = DeviceLayer::GetDeviceInfoProvider();
    if (deviceInfoprovider)
    {
        deviceInfoprovider->SetStorageDelegate(mDeviceStorage);
    }

    // This initializes clusters, so should come after lower level initialization.
    InitDataModelHandler(&mExchangeMgr);

    // Init transport before operations with secure session mgr.
    err = mTransports.Init(UdpListenParameters(DeviceLayer::UDPEndPointManager())
                               .SetAddressType(IPAddressType::kIPv6)
                               .SetListenPort(mOperationalServicePort)
                               .SetNativeParams(initParams.endpointNativeParams)

#if INET_CONFIG_ENABLE_IPV4
                               ,
                           UdpListenParameters(DeviceLayer::UDPEndPointManager())
                               .SetAddressType(IPAddressType::kIPv4)
                               .SetListenPort(mOperationalServicePort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                               ,
                           BleListenParameters(DeviceLayer::ConnectivityMgr().GetBleLayer())
#endif
    );

    err = mListener.Init(this);
    SuccessOrExit(err);
    mGroupsProvider->SetListener(&mListener);

#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#endif
    SuccessOrExit(err);

    err = mSessions.Init(&DeviceLayer::SystemLayer(), &mTransports, &mMessageCounterManager, mDeviceStorage, &GetFabricTable());
    SuccessOrExit(err);

    err = mFabricDelegate.Init(this);
    SuccessOrExit(err);
    mFabrics.AddFabricDelegate(&mFabricDelegate);

    err = mExchangeMgr.Init(&mSessions);
    SuccessOrExit(err);
    err = mMessageCounterManager.Init(&mExchangeMgr);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&mExchangeMgr, &GetFabricTable());
    SuccessOrExit(err);

    chip::Dnssd::Resolver::Instance().Init(DeviceLayer::UDPEndPointManager());

#if CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
    // Initialize event logging subsystem
    err = sGlobalEventIdCounter.Init(mDeviceStorage, &DefaultStorageKeyAllocator::IMEventNumber,
                                     CHIP_DEVICE_CONFIG_EVENT_ID_COUNTER_EPOCH);
    SuccessOrExit(err);

    {
        ::chip::app::LogStorageResources logStorageResources[] = {
            { &sDebugEventBuffer[0], sizeof(sDebugEventBuffer), ::chip::app::PriorityLevel::Debug },
            { &sInfoEventBuffer[0], sizeof(sInfoEventBuffer), ::chip::app::PriorityLevel::Info },
            { &sCritEventBuffer[0], sizeof(sCritEventBuffer), ::chip::app::PriorityLevel::Critical }
        };

        chip::app::EventManagement::GetInstance().Init(&mExchangeMgr, CHIP_NUM_EVENT_LOGGING_BUFFERS, &sLoggingBuffer[0],
                                                       &logStorageResources[0], &sGlobalEventIdCounter);
    }
#endif // CHIP_CONFIG_ENABLE_SERVER_IM_EVENT

#if defined(CHIP_APP_USE_ECHO)
    err = InitEchoHandler(&mExchangeMgr);
    SuccessOrExit(err);
#endif

    app::DnssdServer::Instance().SetSecuredPort(mOperationalServicePort);
    app::DnssdServer::Instance().SetUnsecuredPort(mUserDirectedCommissioningPort);
    app::DnssdServer::Instance().SetInterfaceId(mInterfaceId);

    if (GetFabricTable().FabricCount() != 0)
    {
        // The device is already commissioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Fabric already commissioned. Disabling BLE advertisement");
#if CONFIG_NETWORK_LAYER_BLE
        chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
#endif
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
        GetFabricTable().DeleteAllFabrics();
        SuccessOrExit(err = mCommissioningWindowManager.OpenBasicCommissioningWindow());
#endif
    }

    // TODO @bzbarsky-apple @cecille Move to examples
    // ESP32 and Mbed OS examples have a custom logic for enabling DNS-SD
#if !CHIP_DEVICE_LAYER_TARGET_ESP32 && !CHIP_DEVICE_LAYER_TARGET_MBED &&                                                           \
    (!CHIP_DEVICE_LAYER_TARGET_AMEBA || !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE)
    // StartServer only enables commissioning mode if device has not been commissioned
    app::DnssdServer::Instance().StartServer();
#endif

    caseSessionManagerConfig = {
        .sessionInitParams =  {
            .sessionManager    = &mSessions,
            .sessionResumptionStorage = mSessionResumptionStorage,
            .certificateValidityPolicy = mCertificateValidityPolicy,
            .exchangeMgr       = &mExchangeMgr,
            .fabricTable       = &mFabrics,
            .clientPool        = &mCASEClientPool,
            .groupDataProvider = mGroupsProvider,
        },
        .devicePool        = &mDevicePool,
    };

    err = mCASESessionManager.Init(&DeviceLayer::SystemLayer(), caseSessionManagerConfig);
    SuccessOrExit(err);

    err = mCASEServer.ListenForSessionEstablishment(&mExchangeMgr, &mSessions, &mFabrics, mSessionResumptionStorage,
                                                    mCertificateValidityPolicy, mGroupsProvider);
    SuccessOrExit(err);

    // This code is necessary to restart listening to existing groups after a reboot
    // Each manufacturer needs to validate that they can rejoin groups by placing this code at the appropriate location for them
    //
    // Thread LWIP devices using dedicated Inet endpoint implementations are excluded because they call this function from:
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp
#if !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    RejoinExistingMulticastGroups();
#endif // !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

    PlatformMgr().HandleServerStarted();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        // NOTE: this log is scraped by the test harness.
        ChipLogProgress(AppServer, "Server Listening...");
    }
    return err;
}

void Server::RejoinExistingMulticastGroups()
{
    ChipLogProgress(AppServer, "Joining Multicast groups");
    CHIP_ERROR err = CHIP_NO_ERROR;
    for (const FabricInfo & fabric : mFabrics)
    {
        Credentials::GroupDataProvider::GroupInfo groupInfo;

        auto * iterator = mGroupsProvider->IterateGroupInfo(fabric.GetFabricIndex());
        if (iterator)
        {
            // GroupDataProvider was able to allocate rescources for an iterator
            while (iterator->Next(groupInfo))
            {
                err = mTransports.MulticastGroupJoinLeave(
                    Transport::PeerAddress::Multicast(fabric.GetFabricId(), groupInfo.group_id), true);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "Error when trying to join Group %u of fabric index %u : %" CHIP_ERROR_FORMAT,
                                 groupInfo.group_id, fabric.GetFabricIndex(), err.Format());

                    // We assume the failure is caused by a network issue or a lack of rescources; neither of which will be solved
                    // before the next join. Exit the loop to save rescources.
                    iterator->Release();
                    return;
                }
            }

            iterator->Release();
        }
    }
}

void Server::DispatchShutDownAndStopEventLoop()
{
    PlatformMgr().ScheduleWork([](intptr_t) { PlatformMgr().HandleServerShuttingDown(); });
    PlatformMgr().ScheduleWork(StopEventLoop);
}

void Server::ScheduleFactoryReset()
{
    PlatformMgr().ScheduleWork([](intptr_t) {
        // Delete all fabrics and emit Leave event.
        GetInstance().GetFabricTable().DeleteAllFabrics();
        PlatformMgr().HandleServerShuttingDown();
        ConfigurationMgr().InitiateFactoryReset();
    });
}

void Server::Shutdown()
{
    mCASESessionManager.Shutdown();
    app::DnssdServer::Instance().SetCommissioningModeProvider(nullptr);
    chip::Dnssd::ServiceAdvertiser::Instance().Shutdown();

    chip::Dnssd::Resolver::Instance().Shutdown();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    mMessageCounterManager.Shutdown();
    CHIP_ERROR err = mExchangeMgr.Shutdown();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Exchange Mgr shutdown: %" CHIP_ERROR_FORMAT, err.Format());
    }
    mSessions.Shutdown();
    mTransports.Close();
    mAccessControl.Finish();
    Credentials::SetGroupDataProvider(nullptr);
    mAttributePersister.Shutdown();
    mCommissioningWindowManager.Shutdown();
    // TODO(16969): Remove chip::Platform::MemoryInit() call from Server class, it belongs to outer code
    chip::Platform::MemoryShutdown();
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
// NOTE: UDC client is located in Server.cpp because it really only makes sense
// to send UDC from a Matter device. The UDC message payload needs to include the device's
// randomly generated service name.
CHIP_ERROR Server::SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner)
{
    ChipLogDetail(AppServer, "SendUserDirectedCommissioningRequest2");

    CHIP_ERROR err;
    char nameBuffer[chip::Dnssd::Commission::kInstanceNameMaxLength + 1];
    err = app::DnssdServer::Instance().GetCommissionableInstanceName(nameBuffer, sizeof(nameBuffer));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to get mdns instance name error: %s", ErrorStr(err));
        return err;
    }
    ChipLogDetail(AppServer, "instanceName=%s", nameBuffer);

    chip::System::PacketBufferHandle payloadBuf = chip::MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    if (payloadBuf.IsNull())
    {
        ChipLogError(AppServer, "Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gUDCClient.SendUDCMessage(&mTransports, std::move(payloadBuf), commissioner);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "Send UDC request success");
    }
    else
    {
        ChipLogError(AppServer, "Send UDC request failed, err: %s\n", chip::ErrorStr(err));
    }
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

} // namespace chip
