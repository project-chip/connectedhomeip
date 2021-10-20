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

#include <app/server/AppDelegate.h>
#include <app/server/CommissioningWindowManager.h>
#include <inet/InetConfig.h>
#include <messaging/ExchangeMgr.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <transport/FabricTable.h>
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

class Server : public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR Init(AppDelegate * delegate = nullptr, uint16_t secureServicePort = CHIP_PORT,
                    uint16_t unsecureServicePort = CHIP_UDC_PORT);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    CHIP_ERROR AddTestCommissioning();

    FabricTable & GetFabricTable() { return mFabrics; }

    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeMgr; }

    SessionIDAllocator & GetSessionIDAllocator() { return mSessionIDAllocator; }

    SessionManager & GetSecureSessionManager() { return mSessions; }

    TransportMgrBase & GetTransportManager() { return mTransports; }

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * getBleLayerObject() { return mBleLayer; }
#endif

    CommissioningWindowManager & GetCommissioningWindowManager() { return mCommissioningWindowManager; }

    void Shutdown();

    static Server & GetInstance() { return sServer; }

private:
    Server() : mCommissioningWindowManager(this) {}

    static Server sServer;

    class ServerStorageDelegate : public PersistentStorageDelegate
    {
        CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
        {
            ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size));
            ChipLogProgress(AppServer, "Retrieved from server storage: %s", key);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
        {
            ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(key, value, size));
            ChipLogProgress(AppServer, "Saved into server storage: %s", key);
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR SyncDeleteKeyValue(const char * key) override
        {
            ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(key));
            ChipLogProgress(AppServer, "Deleted from server storage: %s", key);
            return CHIP_NO_ERROR;
        }
    };

    // Messaging::ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && buffer) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    AppDelegate * mAppDelegate = nullptr;

#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif

    ServerTransportMgr mTransports;
    SessionManager mSessions;
    CASEServer mCASEServer;
    Messaging::ExchangeManager mExchangeMgr;
    FabricTable mFabrics;
    SessionIDAllocator mSessionIDAllocator;
    secure_channel::MessageCounterManager mMessageCounterManager;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    chip::Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient gUDCClient;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    SecurePairingUsingTestSecret mTestPairing;

    ServerStorageDelegate mServerStorage;
    CommissioningWindowManager mCommissioningWindowManager;

    // TODO @ceille: Maybe use OperationalServicePort and CommissionableServicePort
    uint16_t mSecuredServicePort;
    uint16_t mUnsecuredServicePort;
};

} // namespace chip
