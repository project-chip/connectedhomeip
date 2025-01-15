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

#include <access/ProviderDeviceTypeResolver.h>
#include <access/examples/ExampleAccessControlDelegate.h>

#include <app/AppConfig.h>
#include <app/EventManagement.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/data-model-provider/Provider.h>
#include <app/server/Dnssd.h>
#include <app/server/EchoHandler.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#endif
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/PersistedCounter.h>
#include <lib/support/TestGroupData.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceControlServer.h>
#include <platform/DeviceInfoProvider.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/LockTracker.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif
#include <setup_payload/SetupPayload.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <transport/raw/WiFiPAF.h>
#endif

#if defined(CHIP_SUPPORT_ENABLE_STORAGE_API_AUDIT) || defined(CHIP_SUPPORT_ENABLE_STORAGE_LOAD_TEST_AUDIT)
#include <lib/support/PersistentStorageAudit.h>
#endif // defined(CHIP_SUPPORT_ENABLE_STORAGE_API_AUDIT) || defined(CHIP_SUPPORT_ENABLE_STORAGE_LOAD_TEST_AUDIT)

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
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
using chip::Transport::TcpListenParameters;
#endif

namespace {

chip::Access::DynamicProviderDeviceTypeResolver sDeviceTypeResolver([] {
    return chip::app::InteractionModelEngine::GetInstance()->GetDataModelProvider();
});

} // namespace

namespace chip {

Server Server::sServer;

#if CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
#define CHIP_NUM_EVENT_LOGGING_BUFFERS 3
static uint8_t sInfoEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE];
static uint8_t sDebugEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE];
static uint8_t sCritEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE];
static PersistedCounter<EventNumber> sGlobalEventIdCounter;
static app::CircularEventBuffer sLoggingBuffer[CHIP_NUM_EVENT_LOGGING_BUFFERS];
#endif // CHIP_CONFIG_ENABLE_SERVER_IM_EVENT

