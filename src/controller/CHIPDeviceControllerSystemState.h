/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *      DeviceControllerSystemState is a representation of all the runtime state
 *      inside of CHIP that can be shared across Controllers and Commissioners.
 *
 *      The System State assumes it's being owned by and managed by the DeviceControllerFactory.
 *      It will automatically shutdown the underlying CHIP Stack when its reference count
 *      decreases to "1".
 *
 */

#pragma once

#include <app/CASEClientPool.h>
#include <app/CASESessionManager.h>
#include <app/reporting/ReportScheduler.h>
#include <credentials/FabricTable.h>
#include <credentials/GroupDataProvider.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPConfig.h>
#include <protocols/bdx/BdxTransferServer.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <protocols/secure_channel/SimpleSessionResumptionStorage.h>
#include <protocols/secure_channel/UnsolicitedStatusHandler.h>

#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/Ble.h>
#include <transport/raw/BLE.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
#include <transport/raw/WiFiPAF.h>
#endif

namespace chip {

inline constexpr size_t kMaxDeviceTransportBlePendingPackets = 1;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
inline constexpr size_t kMaxDeviceTransportWiFiPAFPendingPackets = 1;
#endif

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
inline constexpr size_t kMaxDeviceTransportTcpActiveConnectionCount = CHIP_CONFIG_MAX_ACTIVE_TCP_CONNECTIONS;

inline constexpr size_t kMaxDeviceTransportTcpPendingPackets = CHIP_CONFIG_MAX_TCP_PENDING_PACKETS;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

using DeviceTransportMgr =
    TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                 ,
                 Transport::UDP /* IPv4 */
#endif
#if CONFIG_NETWORK_LAYER_BLE
                 ,
                 Transport::BLE<kMaxDeviceTransportBlePendingPackets> /* BLE */
#endif
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                 ,
                 Transport::TCP<kMaxDeviceTransportTcpActiveConnectionCount, kMaxDeviceTransportTcpPendingPackets>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
                 ,
                 Transport::WiFiPAF<kMaxDeviceTransportWiFiPAFPendingPackets> /* WiFiPAF */
#endif
                 >;

namespace Controller {

struct DeviceControllerSystemStateParams
{
    using SessionSetupPool = OperationalSessionSetupPool<CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_DEVICES>;
    using CASEClientPool   = chip::CASEClientPool<CHIP_CONFIG_CONTROLLER_MAX_ACTIVE_CASE_CLIENTS>;

    // Params that can outlive the DeviceControllerSystemState
    System::Layer * systemLayer                                   = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager = nullptr;
    FabricTable * fabricTable                                     = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    Transport::WiFiPAFLayer * wifipaf_layer = nullptr;
#endif
    Credentials::GroupDataProvider * groupDataProvider = nullptr;
    Crypto::SessionKeystore * sessionKeystore          = nullptr;

    // NOTE: Exactly one of externalSessionResumptionStorage (externally provided,
    // externally owned) or ownedSessionResumptionStorage (managed by the system
    // state) must be non-null.
    SessionResumptionStorage * externalSessionResumptionStorage = nullptr;

