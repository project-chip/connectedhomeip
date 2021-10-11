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

#include <controller/DeviceControllerInteractionModelDelegate.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/TransportMgr.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <ble/BleLayer.h>
#endif

namespace chip {

namespace Controller {

struct DeviceControllerSystemStateParams
{
    System::Layer * systemLayer = nullptr;
    Inet::InetLayer * inetLayer = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * bleLayer = nullptr;
#endif
    DeviceTransportMgr * transportMgr                             = nullptr;
    SessionManager * sessionMgr                                   = nullptr;
    Messaging::ExchangeManager * exchangeMgr                      = nullptr;
    secure_channel::MessageCounterManager * messageCounterManager = nullptr;
    FabricTable * fabricTable                                     = nullptr;
    DeviceControllerInteractionModelDelegate * imDelegate         = nullptr;
};

// A representation of the internal state maintained by the DeviceControllerFactory
// and refcounted by Device Controllers.
// Expects that the creator of this object is the last one to release it.
class DeviceControllerSystemState
{
public:
    ~DeviceControllerSystemState(){};
    DeviceControllerSystemState(DeviceControllerSystemStateParams params) :
        mSystemLayer(params.systemLayer), mInetLayer(params.inetLayer), mTransportMgr(params.transportMgr),
        mSessionMgr(params.sessionMgr), mExchangeMgr(params.exchangeMgr), mMessageCounterManager(params.messageCounterManager),
        mFabrics(params.fabricTable), mIMDelegate(params.imDelegate)
    {
#if CONFIG_NETWORK_LAYER_BLE
        mBleLayer = params.bleLayer;
#endif
    };

    DeviceControllerSystemState * Retain()
    {
        if (mRefCount == std::numeric_limits<uint32_t>::max())
        {
            abort();
        }
        ++mRefCount;
        return this;
    };

    void Release()
    {
        if (mRefCount == 0)
        {
            abort();
        }

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
        return mSystemLayer != nullptr && mInetLayer != nullptr && mTransportMgr != nullptr && mSessionMgr != nullptr &&
            mExchangeMgr != nullptr && mMessageCounterManager != nullptr && mFabrics != nullptr;
    };

    System::Layer * SystemLayer() { return mSystemLayer; };
    Inet::InetLayer * InetLayer() { return mInetLayer; };
    DeviceTransportMgr * TransportMgr() { return mTransportMgr; };
    SessionManager * SessionMgr() { return mSessionMgr; };
    Messaging::ExchangeManager * ExchangeMgr() { return mExchangeMgr; }
    secure_channel::MessageCounterManager * MessageCounterManager() { return mMessageCounterManager; };
    FabricTable * Fabrics() { return mFabrics; };
    DeviceControllerInteractionModelDelegate * IMDelegate() { return mIMDelegate; }
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * BleLayer() { return mBleLayer; };
#endif

private:
    DeviceControllerSystemState(){};

    System::Layer * mSystemLayer = nullptr;
    Inet::InetLayer * mInetLayer = nullptr;
#if CONFIG_NETWORK_LAYER_BLE
    Ble::BleLayer * mBleLayer = nullptr;
#endif
    DeviceTransportMgr * mTransportMgr                             = nullptr;
    SessionManager * mSessionMgr                                   = nullptr;
    Messaging::ExchangeManager * mExchangeMgr                      = nullptr;
    secure_channel::MessageCounterManager * mMessageCounterManager = nullptr;
    FabricTable * mFabrics                                         = nullptr;
    DeviceControllerInteractionModelDelegate * mIMDelegate         = nullptr;

    std::atomic<uint32_t> mRefCount{ 1 };

    CHIP_ERROR Shutdown();
};

} // namespace Controller
} // namespace chip
