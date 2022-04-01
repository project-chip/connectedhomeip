/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <access/AccessControl.h>
#include <access/examples/ExampleAccessControlDelegate.h>
#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/DefaultAttributePersistenceProvider.h>
#include <app/OperationalDeviceProxyPool.h>
#include <app/server/AppDelegate.h>
#include <app/server/CommissioningWindowManager.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <inet/InetConfig.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SafeInt.h>
#include <messaging/ExchangeMgr.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/KvsPersistentStorageDelegate.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/TransportMgrBase.h>
#if CONFIG_NETWORK_LAYER_BLE
#include <transport/raw/BLE.h>
#endif
#include <transport/raw/UDP.h>

namespace chip {

constexpr size_t kMaxBlePendingPackets = 1;

using ServerTransportMgr = chip::TransportMgr<chip::Transport::UDP
#if INET_CONFIG_ENABLE_IPV4
                                              ,
                                              chip::Transport::UDP
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                              ,
                                              chip::Transport::BLE<kMaxBlePendingPackets>
#endif
                                              >;

struct ServerInitParams
{
    ServerInitParams()          = default;
    virtual ~ServerInitParams() = default;

    // Not copyable
    ServerInitParams(const ServerInitParams &) = delete;
    ServerInitParams & operator=(const ServerInitParams &) = delete;

    // App delegate to handle some lifecycle events
    AppDelegate * appDelegate = nullptr;
    // Port to use for Matter commissioning/operational traffic
    uint16_t operationalServicePort = CHIP_PORT;
    // Port to use for UDC if supported
    uint16_t userDirectedCommissioningPort = CHIP_UDC_PORT;
    // Interface on which to run daemon
    Inet::InterfaceId interfaceId = Inet::InterfaceId::Null();

    // Persistent storage delegate: MUST be injected. Used to maintain storage by much common code.
    PersistentStorageDelegate * persistentStorageDelegate = nullptr;
    // Group data provider: MUST be injected. Used to maintain critical keys such as the Identity
    // Protection Key (IPK) for case
    Credentials::GroupDataProvider * groupDataProvider = nullptr;
    // Access control delegate: MUST be injected. Used to look-up access control rules
    Access::AccessControl::Delegate * accessDelegate = nullptr;
};

// Transitional pre-injection version of ServierInitParams
struct CommonCaseDeviceServerInitParams : public ServerInitParams
{
    CommonCaseDeviceServerInitParams() = default;

    // Not copyable
    CommonCaseDeviceServerInitParams(const CommonCaseDeviceServerInitParams &) = delete;
    CommonCaseDeviceServerInitParams & operator=(const CommonCaseDeviceServerInitParams &) = delete;

    virtual CHIP_ERROR InitBeforeServerInit()
    {
        static chip::KvsPersistentStorageDelegate sKvsPersistenStorageDelegate;
        static chip::Credentials::GroupDataProviderImpl sGroupDataProvider;

        // KVS-based persistent storage delegate injection
        chip::DeviceLayer::PersistedStorage::KeyValueStoreManager & kvsManager = DeviceLayer::PersistedStorage::KeyValueStoreMgr();
        ReturnErrorOnFailure(sKvsPersistenStorageDelegate.Init(&kvsManager));
        this->persistentStorageDelegate = &sKvsPersistenStorageDelegate;

        // Group Data provider injection
        sGroupDataProvider.SetStorageDelegate(&sKvsPersistenStorageDelegate);
        ReturnErrorOnFailure(sGroupDataProvider.Init());
        this->groupDataProvider = &sGroupDataProvider;

        // Inject access control delegate
        this->accessDelegate = Access::Examples::GetAccessControlDelegate(&sKvsPersistenStorageDelegate);

        return CHIP_NO_ERROR;
    }

    virtual void Shutdown()
    {
        if (accessDelegate != nullptr)
        {
            accessDelegate->Finish();
        }
        if (groupDataProvider != nullptr)
        {
            groupDataProvider->Finish();
        }
    }
};

class Server
{
public:
    CHIP_ERROR Init(const ServerInitParams & initParams);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    /**
     * @brief Call this function to rejoin existing groups found in the GroupDataProvider
     */
    void RejoinExistingMulticastGroups();

    FabricTable & GetFabricTable() { return mFabrics; }