CHIP_ERROR Server::Init(const ServerInitParams & initParams)
{
    ChipLogProgress(AppServer, "Server initializing...");
    assertChipStackLockedByCurrentThread();

    mInitTimestamp = System::SystemClock().GetMonotonicMicroseconds64();

    CASESessionManagerConfig caseSessionManagerConfig;
    DeviceLayer::DeviceInfoProvider * deviceInfoprovider = nullptr;

    mOperationalServicePort        = initParams.operationalServicePort;
    mUserDirectedCommissioningPort = initParams.userDirectedCommissioningPort;
    mInterfaceId                   = initParams.interfaceId;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    auto tcpListenParams = TcpListenParameters(DeviceLayer::TCPEndPointManager())
                               .SetAddressType(IPAddressType::kIPv6)
                               .SetListenPort(mOperationalServicePort);
#endif

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(initParams.persistentStorageDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.accessDelegate != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.aclStorage != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.groupDataProvider != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.sessionKeystore != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.operationalKeystore != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.opCertStore != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(initParams.reportScheduler != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Extra log since this is an incremental requirement and existing applications may not be aware
    if (initParams.dataModelProvider == nullptr)
    {
        ChipLogError(AppServer, "Application Server requires a `initParams.dataModelProvider` value.");
        ChipLogError(AppServer, "For backwards compatibility, you likely can use `CodegenDataModelProviderInstance(...)`");
    }

    VerifyOrExit(initParams.dataModelProvider != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    // TODO(16969): Remove Platform::MemoryInit() call from Server class, it belongs to outer code
    Platform::MemoryInit();

    // Initialize PersistentStorageDelegate-based storage
    mDeviceStorage                 = initParams.persistentStorageDelegate;
    mSessionResumptionStorage      = initParams.sessionResumptionStorage;
    mSubscriptionResumptionStorage = initParams.subscriptionResumptionStorage;
    mOperationalKeystore           = initParams.operationalKeystore;
    mOpCertStore                   = initParams.opCertStore;
    mSessionKeystore               = initParams.sessionKeystore;

    if (initParams.certificateValidityPolicy)
    {
        mCertificateValidityPolicy.Init(initParams.certificateValidityPolicy);
    }
    else
    {
        mCertificateValidityPolicy.Init(&sDefaultCertValidityPolicy);
    }

#if defined(CHIP_SUPPORT_ENABLE_STORAGE_API_AUDIT)
    VerifyOrDie(audit::ExecutePersistentStorageApiAudit(*mDeviceStorage));
#endif

#if defined(CHIP_SUPPORT_ENABLE_STORAGE_LOAD_TEST_AUDIT)
    VerifyOrDie(audit::ExecutePersistentStorageLoadTestAudit(*mDeviceStorage));
#endif

    // Set up attribute persistence before we try to bring up the data model
    // handler.
    SuccessOrExit(err = mAttributePersister.Init(mDeviceStorage));
    SetSafeAttributePersistenceProvider(&mAttributePersister);

    {
        FabricTable::InitParams fabricTableInitParams;
        fabricTableInitParams.storage             = mDeviceStorage;
        fabricTableInitParams.operationalKeystore = mOperationalKeystore;
        fabricTableInitParams.opCertStore         = mOpCertStore;

        err = mFabrics.Init(fabricTableInitParams);
        SuccessOrExit(err);
    }

    SuccessOrExit(err = mAccessControl.Init(initParams.accessDelegate, sDeviceTypeResolver));
    Access::SetAccessControl(mAccessControl);

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    if (initParams.accessRestrictionProvider != nullptr)
    {
        mAccessControl.SetAccessRestrictionProvider(initParams.accessRestrictionProvider);
    }
#endif

    mAclStorage = initParams.aclStorage;
    SuccessOrExit(err = mAclStorage->Init(*mDeviceStorage, mFabrics.begin(), mFabrics.end()));

    mGroupsProvider = initParams.groupDataProvider;
    SetGroupDataProvider(mGroupsProvider);

    mReportScheduler = initParams.reportScheduler;

    mTestEventTriggerDelegate = initParams.testEventTriggerDelegate;
    if (mTestEventTriggerDelegate == nullptr)
    {
        ChipLogProgress(AppServer, "WARNING: mTestEventTriggerDelegate is null");
    }

    deviceInfoprovider = DeviceLayer::GetDeviceInfoProvider();
    if (deviceInfoprovider)
    {
        deviceInfoprovider->SetStorageDelegate(mDeviceStorage);
    }

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
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                               ,
                           tcpListenParams
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
                           ,
                           Transport::WiFiPAFListenParameters(DeviceLayer::ConnectivityMgr().GetWiFiPAF())
#endif
    );

    SuccessOrExit(err);
    err = mListener.Init(this);
    SuccessOrExit(err);
    mGroupsProvider->SetListener(&mListener);

#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#endif
    SuccessOrExit(err);

    err = mSessions.Init(&DeviceLayer::SystemLayer(), &mTransports, &mMessageCounterManager, mDeviceStorage, &GetFabricTable(),
                         *mSessionKeystore);
    SuccessOrExit(err);

    err = mFabricDelegate.Init(this);
    SuccessOrExit(err);
    mFabrics.AddFabricDelegate(&mFabricDelegate);

    err = mExchangeMgr.Init(&mSessions);
    SuccessOrExit(err);
    err = mMessageCounterManager.Init(&mExchangeMgr);
    SuccessOrExit(err);

    err = mUnsolicitedStatusHandler.Init(&mExchangeMgr);
    SuccessOrExit(err);

    SuccessOrExit(err = mCommissioningWindowManager.Init(this));
    mCommissioningWindowManager.SetAppDelegate(initParams.appDelegate);

    app::DnssdServer::Instance().SetFabricTable(&mFabrics);
    app::DnssdServer::Instance().SetCommissioningModeProvider(&mCommissioningWindowManager);

    Dnssd::Resolver::Instance().Init(DeviceLayer::UDPEndPointManager());

#if CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
    // Initialize event logging subsystem
    err = sGlobalEventIdCounter.Init(mDeviceStorage, DefaultStorageKeyAllocator::IMEventNumber(),
                                     CHIP_DEVICE_CONFIG_EVENT_ID_COUNTER_EPOCH);
    SuccessOrExit(err);

    {
        app::LogStorageResources logStorageResources[] = {
            { &sDebugEventBuffer[0], sizeof(sDebugEventBuffer), app::PriorityLevel::Debug },
            { &sInfoEventBuffer[0], sizeof(sInfoEventBuffer), app::PriorityLevel::Info },
            { &sCritEventBuffer[0], sizeof(sCritEventBuffer), app::PriorityLevel::Critical }
        };

        err = app::EventManagement::GetInstance().Init(&mExchangeMgr, CHIP_NUM_EVENT_LOGGING_BUFFERS, &sLoggingBuffer[0],
                                                       &logStorageResources[0], &sGlobalEventIdCounter,
                                                       std::chrono::duration_cast<System::Clock::Milliseconds64>(mInitTimestamp),
                                                       &app::InteractionModelEngine::GetInstance()->GetReportingEngine());

        SuccessOrExit(err);
    }
#endif // CHIP_CONFIG_ENABLE_SERVER_IM_EVENT

    // SetDataModelProvider() initializes and starts the provider, which in turn
    // triggers the initialization of cluster implementations. This callsite is
    // critical because it ensures that cluster-level initialization occurs only
    // after all necessary low-level dependencies have been set up.
    //
    // Ordering guarantees:
    // 1) Provider initialization (under SetDataModelProvider) must happen after
    //    SetSafeAttributePersistenceProvider to ensure the provider can leverage
    //    the safe persistence provider for attribute persistence logic.
    // 2) It must occur after all low-level components that cluster implementations
    //    might depend on have been initialized, as they rely on these components
    //    during their own initialization.
    //
    // This remains the single point of entry to ensure that all cluster-level
    // initialization is performed in the correct order.
    app::InteractionModelEngine::GetInstance()->SetDataModelProvider(initParams.dataModelProvider);

#if defined(CHIP_APP_USE_ECHO)
    err = InitEchoHandler(&mExchangeMgr);
    SuccessOrExit(err);
#endif

    //
    // We need to advertise the port that we're listening to for unsolicited messages over UDP. However, we have both a IPv4
    // and IPv6 endpoint to pick from. Given that the listen port passed in may be set to 0 (which then has the kernel select
    // a valid port at bind time), that will result in two possible ports being provided back from the resultant endpoint
    // initializations. Since IPv6 is POR for Matter, let's go ahead and pick that port.
    //
    app::DnssdServer::Instance().SetSecuredPort(mTransports.GetTransport().GetImplAtIndex<0>().GetBoundPort());

    app::DnssdServer::Instance().SetUnsecuredPort(mUserDirectedCommissioningPort);
    app::DnssdServer::Instance().SetInterfaceId(mInterfaceId);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // We set the ICDManager reference betfore calling the ICDManager init due to the init ordering limitations.
    // DnssdServer will use the default value initially and will update advertisement once ICDManager
    // init is called.
    app::DnssdServer::Instance().SetICDManager(&mICDManager);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Enable the TCP Server based on the TCPListenParameters setting.
    app::DnssdServer::Instance().SetTCPServerEnabled(tcpListenParams.IsServerListenEnabled());
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    if (GetFabricTable().FabricCount() != 0)
    {
#if CONFIG_NETWORK_LAYER_BLE
        // The device is already commissioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Fabric already commissioned. Disabling BLE advertisement");
        DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
#endif
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
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
            .certificateValidityPolicy = &mCertificateValidityPolicy,
            .exchangeMgr       = &mExchangeMgr,
            .fabricTable       = &mFabrics,
            .groupDataProvider = mGroupsProvider,
            // Don't provide an MRP local config, so each CASE initiation will use
            // the then-current value.
            .mrpLocalConfig = NullOptional,
        },
        .clientPool            = &mCASEClientPool,
        .sessionSetupPool      = &mSessionSetupPool,
    };

    err = mCASESessionManager.Init(&DeviceLayer::SystemLayer(), caseSessionManagerConfig);
    SuccessOrExit(err);

    err = mCASEServer.ListenForSessionEstablishment(&mExchangeMgr, &mSessions, &mFabrics, mSessionResumptionStorage,
                                                    &mCertificateValidityPolicy, mGroupsProvider);
    SuccessOrExit(err);

    err = app::InteractionModelEngine::GetInstance()->Init(&mExchangeMgr, &GetFabricTable(), mReportScheduler, &mCASESessionManager,
                                                           mSubscriptionResumptionStorage);
    SuccessOrExit(err);

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::InteractionModelEngine::GetInstance()->SetICDManager(&mICDManager);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    // ICD Init needs to be after data model init and InteractionModel Init
#if CHIP_CONFIG_ENABLE_ICD_SERVER

    // Register the ICDStateObservers.
    // Call register before init so that observers are notified of any state change during the init.
    // All observers are released at mICDManager.Shutdown(). They can be released individually with ReleaseObserver
    mICDManager.RegisterObserver(mReportScheduler);
    mICDManager.RegisterObserver(&app::DnssdServer::Instance());

#if CHIP_CONFIG_ENABLE_ICD_CIP
    mICDManager.SetPersistentStorageDelegate(mDeviceStorage)
        .SetFabricTable(&GetFabricTable())
        .SetSymmetricKeyStore(mSessionKeystore)
        .SetExchangeManager(&mExchangeMgr)
        .SetSubscriptionsInfoProvider(app::InteractionModelEngine::GetInstance())
        .SetICDCheckInBackOffStrategy(initParams.icdCheckInBackOffStrategy);

#endif // CHIP_CONFIG_ENABLE_ICD_CIP
    mICDManager.Init();

    // Register Test Event Trigger Handler
    if (mTestEventTriggerDelegate != nullptr)
    {
        mTestEventTriggerDelegate->AddHandler(&mICDManager);
    }

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    // This code is necessary to restart listening to existing groups after a reboot
    // Each manufacturer needs to validate that they can rejoin groups by placing this code at the appropriate location for them
    //
    // Thread LWIP devices using dedicated Inet endpoint implementations are excluded because they call this function from:
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp
#if !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    RejoinExistingMulticastGroups();
#endif // !CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT

    // Handle deferred clean-up of a previously armed fail-safe that occurred during FabricTable commit.
    // This is done at the very end since at the earlier time above when FabricTable::Init() is called,
    // the delegates could not have been registered, and the other systems were not initialized. By now,
    // everything is initialized, so we can do a deferred clean-up.
    {
        FabricIndex fabricIndexDeletedOnInit = GetFabricTable().GetDeletedFabricFromCommitMarker();
        if (fabricIndexDeletedOnInit != kUndefinedFabricIndex)
        {
            ChipLogError(AppServer, "FabricIndex 0x%x deleted due to restart while fail-safed. Processing a clean-up!",
                         static_cast<unsigned>(fabricIndexDeletedOnInit));

            // Always pretend it was an add, since being in the middle of an update currently breaks
            // the validity of the fabric table. This is expected to be extremely infrequent, so
            // this "harsher" than usual clean-up is more likely to get us in a valid state for whatever
            // remains.
            const bool addNocCalled    = true;
            const bool updateNocCalled = false;
            GetFailSafeContext().ScheduleFailSafeCleanup(fabricIndexDeletedOnInit, addNocCalled, updateNocCalled);

            // Schedule clearing of the commit marker to only occur after we have processed all fail-safe clean-up.
            // Because Matter runs a single event loop for all scheduled work, it will occur after the above has
            // taken place. If a reset occurs before we have cleaned everything up, the next boot will still
            // see the commit marker.
            PlatformMgr().ScheduleWork(
                [](intptr_t arg) {
                    Server * server = reinterpret_cast<Server *>(arg);
                    VerifyOrReturn(server != nullptr);

                    server->GetFabricTable().ClearCommitMarker();
                    ChipLogProgress(AppServer, "Cleared FabricTable pending commit marker");
                },
                reinterpret_cast<intptr_t>(this));
        }
    }

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT // support UDC port for commissioner declaration msgs
    mUdcTransportMgr = Platform::New<UdcTransportMgr>();
    ReturnErrorOnFailure(mUdcTransportMgr->Init(Transport::UdpListenParameters(DeviceLayer::UDPEndPointManager())
                                                    .SetAddressType(Inet::IPAddressType::kIPv6)
                                                    .SetListenPort(static_cast<uint16_t>(mCdcListenPort))
#if INET_CONFIG_ENABLE_IPV4
                                                    ,
                                                Transport::UdpListenParameters(DeviceLayer::UDPEndPointManager())
                                                    .SetAddressType(Inet::IPAddressType::kIPv4)
                                                    .SetListenPort(static_cast<uint16_t>(mCdcListenPort))
#endif // INET_CONFIG_ENABLE_IPV4
                                                    ));

    gUDCClient = Platform::New<Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient>();
    mUdcTransportMgr->SetSessionManager(gUDCClient);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    PlatformMgr().AddEventHandler(OnPlatformEventWrapper, reinterpret_cast<intptr_t>(this));
    PlatformMgr().HandleServerStarted();

    mIsDnssdReady = Dnssd::Resolver::Instance().IsInitialized();
    CheckServerReadyEvent();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ERROR setting up transport: %" CHIP_ERROR_FORMAT, err.Format());
    }
    else
    {
        // NOTE: this log is scraped by the test harness.
        ChipLogProgress(AppServer, "Server Listening...");
    }
    return err;
}