    // Params that will be deallocated via Platform::Delete in
    // DeviceControllerSystemState::Shutdown.
    DeviceTransportMgr * transportMgr = nullptr;
    // NOTE: Exactly one of externalSessionResumptionStorage (externally provided,
    // externally owned) or ownedSessionResumptionStorage (managed by the system
    // state) must be non-null.
    Platform::UniquePtr<SimpleSessionResumptionStorage> ownedSessionResumptionStorage;
    Credentials::CertificateValidityPolicy * certificateValidityPolicy            = nullptr;
    SessionManager * sessionMgr                                                   = nullptr;
    Protocols::SecureChannel::UnsolicitedStatusHandler * unsolicitedStatusHandler = nullptr;
    Messaging::ExchangeManager * exchangeMgr                                      = nullptr;
    secure_channel::MessageCounterManager * messageCounterManager                 = nullptr;
    bdx::BDXTransferServer * bdxTransferServer                                    = nullptr;
    CASEServer * caseServer                                                       = nullptr;
    CASESessionManager * caseSessionManager                                       = nullptr;
    SessionSetupPool * sessionSetupPool                                           = nullptr;
    CASEClientPool * caseClientPool                                               = nullptr;
    FabricTable::Delegate * fabricTableDelegate                                   = nullptr;
    chip::app::reporting::ReportScheduler::TimerDelegate * timerDelegate          = nullptr;
    chip::app::reporting::ReportScheduler * reportScheduler                       = nullptr;
};

// A representation of the internal state maintained by the DeviceControllerFactory.
//
// This class automatically maintains a count of active device controllers and
// shuts down Matter when there are none remaining.
//
// NB: Lifetime of the object itself is not managed by reference counting; it is
// owned by DeviceControllerFactory.
class DeviceControllerSystemState
{
    using SessionSetupPool = DeviceControllerSystemStateParams::SessionSetupPool;
    using CASEClientPool   = DeviceControllerSystemStateParams::CASEClientPool;

public:
    ~DeviceControllerSystemState()
    {
        // We could get here if a DeviceControllerFactory is shut down
        // without ever creating any controllers, so our refcount never goes
        // above 1.  In that case we need to make sure we call Shutdown().
        Shutdown();
    };

    DeviceControllerSystemState(DeviceControllerSystemStateParams params) :
        mSystemLayer(params.systemLayer), mTCPEndPointManager(params.tcpEndPointManager),
        mUDPEndPointManager(params.udpEndPointManager), mTransportMgr(params.transportMgr), mSessionMgr(params.sessionMgr),
        mUnsolicitedStatusHandler(params.unsolicitedStatusHandler), mExchangeMgr(params.exchangeMgr),
        mMessageCounterManager(params.messageCounterManager), mFabrics(params.fabricTable),
        mBDXTransferServer(params.bdxTransferServer), mCASEServer(params.caseServer),
        mCASESessionManager(params.caseSessionManager), mSessionSetupPool(params.sessionSetupPool),
        mCASEClientPool(params.caseClientPool), mGroupDataProvider(params.groupDataProvider), mTimerDelegate(params.timerDelegate),
        mReportScheduler(params.reportScheduler), mSessionKeystore(params.sessionKeystore),
        mFabricTableDelegate(params.fabricTableDelegate),
        mOwnedSessionResumptionStorage(std::move(params.ownedSessionResumptionStorage))
    {
        if (mOwnedSessionResumptionStorage)
        {
            mSessionResumptionStorage = mOwnedSessionResumptionStorage.get();
        }
        else
        {
            mSessionResumptionStorage = params.externalSessionResumptionStorage;
        }

#if CONFIG_NETWORK_LAYER_BLE
        mBleLayer = params.bleLayer;
#endif
        VerifyOrDie(IsInitialized());
    };

    // Acquires a reference to the system state.
    //
    // While a reference is held, the shared state is kept alive. Release()
    // should be called to release the reference once it is no longer needed.
    DeviceControllerSystemState * Retain()
    {
        auto count = mRefCount++;
        VerifyOrDie(count < std::numeric_limits<decltype(count)>::max()); // overflow
        VerifyOrDie(!IsShutDown());                                       // avoid zombie
        return this;
    };

    // Releases a reference to the system state.
    //
    // The stack will shut down when all references are released.
    //
    // NB: The system state is owned by the factory; Release() will not free it
    // but will free its members (Shutdown()).
    //
    // Returns true if the system state was shut down in response to this call.
    bool Release()
    {
        auto count = mRefCount--;
        VerifyOrDie(count > 0); // underflow
        VerifyOrReturnValue(count == 1, false);
        Shutdown();
        return true;
    };
    bool IsInitialized()
    {
        return mSystemLayer != nullptr && mUDPEndPointManager != nullptr && mTransportMgr != nullptr && mSessionMgr != nullptr &&
            mUnsolicitedStatusHandler != nullptr && mExchangeMgr != nullptr && mMessageCounterManager != nullptr &&
            mFabrics != nullptr && mCASESessionManager != nullptr && mSessionSetupPool != nullptr && mCASEClientPool != nullptr &&
            mGroupDataProvider != nullptr && mReportScheduler != nullptr && mTimerDelegate != nullptr &&
            mSessionKeystore != nullptr && mSessionResumptionStorage != nullptr && mBDXTransferServer != nullptr;
    };
    bool IsShutDown() const { return mHaveShutDown; }

