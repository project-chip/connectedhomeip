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
#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/DefaultAttributePersistenceProvider.h>
#include <app/OperationalDeviceProxyPool.h>
#include <app/server/AppDelegate.h>
#include <app/server/CommissioningWindowManager.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProviderImpl.h>
#include <inet/InetConfig.h>
#include <lib/core/CHIPConfig.h>
#include <lib/support/SafeInt.h>
#include <messaging/ExchangeMgr.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <transport/SessionManager.h>
#include <transport/TransportMgr.h>
#include <transport/TransportMgrBase.h>
#include <transport/raw/BLE.h>
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

class Server
{
public:
    CHIP_ERROR Init(AppDelegate * delegate = nullptr, uint16_t secureServicePort = CHIP_PORT,
                    uint16_t unsecureServicePort = CHIP_UDC_PORT);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CHIP_ERROR AddTestCommissioning();

    FabricTable & GetFabricTable() { return mFabrics; }

    CASESessionManager * GetCASESessionManager() { return &mCASESessionManager; }

    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeMgr; }

    SessionIDAllocator & GetSessionIDAllocator() { return mSessionIDAllocator; }

    SessionManager & GetSecureSessionManager() { return mSessions; }

    TransportMgrBase & GetTransportManager() { return mTransports; }

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * GetBleLayerObject() { return mBleLayer; }
#endif

    CommissioningWindowManager & GetCommissioningWindowManager() { return mCommissioningWindowManager; }

    /**
     * This function send the ShutDown event before stopping
     * the event loop.
     */
    void DispatchShutDownAndStopEventLoop();

    void Shutdown();

    static Server & GetInstance() { return sServer; }

private:
    Server();

    static Server sServer;

    class DeviceStorageDelegate : public PersistentStorageDelegate, public FabricStorage
    {
        CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
        {
            size_t bytesRead;
            ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size, &bytesRead));
            if (!CanCastTo<uint16_t>(bytesRead))
            {
                ChipLogDetail(AppServer, "0x%" PRIx32 " is too big to fit in uint16_t", static_cast<uint32_t>(bytesRead));
                return CHIP_ERROR_BUFFER_TOO_SMALL;
            }
            ChipLogProgress(AppServer, "Retrieved from server storage: %s", key);
            size = static_cast<uint16_t>(bytesRead);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
        {
            return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
        }

        CHIP_ERROR SyncDeleteKeyValue(const char * key) override
        {
            return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
        }

        CHIP_ERROR SyncStore(FabricIndex fabricIndex, const char * key, const void * buffer, uint16_t size) override
        {
            return SyncSetKeyValue(key, buffer, size);
        };

        CHIP_ERROR SyncLoad(FabricIndex fabricIndex, const char * key, void * buffer, uint16_t & size) override
        {
            return SyncGetKeyValue(key, buffer, size);
        };

        CHIP_ERROR SyncDelete(FabricIndex fabricIndex, const char * key) override { return SyncDeleteKeyValue(key); };
    };

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

    // Both PersistentStorageDelegate, and GroupDataProvider should be injected by the applications
    // See: https://github.com/project-chip/connectedhomeip/issues/12276
    DeviceStorageDelegate mDeviceStorage;
    Credentials::GroupDataProviderImpl mGroupsProvider;
    app::DefaultAttributePersistenceProvider mAttributePersister;
    GroupDataProviderListener mListener;

    Access::AccessControl mAccessControl;

    // TODO @ceille: Maybe use OperationalServicePort and CommissionableServicePort
    uint16_t mSecuredServicePort;
    uint16_t mUnsecuredServicePort;
};

} // namespace chip