void Server::OnPlatformEvent(const DeviceLayer::ChipDeviceEvent & event)
{
    switch (event.Type)
    {
    case DeviceEventType::kDnssdInitialized:
        // Platform DNS-SD implementation uses kPlatformDnssdInitialized event to signal that it's ready.
        if (!mIsDnssdReady)
        {
            mIsDnssdReady = true;
            CheckServerReadyEvent();
        }
        break;
    case DeviceEventType::kServerReady:
#if CHIP_CONFIG_ENABLE_ICD_SERVER && CHIP_CONFIG_ENABLE_ICD_CIP
        // Only Trigger Check-In messages if we are not in the middle of a commissioning.
        // This check is only necessary for the first commissioiner since the kServerReady event
        // is triggered once we join the network.
        // We trigger Check-In messages before resuming subscriptions to avoid doing both.
        if (!mFailSafeContext.IsFailSafeArmed())
        {
            std::function<app::ICDManager::ShouldCheckInMsgsBeSentFunction> sendCheckInMessagesOnBootUp =
                std::bind(&Server::ShouldCheckInMsgsBeSentAtBootFunction, this, std::placeholders::_1, std::placeholders::_2);
            mICDManager.TriggerCheckInMessages(sendCheckInMessagesOnBootUp);
        }
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER && CHIP_CONFIG_ENABLE_ICD_CIP
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
        ResumeSubscriptions();
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
        break;
#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    case DeviceEventType::kThreadConnectivityChange:
        if (event.ThreadConnectivityChange.Result == kConnectivity_Established)
        {
            // Refresh Multicast listening
            ChipLogDetail(DeviceLayer, "Thread Attached updating Multicast address");
            RejoinExistingMulticastGroups();
        }
        break;
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_ENDPOINT
    default:
        break;
    }
}

