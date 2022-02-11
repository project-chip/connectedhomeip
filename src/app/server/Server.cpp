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

#include <ble/BLEEndPoint.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/SetupPayload.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

using chip::kMinValidFabricIndex;
using chip::RendezvousInformationFlag;
using chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr;
using chip::Inet::IPAddressType;
using chip::Transport::BleListenParameters;
using chip::Transport::PeerAddress;
using chip::Transport::UdpListenParameters;

namespace {

constexpr bool isRendezvousBypassed()
{
#if defined(CHIP_BYPASS_RENDEZVOUS) && CHIP_BYPASS_RENDEZVOUS
    return true;
#elif defined(CONFIG_RENDEZVOUS_MODE)
    return static_cast<RendezvousInformationFlag>(CONFIG_RENDEZVOUS_MODE) == RendezvousInformationFlag::kNone;
#else
    return false;
#endif
}

} // namespace

namespace chip {

Server Server::sServer;

#if CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
#define CHIP_NUM_EVENT_LOGGING_BUFFERS 3
static uint8_t sInfoEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE];
static uint8_t sDebugEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE];
static uint8_t sCritEventBuffer[CHIP_DEVICE_CONFIG_EVENT_LOGGING_CRIT_BUFFER_SIZE];
static ::chip::PersistedCounter sGlobalEventIdCounter;
static ::chip::app::CircularEventBuffer sLoggingBuffer[CHIP_NUM_EVENT_LOGGING_BUFFERS];
#endif // CHIP_CONFIG_ENABLE_SERVER_IM_EVENT

Server::Server() :
    mCASESessionManager(CASESessionManagerConfig {
        .sessionInitParams =  {
            .sessionManager = &mSessions,
            .exchangeMgr    = &mExchangeMgr,
            .idAllocator    = &mSessionIDAllocator,
            .fabricTable    = &mFabrics,
            .clientPool     = &mCASEClientPool,
        },
        .dnsCache          = nullptr,
        .devicePool        = &mDevicePool,
        .dnsResolver       = nullptr,
    }), mCommissioningWindowManager(this), mGroupsProvider(mDeviceStorage),
    mAttributePersister(mDeviceStorage), mAccessControl(Access::Examples::GetAccessControlDelegate(&mDeviceStorage))
{}