    System::Layer * SystemLayer() const { return mSystemLayer; };
    Inet::EndPointManager<Inet::TCPEndPoint> * TCPEndPointManager() const { return mTCPEndPointManager; };
    Inet::EndPointManager<Inet::UDPEndPoint> * UDPEndPointManager() const { return mUDPEndPointManager; };
    DeviceTransportMgr * TransportMgr() const { return mTransportMgr; };
    SessionManager * SessionMgr() const { return mSessionMgr; };
    Messaging::ExchangeManager * ExchangeMgr() const { return mExchangeMgr; }
    secure_channel::MessageCounterManager * MessageCounterManager() const { return mMessageCounterManager; };
    FabricTable * Fabrics() const { return mFabrics; };
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * BleLayer() const { return mBleLayer; };
#endif
    CASESessionManager * CASESessionMgr() const { return mCASESessionManager; }
    Credentials::GroupDataProvider * GetGroupDataProvider() const { return mGroupDataProvider; }
    chip::app::reporting::ReportScheduler * GetReportScheduler() const { return mReportScheduler; }

    Crypto::SessionKeystore * GetSessionKeystore() const { return mSessionKeystore; }
    void SetTempFabricTable(FabricTable * tempFabricTable, bool enableServerInteractions)
    {
        mTempFabricTable          = tempFabricTable;
        mEnableServerInteractions = enableServerInteractions;
    }
    bdx::BDXTransferServer * BDXTransferServer() const { return mBDXTransferServer; }

private:
    DeviceControllerSystemState() {}

    System::Layer * mSystemLayer                                   = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * mTCPEndPointManager = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * mUDPEndPointManager = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif
    DeviceTransportMgr * mTransportMgr                                             = nullptr;
    SessionManager * mSessionMgr                                                   = nullptr;
    Protocols::SecureChannel::UnsolicitedStatusHandler * mUnsolicitedStatusHandler = nullptr;
    Messaging::ExchangeManager * mExchangeMgr                                      = nullptr;
    secure_channel::MessageCounterManager * mMessageCounterManager                 = nullptr;
    FabricTable * mFabrics                                                         = nullptr;
    bdx::BDXTransferServer * mBDXTransferServer                                    = nullptr;
    CASEServer * mCASEServer                                                       = nullptr;
    CASESessionManager * mCASESessionManager                                       = nullptr;
    SessionSetupPool * mSessionSetupPool                                           = nullptr;
    CASEClientPool * mCASEClientPool                                               = nullptr;
    Credentials::GroupDataProvider * mGroupDataProvider                            = nullptr;
    app::reporting::ReportScheduler::TimerDelegate * mTimerDelegate                = nullptr;
    app::reporting::ReportScheduler * mReportScheduler                             = nullptr;
    Crypto::SessionKeystore * mSessionKeystore                                     = nullptr;
    FabricTable::Delegate * mFabricTableDelegate                                   = nullptr;
    SessionResumptionStorage * mSessionResumptionStorage                           = nullptr;
    Platform::UniquePtr<SimpleSessionResumptionStorage> mOwnedSessionResumptionStorage;

    // If mTempFabricTable is not null, it was created during
    // DeviceControllerFactory::InitSystemState and needs to be
    // freed during shutdown
    FabricTable * mTempFabricTable = nullptr;

    std::atomic<uint32_t> mRefCount{ 0 };

    bool mHaveShutDown = false;

    bool mEnableServerInteractions = false;

    void Shutdown();
};

} // namespace Controller
} // namespace chip