void Server::CheckServerReadyEvent()
{
    // Check if all asynchronously initialized server components (currently, only DNS-SD)
    // are ready, and emit the 'server ready' event if so.
    if (mIsDnssdReady)
    {
        ChipLogProgress(AppServer, "Server initialization complete");

        ChipDeviceEvent event = { .Type = DeviceEventType::kServerReady };
        PlatformMgr().PostEventOrDie(&event);
    }
}

void Server::OnPlatformEventWrapper(const DeviceLayer::ChipDeviceEvent * event, intptr_t server)
{
    reinterpret_cast<Server *>(server)->OnPlatformEvent(*event);
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

#if CHIP_CONFIG_ENABLE_ICD_CIP
bool Server::ShouldCheckInMsgsBeSentAtBootFunction(FabricIndex aFabricIndex, NodeId subjectID)
{
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    // If at least one registration has a persisted entry, do not send Check-In message.
    // The resumption of the persisted subscription will serve the same function a check-in would have served.
    return !app::InteractionModelEngine::GetInstance()->SubjectHasPersistedSubscription(aFabricIndex, subjectID);
#else
    return true;
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

void Server::GenerateShutDownEvent()
{
    PlatformMgr().ScheduleWork([](intptr_t) { PlatformMgr().HandleServerShuttingDown(); });
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
    assertChipStackLockedByCurrentThread();
    PlatformMgr().RemoveEventHandler(OnPlatformEventWrapper, 0);
    mCASEServer.Shutdown();
    mCASESessionManager.Shutdown();
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::DnssdServer::Instance().SetICDManager(nullptr);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    app::DnssdServer::Instance().SetCommissioningModeProvider(nullptr);
    Dnssd::ServiceAdvertiser::Instance().Shutdown();

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    if (mUdcTransportMgr != nullptr)
    {
        Platform::Delete(mUdcTransportMgr);
        mUdcTransportMgr = nullptr;
    }
    if (gUDCClient != nullptr)
    {
        Platform::Delete(gUDCClient);
        gUDCClient = nullptr;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY

    Dnssd::Resolver::Instance().Shutdown();
    app::InteractionModelEngine::GetInstance()->Shutdown();
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    app::InteractionModelEngine::GetInstance()->SetICDManager(nullptr);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
    // Shut down any remaining sessions (and hence exchanges) before we do any
    // futher teardown.  CASE handshakes have been shut down already via
    // shutting down mCASESessionManager and mCASEServer above; shutting
    // down mCommissioningWindowManager will shut down any PASE handshakes we
    // have going on.
    mSessions.ExpireAllSecureSessions();
    mCommissioningWindowManager.Shutdown();
    mMessageCounterManager.Shutdown();
    mExchangeMgr.Shutdown();
    mSessions.Shutdown();
    mTransports.Close();
    mAccessControl.Finish();
    Access::ResetAccessControlToDefault();
    Credentials::SetGroupDataProvider(nullptr);
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    // Remove Test Event Trigger Handler
    if (mTestEventTriggerDelegate != nullptr)
    {
        mTestEventTriggerDelegate->RemoveHandler(&mICDManager);
    }
    mICDManager.Shutdown();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    // TODO(16969): Remove chip::Platform::MemoryInit() call from Server class, it belongs to outer code
    chip::Platform::MemoryShutdown();
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
// NOTE: UDC client is located in Server.cpp because it really only makes sense
// to send UDC from a Matter device. The UDC message payload needs to include the device's
// randomly generated service name.
CHIP_ERROR Server::SendUserDirectedCommissioningRequest(Transport::PeerAddress commissioner,
                                                        Protocols::UserDirectedCommissioning::IdentificationDeclaration & id)
{
    ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest()");

    CHIP_ERROR err;

    // only populate fields left blank by the client
    if (strlen(id.GetInstanceName()) == 0)
    {
        ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest() Instance Name not known");
        char nameBuffer[Dnssd::Commission::kInstanceNameMaxLength + 1];
        err = app::DnssdServer::Instance().GetCommissionableInstanceName(nameBuffer, sizeof(nameBuffer));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(
                AppServer,
                "Server::SendUserDirectedCommissioningRequest() Failed to get mdns instance name error: %" CHIP_ERROR_FORMAT,
                err.Format());
            return err;
        }
        id.SetInstanceName(nameBuffer);
        ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest() Instance Name set to %s", nameBuffer);
    }

    if (id.GetVendorId() == 0)
    {
        uint16_t vendorId = 0;
        if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(vendorId) != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest() Vendor ID not known");
        }
        else
        {
            id.SetVendorId(vendorId);
        }
    }

    if (id.GetProductId() == 0)
    {
        uint16_t productId = 0;
        if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(productId) != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest() Product ID not known");
        }
        else
        {
            id.SetProductId(productId);
        }
    }

    if (strlen(id.GetDeviceName()) == 0)
    {
        char deviceName[Dnssd::kKeyDeviceNameMaxLength + 1] = {};
        if (!DeviceLayer::ConfigurationMgr().IsCommissionableDeviceNameEnabled() ||
            DeviceLayer::ConfigurationMgr().GetCommissionableDeviceName(deviceName, sizeof(deviceName)) != CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest() Device Name not known");
        }
        else
        {
            id.SetDeviceName(deviceName);
        }
    }

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    if (id.GetRotatingIdLength() == 0)
    {
        AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
        uint8_t rotatingDeviceIdUniqueId[DeviceLayer::ConfigurationManager::kRotatingDeviceIDUniqueIDLength];
        MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);

        ReturnErrorOnFailure(
            DeviceLayer::GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan));
        ReturnErrorOnFailure(
            DeviceLayer::ConfigurationMgr().GetLifetimeCounter(additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter));
        additionalDataPayloadParams.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;

        uint8_t rotatingDeviceIdInternalBuffer[RotatingDeviceId::kMaxLength];
        MutableByteSpan rotatingDeviceIdBufferTemp(rotatingDeviceIdInternalBuffer);
        ReturnErrorOnFailure(AdditionalDataPayloadGenerator().generateRotatingDeviceIdAsBinary(additionalDataPayloadParams,
                                                                                               rotatingDeviceIdBufferTemp));

        id.SetRotatingId(rotatingDeviceIdInternalBuffer, RotatingDeviceId::kMaxLength);
    }
