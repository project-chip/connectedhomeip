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

class Server
{
public:
    CHIP_ERROR Init(AppDelegate * delegate = nullptr, uint16_t secureServicePort = CHIP_PORT,
                    uint16_t unsecureServicePort = CHIP_UDC_PORT, Inet::InterfaceId interfaceId = Inet::InterfaceId::Null());

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CHIP_ERROR AddTestCommissioning();

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

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * GetBleLayerObject() { return mBleLayer; }
#endif

    CommissioningWindowManager & GetCommissioningWindowManager() { return mCommissioningWindowManager; }

    PersistentStorageDelegate & GetPersistentStorage() { return mDeviceStorage; }

    /**
     * This function send the ShutDown event before stopping
     * the event loop.
     */
    void DispatchShutDownAndStopEventLoop();

    void Shutdown();

    void ScheduleFactoryReset();

    static Server & GetInstance() { return sServer; }

private:
    Server();

    static Server sServer;

    class DeviceStorageDelegate : public PersistentStorageDelegate
    {
        CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
        {
            uint8_t emptyPlaceholder = 0;
            if (buffer == nullptr)
            {
                if (size != 0)
                {
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }

                // When size is zero, let's give a non-nullptr to the KVS backend
                buffer = &emptyPlaceholder;
            }

            size_t bytesRead = 0;
            CHIP_ERROR err   = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size, &bytesRead);

            // Update size only if it made sense
            if ((CHIP_ERROR_BUFFER_TOO_SMALL == err) || (CHIP_NO_ERROR == err))
            {
                size = CanCastTo<uint16_t>(bytesRead) ? static_cast<uint16_t>(bytesRead) : 0;
            }

            if (err == CHIP_NO_ERROR)
            {
                ChipLogProgress(AppServer, "Retrieved from server storage: %s", key);
            }

            return err;
        }

        CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
        {
            uint8_t placeholderForEmpty = 0;
            if (value == nullptr)
            {
                if (size == 0)
                {
                    value = &placeholderForEmpty;
                }
                else
                {
                    return CHIP_ERROR_INVALID_ARGUMENT;
                }
            }
            return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
        }

        CHIP_ERROR SyncDeleteKeyValue(const char * key) override
        {
            return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key);
        }
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

    class ServerFabricDelegate final : public FabricTableDelegate
    {
    public:
        ServerFabricDelegate() {}

        CHIP_ERROR Init(SessionManager * sessionManager)
        {
            VerifyOrReturnError(sessionManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

            mSessionManager = sessionManager;
            return CHIP_NO_ERROR;
        };

        void OnFabricDeletedFromStorage(CompressedFabricId compressedId, FabricIndex fabricIndex) override
        {
            (void) compressedId;
            if (mSessionManager != nullptr)
            {
                mSessionManager->FabricRemoved(fabricIndex);
            }
            Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
            if (groupDataProvider != nullptr)
            {
                groupDataProvider->RemoveFabric(fabricIndex);
            }
        };
        void OnFabricRetrievedFromStorage(FabricInfo * fabricInfo) override { (void) fabricInfo; }

        void OnFabricPersistedToStorage(FabricInfo * fabricInfo) override { (void) fabricInfo; }

    private:
        SessionManager * mSessionManager = nullptr;
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
    ServerFabricDelegate mFabricDelegate;

    Access::AccessControl mAccessControl;

    // TODO @ceille: Maybe use OperationalServicePort and CommissionableServicePort
    uint16_t mSecuredServicePort;
    uint16_t mUnsecuredServicePort;
    Inet::InterfaceId mInterfaceId;
};

} // namespace chip
