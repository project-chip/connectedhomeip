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

#include <credentials/FabricTable.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#include <transport/raw/BLE.h>
#endif

namespace chip {

constexpr size_t kMaxDeviceTransportBlePendingPackets = 1;

using DeviceTransportMgr = TransportMgr<Transport::UDP /* IPv6 */
#if INET_CONFIG_ENABLE_IPV4
                                        ,
                                        Transport::UDP /* IPv4 */
#endif
#if CONFIG_NETWORK_LAYER_BLE
                                        ,
                                        Transport::BLE<kMaxDeviceTransportBlePendingPackets> /* BLE */
#endif
                                        >;

namespace Controller {

struct DeviceControllerSystemStateParams
{
    System::Layer * systemLayer                                   = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * tcpEndPointManager = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * udpEndPointManager = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
    DeviceTransportMgr * transportMgr                             = nullptr;
    SessionManager * sessionMgr                                   = nullptr;
    Messaging::ExchangeManager * exchangeMgr                      = nullptr;
    secure_channel::MessageCounterManager * messageCounterManager = nullptr;
    FabricTable * fabricTable                                     = nullptr;
};

// A representation of the internal state maintained by the DeviceControllerFactory
// and refcounted by Device Controllers.
// Expects that the creator of this object is the last one to release it.
class DeviceControllerSystemState
{
public:
    ~DeviceControllerSystemState(){};
    DeviceControllerSystemState(DeviceControllerSystemStateParams params) :
        mSystemLayer(params.systemLayer), mTCPEndPointManager(params.tcpEndPointManager),
        mUDPEndPointManager(params.udpEndPointManager), mTransportMgr(params.transportMgr), mSessionMgr(params.sessionMgr),
        mExchangeMgr(params.exchangeMgr), mMessageCounterManager(params.messageCounterManager), mFabrics(params.fabricTable)
    {
#if CONFIG_NETWORK_LAYER_BLE
        mBleLayer = params.bleLayer;
#endif
    };

    DeviceControllerSystemState * Retain()
    {
        VerifyOrDie(mRefCount < std::numeric_limits<uint32_t>::max());
        ++mRefCount;
        return this;
    };

    void Release()
    {
        VerifyOrDie(mRefCount > 0);

        mRefCount--;
        if (mRefCount == 1)
        {
            // Only the factory should have a ref now, shutdown and release the underlying objects
            Shutdown();
        }
        else if (mRefCount == 0)
        {
            this->~DeviceControllerSystemState();
        }
    };
    bool IsInitialized()
    {
        return mSystemLayer != nullptr && mUDPEndPointManager != nullptr && mTransportMgr != nullptr && mSessionMgr != nullptr &&
            mExchangeMgr != nullptr && mMessageCounterManager != nullptr && mFabrics != nullptr;
    };

    System::Layer * SystemLayer() { return mSystemLayer; };
    Inet::EndPointManager<Inet::TCPEndPoint> * TCPEndPointManager() { return mTCPEndPointManager; };
    Inet::EndPointManager<Inet::UDPEndPoint> * UDPEndPointManager() { return mUDPEndPointManager; };
    DeviceTransportMgr * TransportMgr() { return mTransportMgr; };
    SessionManager * SessionMgr() { return mSessionMgr; };
    Messaging::ExchangeManager * ExchangeMgr() { return mExchangeMgr; }
    secure_channel::MessageCounterManager * MessageCounterManager() { return mMessageCounterManager; };
    FabricTable * Fabrics() { return mFabrics; };
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * BleLayer() { return mBleLayer; };
#endif

private:
    DeviceControllerSystemState(){};

    System::Layer * mSystemLayer                                   = nullptr;
    Inet::EndPointManager<Inet::TCPEndPoint> * mTCPEndPointManager = nullptr;
    Inet::EndPointManager<Inet::UDPEndPoint> * mUDPEndPointManager = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif
    DeviceTransportMgr * mTransportMgr                             = nullptr;
    SessionManager * mSessionMgr                                   = nullptr;
    Messaging::ExchangeManager * mExchangeMgr                      = nullptr;
    secure_channel::MessageCounterManager * mMessageCounterManager = nullptr;
    FabricTable * mFabrics                                         = nullptr;

    std::atomic<uint32_t> mRefCount{ 1 };

    CHIP_ERROR Shutdown();
};

} // namespace Controller
} // namespace chip