CHIP_ERROR Server::Init(AppDelegate * delegate, uint16_t secureServicePort, uint16_t unsecureServicePort)
{
    mSecuredServicePort   = secureServicePort;
    mUnsecuredServicePort = unsecureServicePort;

    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Platform::MemoryInit();

    mCommissioningWindowManager.SetAppDelegate(delegate);
    mCommissioningWindowManager.SetSessionIDAllocator(&mSessionIDAllocator);

    // Set up attribute persistence before we try to bring up the data model
    // handler.
    SetAttributePersistenceProvider(&mAttributePersister);

    InitDataModelHandler(&mExchangeMgr);

    err = mFabrics.Init(&mDeviceStorage);
    SuccessOrExit(err);

    // Group data provider must be initialized after mDeviceStorage
    err = mGroupsProvider.Init();
    SuccessOrExit(err);
    SetGroupDataProvider(&mGroupsProvider);

    // Access control must be initialized after mDeviceStorage.
    err = mAccessControl.Init();
    SuccessOrExit(err);
    Access::SetAccessControl(mAccessControl);

    // Init transport before operations with secure session mgr.
    err = mTransports.Init(UdpListenParameters(DeviceLayer::UDPEndPointManager())
                               .SetAddressType(IPAddressType::kIPv6)
                               .SetListenPort(mSecuredServicePort)
#if CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_UDP
                               .SetNativeParams(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance())
#endif // CHIP_SYSTEM_CONFIG_USE_OPEN_THREAD_UDP

#if INET_CONFIG_ENABLE_IPV4
                               ,
                           UdpListenParameters(DeviceLayer::UDPEndPointManager())
                               .SetAddressType(IPAddressType::kIPv4)
                               .SetListenPort(mSecuredServicePort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                               ,
                           BleListenParameters(DeviceLayer::ConnectivityMgr().GetBleLayer())
#endif
    );

    err = mListener.Init(&mTransports);
    SuccessOrExit(err);
    mGroupsProvider.SetListener(&mListener);

#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#endif
    SuccessOrExit(err);

    err = mSessions.Init(&DeviceLayer::SystemLayer(), &mTransports, &mMessageCounterManager, &mDeviceStorage);
    SuccessOrExit(err);

    err = mExchangeMgr.Init(&mSessions);
    SuccessOrExit(err);
    err = mMessageCounterManager.Init(&mExchangeMgr);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&mExchangeMgr);
    SuccessOrExit(err);

#if CHIP_CONFIG_ENABLE_SERVER_IM_EVENT
    // Initialize event logging subsystem
    {
        ::chip::Platform::PersistedStorage::Key globalEventIdCounterStorageKey =
            CHIP_DEVICE_CONFIG_PERSISTED_STORAGE_GLOBAL_EIDC_KEY;

        ::chip::app::LogStorageResources logStorageResources[] = {
            { &sDebugEventBuffer[0], sizeof(sDebugEventBuffer), ::chip::app::PriorityLevel::Debug },
            { &sInfoEventBuffer[0], sizeof(sInfoEventBuffer), ::chip::app::PriorityLevel::Info },
            { &sCritEventBuffer[0], sizeof(sCritEventBuffer), ::chip::app::PriorityLevel::Critical }
        };

        chip::app::EventManagement::GetInstance().Init(&mExchangeMgr, CHIP_NUM_EVENT_LOGGING_BUFFERS, &sLoggingBuffer[0],
                                                       &logStorageResources[0], &globalEventIdCounterStorageKey,
                                                       CHIP_DEVICE_CONFIG_EVENT_ID_COUNTER_EPOCH, &sGlobalEventIdCounter);
    }
#endif // CHIP_CONFIG_ENABLE_SERVER_IM_EVENT

#if defined(CHIP_APP_USE_ECHO)
    err = InitEchoHandler(&gExchangeMgr);
    SuccessOrExit(err);
#endif

    if (isRendezvousBypassed())
    {
        ChipLogProgress(AppServer, "Rendezvous and secure pairing skipped");
        SuccessOrExit(err = AddTestCommissioning());
    }
    else if (GetFabricTable().FabricCount() != 0)
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

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    app::DnssdServer::Instance().SetSecuredPort(mSecuredServicePort);
    app::DnssdServer::Instance().SetUnsecuredPort(mUnsecuredServicePort);
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    // TODO @bzbarsky-apple @cecille Move to examples
    // ESP32 and Mbed OS examples have a custom logic for enabling DNS-SD
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD && !CHIP_DEVICE_LAYER_TARGET_ESP32 && !CHIP_DEVICE_LAYER_TARGET_MBED &&                        \
    (!CHIP_DEVICE_LAYER_TARGET_AMEBA || !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE)
    // StartServer only enables commissioning mode if device has not been commissioned
    app::DnssdServer::Instance().StartServer();
#endif

    err = mCASEServer.ListenForSessionEstablishment(&mExchangeMgr, &mTransports, chip::DeviceLayer::ConnectivityMgr().GetBleLayer(),
                                                    &mSessions, &mFabrics, &mSessionIDAllocator);
    SuccessOrExit(err);

    err = mCASESessionManager.Init();

    // This code is necessary to restart listening to existing groups after a reboot
    // Each manufacturer needs to validate that they can rejoin groups by placing this code at the appropriate location for them
    //
    // This is disabled for thread device because the same code is already present for thread devices in
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp
    // https://github.com/project-chip/connectedhomeip/issues/14254
#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
    {
        ChipLogProgress(AppServer, "Adding Multicast groups");
        ConstFabricIterator fabricIterator = mFabrics.cbegin();
        while (!fabricIterator.IsAtEnd())
        {
            const FabricInfo & fabric = *fabricIterator;
            Credentials::GroupDataProvider::GroupInfo groupInfo;

            Credentials::GroupDataProvider::GroupInfoIterator * iterator =
                mGroupsProvider.IterateGroupInfo(fabric.GetFabricIndex());
            while (iterator->Next(groupInfo))
            {
                err = mTransports.MulticastGroupJoinLeave(
                    Transport::PeerAddress::Multicast(fabric.GetFabricIndex(), groupInfo.group_id), true);
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "Error when trying to join Group %" PRIu16 " of fabric index %" PRIu8,
                                 groupInfo.group_id, fabric.GetFabricIndex());
                    break;
                }
            }

            fabricIterator++;
            iterator->Release();
        }
    }
#endif // !CHIP_DEVICE_CONFIG_ENABLE_THREAD
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        ChipLogProgress(AppServer, "Server Listening...");
    }
    return err;
}

void Server::Shutdown()
{
    chip::Dnssd::ServiceAdvertiser::Instance().Shutdown();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    mExchangeMgr.Shutdown();
    mSessions.Shutdown();
    mTransports.Close();
    mCommissioningWindowManager.Shutdown();
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

CHIP_ERROR Server::AddTestCommissioning()
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    PASESession * testSession = nullptr;
    PASESessionSerializable serializedTestSession;
    SessionHolder session;

    mTestPairing.ToSerializable(serializedTestSession);

    testSession = chip::Platform::New<PASESession>();
    testSession->FromSerializable(serializedTestSession);
    SuccessOrExit(err = mSessions.NewPairing(session, Optional<PeerAddress>{ PeerAddress::Uninitialized() },
                                             chip::kTestControllerNodeId, testSession, CryptoContext::SessionRole::kResponder,
                                             kMinValidFabricIndex));

exit:
    if (testSession)
    {
        testSession->Clear();
        chip::Platform::Delete(testSession);
    }

    if (err != CHIP_NO_ERROR)
    {
        mFabrics.ReleaseFabricIndex(kMinValidFabricIndex);
    }
    return err;
}

} // namespace chip