#endif

    if (id.GetCdPort() == 0)
    {
        id.SetCdPort(mCdcListenPort);
    }

    err = gUDCClient->SendUDCMessage(&mTransports, id, commissioner);

    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "Server::SendUserDirectedCommissioningRequest() Send UDC request success");
    }
    else
    {
        ChipLogError(AppServer, "Server::SendUserDirectedCommissioningRequest() Send UDC request failed, err: %" CHIP_ERROR_FORMAT,
                     err.Format());
    }
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
void Server::ResumeSubscriptions()
{
    CHIP_ERROR err = app::InteractionModelEngine::GetInstance()->ResumeSubscriptions();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Error when trying to resume subscriptions : %" CHIP_ERROR_FORMAT, err.Format());
    }
}
#endif

Credentials::IgnoreCertificateValidityPeriodPolicy Server::sDefaultCertValidityPolicy;

KvsPersistentStorageDelegate CommonCaseDeviceServerInitParams::sKvsPersistenStorageDelegate;
PersistentStorageOperationalKeystore CommonCaseDeviceServerInitParams::sPersistentStorageOperationalKeystore;
Credentials::PersistentStorageOpCertStore CommonCaseDeviceServerInitParams::sPersistentStorageOpCertStore;
Credentials::GroupDataProviderImpl CommonCaseDeviceServerInitParams::sGroupDataProvider;
app::DefaultTimerDelegate CommonCaseDeviceServerInitParams::sTimerDelegate;
app::reporting::ReportSchedulerImpl
    CommonCaseDeviceServerInitParams::sReportScheduler(&CommonCaseDeviceServerInitParams::sTimerDelegate);
#if CHIP_CONFIG_ENABLE_SESSION_RESUMPTION
SimpleSessionResumptionStorage CommonCaseDeviceServerInitParams::sSessionResumptionStorage;
#endif
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
app::SimpleSubscriptionResumptionStorage CommonCaseDeviceServerInitParams::sSubscriptionResumptionStorage;
#endif
app::DefaultAclStorage CommonCaseDeviceServerInitParams::sAclStorage;
Crypto::DefaultSessionKeystore CommonCaseDeviceServerInitParams::sSessionKeystore;
#if CHIP_CONFIG_ENABLE_ICD_CIP
app::DefaultICDCheckInBackOffStrategy CommonCaseDeviceServerInitParams::sDefaultICDCheckInBackOffStrategy;
#endif

} // namespace chip