    CASESessionManager * GetCASESessionManager() { return &mCASESessionManager; }

    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeMgr; }

    SessionIDAllocator & GetSessionIDAllocator() { return mSessionIDAllocator; }

    SessionManager & GetSecureSessionManager() { return mSessions; }

    TransportMgrBase & GetTransportManager() { return mTransports; }

    Credentials::GroupDataProvider * GetGroupDataProvider() { return mGroupsProvider; }

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * GetBleLayerObject() { return mBleLayer; }
#endif

    CommissioningWindowManager & GetCommissioningWindowManager() { return mCommissioningWindowManager; }

    PersistentStorageDelegate & GetPersistentStorage() { return *mDeviceStorage; }

    /**
     * This function send the ShutDown event before stopping
     * the event loop.
     */
    void DispatchShutDownAndStopEventLoop();

    void Shutdown();

    void ScheduleFactoryReset();

    static Server & GetInstance() { return sServer; }

private:
    Server() = default;

    static Server sServer;

    class GroupDataProviderListener final : public Credentials::GroupDataProvider::GroupListener
    {
    public:
        GroupDataProviderListener() {}

        CHIP_ERROR Init(ServerTransportMgr * transports)
        {
            VerifyOrReturnError(transports != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            mTransports = transports;
            return CHIP_NO_ERROR;
        };

        void OnGroupAdded(chip::FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & new_group) override
        {
            if (mTransports->MulticastGroupJoinLeave(Transport::PeerAddress::Multicast(fabric_index, new_group.group_id), true) !=
                CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Unable to listen to group");
            }
        };

        void OnGroupRemoved(chip::FabricIndex fabric_index, const Credentials::GroupDataProvider::GroupInfo & old_group) override
        {
            mTransports->MulticastGroupJoinLeave(Transport::PeerAddress::Multicast(fabric_index, old_group.group_id), false);
        };

    private:
        ServerTransportMgr * mTransports;
    };

    class ServerFabricDelegate final : public FabricTableDelegate
    {
    public:
        ServerFabricDelegate() {}

        CHIP_ERROR Init(Server * server)
        {
            VerifyOrReturnError(server != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            mServer = server;
            return CHIP_NO_ERROR;
        };

        void OnFabricDeletedFromStorage(CompressedFabricId compressedId, FabricIndex fabricIndex) override
        {
            (void) compressedId;
            auto & sessionManager = mServer->GetSecureSessionManager();
            sessionManager.FabricRemoved(fabricIndex);

            Credentials::GroupDataProvider * groupDataProvider = mServer->GetGroupDataProvider();
            if (groupDataProvider != nullptr)
            {
                groupDataProvider->RemoveFabric(fabricIndex);
            }
            Access::GetAccessControl().RemoveFabric(fabricIndex);
        };
        void OnFabricRetrievedFromStorage(FabricInfo * fabricInfo) override { (void) fabricInfo; }

        void OnFabricPersistedToStorage(FabricInfo * fabricInfo) override { (void) fabricInfo; }

    private:
        Server * mServer = nullptr;
    };

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif

    ServerTransportMgr mTransports;
    SessionManager mSessions;
    CASEServer mCASEServer;

    CASESessionManager mCASESessionManager;
    CASEClientPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_CASE_CLIENTS> mCASEClientPool;
    OperationalDeviceProxyPool<CHIP_CONFIG_DEVICE_MAX_ACTIVE_DEVICES> mDevicePool;

    Messaging::ExchangeManager mExchangeMgr;
    FabricTable mFabrics;
    SessionIDAllocator mSessionIDAllocator;
    secure_channel::MessageCounterManager mMessageCounterManager;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    chip::Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient gUDCClient;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    SecurePairingUsingTestSecret mTestPairing;
    CommissioningWindowManager mCommissioningWindowManager;

    PersistentStorageDelegate * mDeviceStorage;
    Credentials::GroupDataProvider * mGroupsProvider;
    app::DefaultAttributePersistenceProvider mAttributePersister;
    GroupDataProviderListener mListener;
    ServerFabricDelegate mFabricDelegate;

    Access::AccessControl mAccessControl;

    uint16_t mOperationalServicePort;
    uint16_t mUserDirectedCommissioningPort;
    Inet::InterfaceId mInterfaceId;
};

} // namespace chip
