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
#include <app/server/RendezvousServer.h>
#include <inet/InetConfig.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#include <transport/FabricTable.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/BLE.h>
#include <transport/raw/UDP.h>

namespace chip {

constexpr size_t kMaxBlePendingPackets = 1;

using DemoTransportMgr = chip::TransportMgr<chip::Transport::UDP
#if INET_CONFIG_ENABLE_IPV4
                                            ,
                                            chip::Transport::UDP
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                            ,
                                            chip::Transport::BLE<kMaxBlePendingPackets>
#endif
                                            >;

enum class ResetFabrics
{
    kYes,
    kNo,
};

enum class PairingWindowAdvertisement
{
    kBle,
    kMdns,
};

constexpr uint16_t kNoCommissioningTimeout = UINT16_MAX;

class Server : public RendezvousAdvertisementDelegate, public PersistentStorageDelegate, public Messaging::ExchangeDelegate
{
public:
    CHIP_ERROR Init(AppDelegate * delegate = nullptr, uint16_t secureServicePort = CHIP_PORT,
                    uint16_t unsecureServicePort = CHIP_UDC_PORT);

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner);
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

    /**
     * Open the pairing window using default configured parameters.
     */
    CHIP_ERROR OpenBasicCommissioningWindow(ResetFabrics resetFabrics,
                                            uint16_t commissioningTimeoutSeconds         = kNoCommissioningTimeout,
                                            PairingWindowAdvertisement advertisementMode = chip::PairingWindowAdvertisement::kBle);

    CHIP_ERROR OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                               PASEVerifier & verifier, uint32_t iterations, chip::ByteSpan salt,
                                               uint16_t passcodeID);

    CHIP_ERROR AddTestCommissioning();

    void ClosePairingWindow();

    bool IsPairingWindowOpen();

    void SetBLE(bool ble) { mIsBLE = ble; }

    void SetFabricIndex(FabricIndex id) { mFabricIndex = id; }

    Transport::FabricTable & GetFabricTable() { return mFabrics; }

    Messaging::ExchangeManager & GetExchangManager() { return mExchangeMgr; }

    SessionIDAllocator & GetSessionIDAllocator() { return mSessionIDAllocator; }

    SecureSessionMgr & GetSecureSessionManager() { return mSessions; }

    static Server & GetServer() { return sServer; }

private:
    Server() {}

    static Server sServer;

    class DeviceDiscriminatorCache
    {
    public:
        CHIP_ERROR UpdateDiscriminator(uint16_t discriminator);

        CHIP_ERROR RestoreDiscriminator();

    private:
        bool mOriginalDiscriminatorCached = false;
        uint16_t mOriginalDiscriminator   = 0;
    };

    /// PersistentStorageDelegate implementation
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;
    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;
    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

    /// RendezvousAdvertisementDelegate implementation
    CHIP_ERROR StartAdvertisement() override;
    CHIP_ERROR StopAdvertisement() override;

    // Messaging::ExchangeDelegate
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && buffer) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    AppDelegate * mAppDelgate;

    DemoTransportMgr mTransports;
    SecureSessionMgr mSessions;
    RendezvousServer mRendezvousServer;
    CASEServer mCASEServer;
    Messaging::ExchangeManager mExchangeMgr;
    Transport::FabricTable mFabrics;
    SessionIDAllocator mSessionIDAllocator;
    secure_channel::MessageCounterManager mMessageCounterManager;
#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    chip::Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient gUDCClient;
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
    SecurePairingUsingTestSecret mTestPairing;

    DeviceDiscriminatorCache mDeviceDiscriminatorCache;

    uint16_t mSecuredServicePort;
    uint16_t mUnsecuredServicePort;
    bool mPairingWindowOpen = false;
    FabricIndex mFabricIndex;
    bool mIsBLE = true;
};

} // namespace chip
